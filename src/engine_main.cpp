#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QTimer>

#include <algorithm>
#include <cmath>
#ifndef Q_OS_WIN
#include <cerrno>
#include <cstring>
#include <ctime>
#endif

#include "engine/Adl200Meter.h"
#include "engine/Amc16zFak24Meter.h"
#include "engine/AppConfig.h"
#include "engine/Asj60Ld16aMonitor.h"
#include "engine/LineManager.h"
#include "engine/LineOperationalMonitor.h"
#include "engine/Logger.h"
#include "engine/MaintenanceChecker.h"
#include "engine/ManualEmergencyController.h"
#include "engine/MeteringBusController.h"
#include "engine/ModbusController.h"
#include "engine/StateEngine.h"
#include "engine/StateFileStore.h"
#include "engine/TestController.h"
#include "engine/TestJournalStore.h"
#include "engine/TestScheduleManager.h"
#include "engine/WhdTemperatureHumidityController.h"
#include "PasswordManager.h"

using namespace DialogG2;

static QString defaultStatePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("state/current_state.json"));
}

static QString defaultLogPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("logs/system.log"));
}

static QString defaultLinesConfigPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config/lines.json"));
}

static QString defaultTestJournalPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("state/test_journal.json"));
}

static QString defaultTestSchedulePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config/test_schedule.json"));
}

static QString defaultAppConfigPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config/app.json"));
}

static QString ipcServerName()
{
    return QStringLiteral("emergency_panel_backend");
}

static QJsonObject okResponse()
{
    return {{QStringLiteral("ok"), true}};
}

static QJsonObject errorResponse(const QString &message)
{
    return {
        {QStringLiteral("ok"), false},
        {QStringLiteral("error"), message}
    };
}

static int lineModeToHmi(LineKind kind, bool enabled)
{
    if (!enabled)
        return 2;
    return kind == LineKind::Constant ? 0 : 1;
}

static void applyHmiLineMode(int mode, LineConfig *line)
{
    if (!line)
        return;

    line->enabled = mode != 2;
    line->kind = mode == 1 ? LineKind::NonConstant : LineKind::Constant;
}

static QJsonObject scheduleEntryToHmi(const TestScheduleEntry &entry)
{
    QJsonArray weekDays;
    for (const QString &day : entry.weekDays)
        weekDays.append(day);

    return {
        {QStringLiteral("enabled"), entry.enabled},
        {QStringLiteral("period"), entry.period},
        {QStringLiteral("startDate"), entry.startDate.toString(QStringLiteral("yyyy-MM-dd"))},
        {QStringLiteral("startTime"), entry.startTime.toString(QStringLiteral("HH:mm"))},
        {QStringLiteral("testType"), entry.testType},
        {QStringLiteral("weekDays"), weekDays}
    };
}

static TestScheduleEntry scheduleEntryFromHmi(const QJsonObject &object)
{
    TestScheduleEntry entry;
    entry.enabled = object.value(QStringLiteral("enabled")).toBool(true);
    entry.period = object.value(QStringLiteral("period")).toString(QStringLiteral("один раз"));
    entry.startDate = QDate::fromString(object.value(QStringLiteral("startDate")).toString(), QStringLiteral("yyyy-MM-dd"));
    entry.startTime = QTime::fromString(object.value(QStringLiteral("startTime")).toString(), QStringLiteral("HH:mm"));
    entry.testType = object.value(QStringLiteral("testType")).toString(QStringLiteral("Функциональный"));

    const QJsonArray days = object.value(QStringLiteral("weekDays")).toArray();
    for (const QJsonValue &day : days)
        entry.weekDays.append(day.toString());

    return entry;
}

static int maxModule(const LineManager &lineManager)
{
    int result = 1;
    const CabinetIoMap &io = lineManager.ioMap();
    const QVector<WaveSharePoint> cabinetPoints = {
        io.fireInput,
        io.manualFireButton,
        io.manualStopButton,
        io.voltageControlInput,
        io.modeRelay,
        io.faultLampRelay,
        io.testLampRelay,
        io.reserveRelay
    };

    for (const WaveSharePoint &point : cabinetPoints) {
        if (point.isValid())
            result = std::max(result, point.module);
    }

    for (const LineConfig &line : lineManager.lines()) {
        if (line.requestInput.isValid())
            result = std::max(result, line.requestInput.module);
        if (line.outputRelay.isValid())
            result = std::max(result, line.outputRelay.module);
    }

    return result;
}

class EngineRuntime : public QObject
{
public:
    EngineRuntime(AppConfig config, QString statePath, bool demoMode, QObject *parent = nullptr)
        : QObject(parent)
        , m_config(std::move(config))
        , m_stateStore(std::move(statePath))
        , m_journalStore(defaultTestJournalPath())
        , m_demoMode(demoMode)
    {
        QString error;
        if (!m_lineManager.loadConfig(defaultLinesConfigPath(), &error)) {
            LOG_WARN(QStringLiteral("Line config not loaded: %1. Using defaults").arg(error));
            if (!m_lineManager.saveDefaultConfig(defaultLinesConfigPath(), &error))
                LOG_WARN(QStringLiteral("Default line config not saved: %1").arg(error));
        }

        if (!m_journalStore.read(&m_storedJournal, &error))
            LOG_WARN(QStringLiteral("Test journal not loaded: %1").arg(error));

        if (!m_scheduleManager.load(defaultTestSchedulePath(), &error)) {
            LOG_WARN(QStringLiteral("Test schedule not loaded: %1. Creating empty schedule").arg(error));
            if (!m_scheduleManager.saveDefault(defaultTestSchedulePath(), &error))
                LOG_WARN(QStringLiteral("Default test schedule not saved: %1").arg(error));
        }

        m_battery.connected = false;
        m_battery.communicationOk = false;
        m_battery.state = BatteryState::Disconnected;
        m_battery.faults = {QStringLiteral("нет данных BMS")};

        const int moduleCount = maxModule(m_lineManager);
        for (int module = 1; module <= moduleCount; ++module) {
            WaveShareModuleState state;
            state.module = module;
            state.online = false;
            m_modules.insert(module, state);
        }

        if (m_demoMode) {
            setupDemoState();
        } else {
            setupRelayBus(moduleCount);
            setupMeteringBus();
        }

        connect(&m_tickTimer, &QTimer::timeout, this, &EngineRuntime::tick);
        m_tickTimer.setInterval(500);
    }

    void start()
    {
        if (m_demoMode) {
            LOG_INFO(QStringLiteral("Demo mode enabled: hardware polling is disabled"));
        } else {
            m_relayBus.connectDevice();
            m_meteringBus.connectDevice();

            m_relayBus.startPolling();
            m_meteringBus.startPolling();
        }

        m_tickTimer.start();
        startIpcServer();
        tick();
    }

private:
    void startIpcServer()
    {
        connect(&m_ipcServer, &QLocalServer::newConnection, this, [this]() {
            while (QLocalSocket *socket = m_ipcServer.nextPendingConnection()) {
                connect(socket, &QLocalSocket::readyRead, this, [this, socket]() {
                    processIpcMessage(socket, socket->readAll());
                });
                connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            }
        });

        QLocalServer::removeServer(ipcServerName());
        if (!m_ipcServer.listen(ipcServerName())) {
            LOG_WARN(QStringLiteral("IPC listen failed: %1").arg(m_ipcServer.errorString()));
            return;
        }

        LOG_INFO(QStringLiteral("IPC server started: %1").arg(ipcServerName()));
    }

    void processIpcMessage(QLocalSocket *socket, const QByteArray &data)
    {
        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            sendIpcJson(socket, errorResponse(QStringLiteral("invalid json")));
            return;
        }

        const QJsonObject request = doc.object();
        const QString command = request.value(QStringLiteral("cmd")).toString();

        if (command == QStringLiteral("getState")) {
            sendIpcJson(socket, {
                {QStringLiteral("ok"), true},
                {QStringLiteral("state"), stateForHmi()}
            });
            return;
        }

        if (command == QStringLiteral("lineAt")) {
            const int index = request.value(QStringLiteral("index")).toInt(-1);
            const QJsonObject line = lineAtForHmi(index);
            sendIpcJson(socket, line.isEmpty()
                                    ? errorResponse(QStringLiteral("line not found"))
                                    : QJsonObject{{QStringLiteral("ok"), true}, {QStringLiteral("line"), line}});
            return;
        }

        if (command == QStringLiteral("addLine")) {
            QString error;
            LineConfig line = m_lineManager.makeNextLine(LineKind::Constant);
            const bool ok = m_lineManager.addLine(line, &error)
                && m_lineManager.saveConfig(defaultLinesConfigPath(), &error);
            if (!ok)
                LOG_WARN(QStringLiteral("Line not added: %1").arg(error));
            sendIpcJson(socket, {{QStringLiteral("ok"), ok}});
            return;
        }

        if (command == QStringLiteral("updateLine")) {
            const int index = request.value(QStringLiteral("index")).toInt(-1);
            const QJsonObject lineData = request.value(QStringLiteral("lineData")).toObject();
            sendIpcJson(socket, {{QStringLiteral("ok"), updateLineFromHmi(index, lineData)}});
            return;
        }

        if (command == QStringLiteral("saveLines")) {
            QString saveError;
            const bool ok = m_lineManager.saveConfig(defaultLinesConfigPath(), &saveError);
            if (!ok)
                LOG_WARN(QStringLiteral("Line config not saved: %1").arg(saveError));
            sendIpcJson(socket, {{QStringLiteral("ok"), ok}});
            return;
        }

        if (command == QStringLiteral("applyLineModes")) {
            sendIpcJson(socket, okResponse());
            return;
        }

        if (command == QStringLiteral("stopCurrentTest")) {
            m_stopTestRequested = true;
            sendIpcJson(socket, okResponse());
            return;
        }

        if (command == QStringLiteral("startFunctionalTest")) {
            const int warmupSec = std::clamp(request.value(QStringLiteral("warmupSec")).toInt(0), 0, 59 * 60);
            m_manualFunctionalRequest = {true, warmupSec};
            sendIpcJson(socket, okResponse());
            return;
        }

        if (command == QStringLiteral("startDurationTest")) {
            const int durationSec = std::clamp(request.value(QStringLiteral("durationSec")).toInt(m_testControllerDefaultDurationSec),
                                               1,
                                               3 * 3600);
            m_manualDurationRequest = {true, durationSec};
            sendIpcJson(socket, okResponse());
            return;
        }

        if (command == QStringLiteral("setLineSetupActive")) {
            const int hmiIndex = request.value(QStringLiteral("index")).toInt(-1);
            if (request.value(QStringLiteral("active")).toBool(false) && hmiIndex >= 0)
                m_hmiSetupLineIndex = hmiIndex + 1;
            else if (m_hmiSetupLineIndex == hmiIndex + 1 || hmiIndex < 0)
                m_hmiSetupLineIndex = 0;
            sendIpcJson(socket, okResponse());
            return;
        }

        if (command == QStringLiteral("startManualEmergency")) {
            m_hmiManualEmergencyStartRequested = true;
            sendIpcJson(socket, okResponse());
            return;
        }

        if (command == QStringLiteral("stopManualEmergency")) {
            m_hmiManualEmergencyStopRequested = true;
            sendIpcJson(socket, okResponse());
            return;
        }

        if (command == QStringLiteral("checkPassword")) {
            sendIpcJson(socket, {
                {QStringLiteral("ok"), true},
                {QStringLiteral("match"), m_passwordManager.password() == request.value(QStringLiteral("password")).toString()}
            });
            return;
        }

        if (command == QStringLiteral("changePassword")) {
            m_passwordManager.setPassword(request.value(QStringLiteral("password")).toString());
            sendIpcJson(socket, okResponse());
            return;
        }

        if (command == QStringLiteral("setSystemTime")) {
            const qint64 msec = request.value(QStringLiteral("time")).toVariant().toLongLong();
            const bool ok = setSystemTimeFromHmi(msec);
            sendIpcJson(socket, {{QStringLiteral("ok"), ok}});
            return;
        }

        if (command == QStringLiteral("getAllTests")) {
            QJsonArray entries;
            for (const TestScheduleEntry &entry : m_scheduleManager.entries())
                entries.append(scheduleEntryToHmi(entry));
            sendIpcJson(socket, {
                {QStringLiteral("ok"), true},
                {QStringLiteral("entries"), entries}
            });
            return;
        }

        if (command == QStringLiteral("addTest")) {
            QString error;
            const TestScheduleEntry entry = scheduleEntryFromHmi(request.value(QStringLiteral("data")).toObject());
            const bool ok = m_scheduleManager.addEntry(entry, &error)
                && m_scheduleManager.save(defaultTestSchedulePath(), &error);
            if (!ok)
                LOG_WARN(QStringLiteral("Schedule entry not added: %1").arg(error));
            sendIpcJson(socket, {{QStringLiteral("ok"), ok}});
            return;
        }

        if (command == QStringLiteral("removeTest")) {
            QString error;
            const bool ok = m_scheduleManager.removeEntry(request.value(QStringLiteral("index")).toInt(-1), &error)
                && m_scheduleManager.save(defaultTestSchedulePath(), &error);
            if (!ok)
                LOG_WARN(QStringLiteral("Schedule entry not removed: %1").arg(error));
            sendIpcJson(socket, {{QStringLiteral("ok"), ok}});
            return;
        }

        if (command == QStringLiteral("updateTestProperty")) {
            QString error;
            const bool ok = m_scheduleManager.updateEntryProperty(request.value(QStringLiteral("index")).toInt(-1),
                                                                  request.value(QStringLiteral("key")).toString(),
                                                                  request.value(QStringLiteral("value")).toVariant(),
                                                                  &error)
                && m_scheduleManager.save(defaultTestSchedulePath(), &error);
            if (!ok)
                LOG_WARN(QStringLiteral("Schedule entry not updated: %1").arg(error));
            sendIpcJson(socket, {{QStringLiteral("ok"), ok}});
            return;
        }

        if (command == QStringLiteral("updateWeekDays")) {
            QStringList days;
            const QJsonArray array = request.value(QStringLiteral("days")).toArray();
            for (const QJsonValue &day : array)
                days.append(day.toString());

            QString error;
            const bool ok = m_scheduleManager.updateEntryProperty(request.value(QStringLiteral("index")).toInt(-1),
                                                                  QStringLiteral("weekDays"),
                                                                  days,
                                                                  &error)
                && m_scheduleManager.save(defaultTestSchedulePath(), &error);
            if (!ok)
                LOG_WARN(QStringLiteral("Schedule week days not updated: %1").arg(error));
            sendIpcJson(socket, {{QStringLiteral("ok"), ok}});
            return;
        }

        if (command == QStringLiteral("readLogs")) {
            sendIpcJson(socket, {
                {QStringLiteral("ok"), true},
                {QStringLiteral("lines"), logsForHmi(request.value(QStringLiteral("offset")).toInt(0),
                                                     request.value(QStringLiteral("limit")).toInt(200))}
            });
            return;
        }

        if (command == QStringLiteral("journal")) {
            sendIpcJson(socket, {
                {QStringLiteral("ok"), true},
                {QStringLiteral("entries"), journalForHmi()}
            });
            return;
        }

        sendIpcJson(socket, errorResponse(QStringLiteral("unknown command")));
    }

    void sendIpcJson(QLocalSocket *socket, const QJsonObject &object)
    {
        socket->write(QJsonDocument(object).toJson(QJsonDocument::Compact));
        socket->flush();
        socket->disconnectFromServer();
    }

    QJsonObject lineAtForHmi(int hmiIndex) const
    {
        const LineConfig *line = m_lineManager.line(hmiIndex + 1);
        if (!line)
            return {};

        double measuredPower = std::numeric_limits<double>::quiet_NaN();
        double measuredVoltage = std::numeric_limits<double>::quiet_NaN();
        double measuredCurrent = std::numeric_limits<double>::quiet_NaN();
        double leakage = std::numeric_limits<double>::quiet_NaN();

        for (const LineSnapshot &snapshotLine : m_lastSnapshot.lines) {
            if (snapshotLine.index != line->index)
                continue;
            measuredPower = snapshotLine.outputPower;
            measuredVoltage = snapshotLine.outputVoltage;
            measuredCurrent = snapshotLine.outputCurrent;
            leakage = snapshotLine.leakageCurrent;
            break;
        }

        return {
            {QStringLiteral("index"), line->index},
            {QStringLiteral("description"), line->name},
            {QStringLiteral("mpower"), std::isfinite(line->nominalPower) ? line->nominalPower : 0.0},
            {QStringLiteral("power"), std::isfinite(measuredPower) ? measuredPower : 0.0},
            {QStringLiteral("voltage"), std::isfinite(measuredVoltage) ? measuredVoltage : 0.0},
            {QStringLiteral("current"), std::isfinite(measuredCurrent) ? measuredCurrent : 0.0},
            {QStringLiteral("leakage"), std::isfinite(leakage) ? leakage : 0.0},
            {QStringLiteral("powerAvailable"), std::isfinite(measuredPower)},
            {QStringLiteral("voltageAvailable"), std::isfinite(measuredVoltage)},
            {QStringLiteral("currentAvailable"), std::isfinite(measuredCurrent)},
            {QStringLiteral("tolerance"), std::isfinite(line->powerTestTolerancePercent) ? line->powerTestTolerancePercent : 5.0},
            {QStringLiteral("mode"), lineModeToHmi(line->kind, line->enabled)}
        };
    }

    bool updateLineFromHmi(int hmiIndex, const QJsonObject &lineData)
    {
        const LineConfig *existing = m_lineManager.line(hmiIndex + 1);
        if (!existing)
            return false;

        LineConfig line = *existing;
        if (lineData.contains(QStringLiteral("description")))
            line.name = lineData.value(QStringLiteral("description")).toString(line.name);
        if (lineData.contains(QStringLiteral("mpower")))
            line.nominalPower = lineData.value(QStringLiteral("mpower")).toDouble(line.nominalPower);
        if (lineData.contains(QStringLiteral("tolerance")))
            line.powerTestTolerancePercent = lineData.value(QStringLiteral("tolerance")).toDouble(line.powerTestTolerancePercent);
        if (lineData.contains(QStringLiteral("mode")))
            applyHmiLineMode(lineData.value(QStringLiteral("mode")).toInt(lineModeToHmi(line.kind, line.enabled)), &line);

        QString error;
        const bool ok = m_lineManager.updateLine(line, &error);
        if (!ok)
            LOG_WARN(QStringLiteral("Line update failed: %1").arg(error));
        return ok;
    }

    QJsonObject stateForHmi() const
    {
        QJsonArray lines;
        for (int i = 0; i < m_lineManager.lines().size(); ++i)
            lines.append(lineAtForHmi(i));

        const bool testRunning = m_lastSnapshot.activeTest.active;
        const int remainingSec = testRunning && m_lastSnapshot.activeTest.dueAt.isValid()
            ? std::max(0, static_cast<int>(QDateTime::currentDateTimeUtc().secsTo(m_lastSnapshot.activeTest.dueAt)))
            : 0;

        return {
            {QStringLiteral("connected"), true},
            {QStringLiteral("busConnected"), m_relayBusStatus.online && m_meteringBusStatus.online},
            {QStringLiteral("testRunning"), testRunning},
            {QStringLiteral("testPlannedSec"), m_lastSnapshot.activeTest.durationSeconds},
            {QStringLiteral("testRemainingSec"), remainingSec},
            {QStringLiteral("modeText"), modeText(m_lastSnapshot.mode)},
            {QStringLiteral("healthText"), healthText(m_lastSnapshot.health)},
            {QStringLiteral("modeColor"), m_lastSnapshot.mode == CabinetMode::Normal ? QStringLiteral("#11bf5d") : QStringLiteral("#d84236")},
            {QStringLiteral("manualEmergencyActive"), m_lastSnapshot.manualEmergencyActive},
            {QStringLiteral("systemOk"), m_lastSnapshot.health == SystemHealth::Normal},
            {QStringLiteral("linesOk"), !m_lastSnapshot.activeFaults.join(QString()).contains(QStringLiteral("линия"), Qt::CaseInsensitive)},
            {QStringLiteral("batteryOk"), m_lastSnapshot.battery.state == BatteryState::Normal},
            {QStringLiteral("batteryPercent"), m_lastSnapshot.battery.socPercent},
            {QStringLiteral("battery"), toJson(m_lastSnapshot.battery)},
            {QStringLiteral("systemState"), m_lastSnapshot.health == SystemHealth::Normal ? 0 : 1},
            {QStringLiteral("cabinetMode"), static_cast<int>(m_lastSnapshot.mode)},
            {QStringLiteral("lineCount"), m_lineManager.lines().size()},
            {QStringLiteral("lines"), lines},
            {QStringLiteral("inletU"), std::isfinite(m_lastSnapshot.inputVoltage) ? m_lastSnapshot.inputVoltage : 0.0},
            {QStringLiteral("inletP"), std::isfinite(m_lastSnapshot.inputPower) ? m_lastSnapshot.inputPower : 0.0},
            {QStringLiteral("inletI"), std::isfinite(m_lastSnapshot.inputCurrent) ? m_lastSnapshot.inputCurrent : 0.0},
            {QStringLiteral("inletF"), std::isfinite(m_lastSnapshot.inputFrequency) ? m_lastSnapshot.inputFrequency : 0.0},
            {QStringLiteral("temperature"), std::isfinite(m_lastSnapshot.temperature) ? m_lastSnapshot.temperature : 0.0},
            {QStringLiteral("temperatureAvailable"), std::isfinite(m_lastSnapshot.temperature)},
            {QStringLiteral("logLevel"), m_demoMode ? QStringLiteral("DEBUG") : QStringLiteral("INFO")}
        };
    }

    bool setSystemTimeFromHmi(qint64 msec)
    {
        if (msec <= 0)
            return false;

        const QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(msec);
        LOG_INFO(QStringLiteral("System time change requested from HMI: %1").arg(dateTime.toString(Qt::ISODate)));
#ifdef Q_OS_WIN
        LOG_WARN(QStringLiteral("System time change is not supported in Windows debug build"));
        return false;
#else
        timespec ts;
        ts.tv_sec = static_cast<time_t>(msec / 1000);
        ts.tv_nsec = static_cast<long>((msec % 1000) * 1000000);

        if (::clock_settime(CLOCK_REALTIME, &ts) != 0) {
            LOG_WARN(QStringLiteral("clock_settime failed errno=%1: %2")
                         .arg(errno)
                         .arg(QString::fromLocal8Bit(std::strerror(errno))));
            return false;
        }

        QProcess hwclock;
        hwclock.start(QStringLiteral("hwclock"), {QStringLiteral("--systohc")});
        if (!hwclock.waitForFinished(3000))
            LOG_WARN(QStringLiteral("hwclock --systohc timeout"));

        return true;
#endif
    }

    QJsonArray journalForHmi() const
    {
        QJsonArray entries;
        for (const TestJournalEntry &entry : m_storedJournal)
            entries.append(toJson(entry));
        return entries;
    }

    QJsonArray logsForHmi(int offset, int limit) const
    {
        QJsonArray lines;
        QFile file(defaultLogPath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return lines;

        const QList<QByteArray> allLines = file.readAll().split('\n');
        const int lineCount = static_cast<int>(allLines.size());
        const int start = offset < 0 ? std::max(0, lineCount + offset) : std::max(0, offset);
        const int end = std::min(lineCount, start + std::max(0, limit));
        for (int i = start; i < end; ++i)
            lines.append(QString::fromUtf8(allLines.at(i)).trimmed());
        return lines;
    }

    void setupRelayBus(int moduleCount)
    {
        m_relayBus.configure(m_config.relayRtu());
        m_relayBus.setWaveShareModuleCount(moduleCount);
        for (int module = 1; module <= moduleCount; ++module)
            m_relayBus.addWaveShareModulePolling(module, 200, 1000);

        connect(&m_relayBus, &ModbusController::waveShareInputsUpdated, this, [this](int module, quint8 bits) {
            WaveShareModuleState state = m_modules.value(module);
            state.module = module;
            state.inputs = bits;
            state.online = true;
            m_modules.insert(module, state);
        });
        connect(&m_relayBus, &ModbusController::waveShareRelaysUpdated, this, [this](int module, quint8 bits) {
            WaveShareModuleState state = m_modules.value(module);
            state.module = module;
            state.relays = bits;
            state.online = true;
            m_modules.insert(module, state);
        });
        connect(&m_relayBus, &ModbusController::busStatusChanged, this, [this](const ModbusBusStatus &status) {
            m_relayBusStatus = status;
        });
        connect(&m_relayBus, &ModbusController::errorOccurred, this, [](const QString &message) {
            LOG_WARN(QStringLiteral("Relay bus: %1").arg(message));
        });
    }

    void setupMeteringBus()
    {
        m_meteringBus.configure(m_config.meteringRtu());
        m_meteringBus.addAdl200InputMeterPolling(1000);
        m_meteringBus.addAmc16zFak24BranchPowerPolling(1000);
        m_meteringBus.addAsj60Ld16aLeakagePolling(1000);
        m_meteringBus.addWhdTemperatureHumidityPolling(5000);
        m_meteringBus.addJbdBmsPolling(2000, 10000);

        connect(&m_meteringBus, &MeteringBusController::adl200InputMeterUpdated,
                this, [this](const Adl200Measurement &measurement) {
            if (!measurement.valid)
                return;
            m_inputVoltage = measurement.voltage;
            m_inputCurrent = measurement.current;
            m_inputPower = measurement.activePower;
            m_inputFrequency = measurement.frequency;
        });
        connect(&m_meteringBus, &MeteringBusController::amc16zFak24BranchPowersUpdated,
                this, [this](const QVector<Amc16zBranchMeasurement> &measurements) {
            for (const Amc16zBranchMeasurement &measurement : measurements) {
                if (measurement.valid)
                    m_linePowers.insert(measurement.channel, measurement.activePower);
            }
        });
        connect(&m_meteringBus, &MeteringBusController::asj60Ld16aLeakageUpdated,
                this, [this](const QVector<Asj60LeakageChannel> &channels) {
            for (const Asj60LeakageChannel &channel : channels) {
                if (channel.valid)
                    m_lineLeakage.insert(channel.channel, channel.leakageCurrent);
            }
        });
        connect(&m_meteringBus, &MeteringBusController::whdTemperatureHumidityUpdated,
                this, [this](const WhdMeasurement &measurement) {
            if (measurement.valid)
                m_temperature = measurement.temperature;
        });
        connect(&m_meteringBus, &MeteringBusController::jbdBmsBatteryUpdated,
                this, [this](const BatterySnapshot &battery) {
            m_battery = battery;
        });
        connect(&m_meteringBus, &MeteringBusController::busStatusChanged, this, [this](const ModbusBusStatus &status) {
            m_meteringBusStatus = status;
        });
        connect(&m_meteringBus, &MeteringBusController::errorOccurred, this, [](const QString &message) {
            LOG_WARN(QStringLiteral("Metering bus: %1").arg(message));
        });
    }

    void setupDemoState()
    {
        for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
            WaveShareModuleState state = it.value();
            state.online = true;
            state.inputs = 0;
            state.relays = 0;
            if (state.module == 1)
                state.inputs = static_cast<quint8>(state.inputs | (1u << 3)); // voltage control input, channel 4
            it.value() = state;
        }

        m_relayBusStatus.online = true;
        m_meteringBusStatus.online = true;

        m_inputVoltage = 230.0;
        m_inputCurrent = 1.2;
        m_inputPower = 180.0;
        m_inputFrequency = 50.0;
        m_temperature = 24.0;

        for (const LineConfig &line : m_lineManager.lines()) {
            if (std::isfinite(line.nominalPower))
                m_linePowers.insert(line.index, line.nominalPower);
            m_lineLeakage.insert(line.index, 0.0);
        }

        m_battery.connected = true;
        m_battery.communicationOk = true;
        m_battery.state = BatteryState::Normal;
        m_battery.socPercent = 80;
        m_battery.voltage = 51.2;
        m_battery.current = 0.0;
        m_battery.remainingCapacityAh = 80.0;
        m_battery.nominalCapacityAh = 100.0;
        m_battery.fullChargeCapacityAh = 96.0;
        m_battery.cycleCount = 12;
        m_battery.cellCount = 16;
        m_battery.cellVoltages = QVector<double>(16, 3.2);
        m_battery.minCellVoltage = 3.19;
        m_battery.maxCellVoltage = 3.21;
        m_battery.cellVoltageDelta = 0.02;
        m_battery.temperatures = {24.0, 25.0};
        m_battery.minTemperature = 24.0;
        m_battery.maxTemperature = 25.0;
    }

    void applyMeasurements(QVector<LineSnapshot> *lines) const
    {
        if (!lines)
            return;

        for (LineSnapshot &line : *lines) {
            if (m_linePowers.contains(line.index))
                line.outputPower = m_linePowers.value(line.index);
            if (m_lineLeakage.contains(line.index)) {
                line.leakageCurrent = m_lineLeakage.value(line.index);
                if (std::isfinite(line.leakageCurrent) && line.leakageCurrent > line.leakageCurrentLimit)
                    line.state = LineState::InsulationBreakdown;
            }
        }
    }

    void tick()
    {
        const QDateTime now = QDateTime::currentDateTimeUtc();

        LineManagerInputs lineInputs;
        lineInputs.modules = m_modules;
        lineInputs.forceLinesOn = m_lastSnapshot.activeTest.active;
        lineInputs.forceLineIndex = m_hmiSetupLineIndex;
        LineManagerResult lineResult = m_lineManager.evaluate(lineInputs);

        QVector<LineSnapshot> lines = lineResult.lines;
        applyMeasurements(&lines);
        lines = m_operationalMonitor.evaluate(lines, now);

        ManualEmergencyInputs manualEmergencyInputs;
        manualEmergencyInputs.startRequested = lineResult.manualFireButtonActive || m_hmiManualEmergencyStartRequested;
        manualEmergencyInputs.stopRequested = lineResult.manualStopButtonActive || m_hmiManualEmergencyStopRequested;
        const bool manualEmergencyActive = m_manualEmergencyController.evaluate(manualEmergencyInputs);
        m_hmiManualEmergencyStartRequested = false;
        m_hmiManualEmergencyStopRequested = false;

        TestControllerInputs testInputs;
        testInputs.now = now;
        testInputs.voltageControlOk = lineResult.voltageControlOk;
        testInputs.fireInputActive = lineResult.fireInputActive || manualEmergencyActive;
        testInputs.stopRequested = lineResult.manualStopButtonActive || m_stopTestRequested;
        testInputs.lines = lines;
        testInputs.manualFunctional = m_manualFunctionalRequest;
        testInputs.manualDuration = m_manualDurationRequest;
        const TestScheduleRequest scheduleRequest = m_scheduleManager.evaluate(now);
        if (scheduleRequest.functional.active || scheduleRequest.duration.active) {
            LOG_INFO(QStringLiteral("Scheduled test requested: %1, period=%2, planned=%3")
                         .arg(scheduleRequest.testType,
                              scheduleRequest.period,
                              scheduleRequest.plannedAt.toString(Qt::ISODate)));
        }
        testInputs.scheduledFunctional = scheduleRequest.functional;
        testInputs.scheduledDuration = scheduleRequest.duration;
        const TestControllerResult testResult = m_testController.evaluate(testInputs);

        if (!testResult.newJournalEntries.isEmpty())
            persistTestResults(testResult.newJournalEntries);

        if (m_manualFunctionalRequest.active && testResult.manualTestActive)
            m_manualFunctionalRequest = {};
        if (m_manualDurationRequest.active && testResult.manualTestActive)
            m_manualDurationRequest = {};
        if (m_stopTestRequested && !testResult.activeTest.active)
            m_stopTestRequested = false;

        QVector<LineSnapshot> snapshotLines = testResult.lines;
        if (testResult.activeTest.active) {
            for (LineSnapshot &line : snapshotLines) {
                if (line.enabled)
                    line.outputState = LineOutputState::On;
            }
        }
        const MaintenanceSnapshot maintenance =
            m_maintenanceChecker.evaluate(snapshotLines, m_storedJournal, now);

        EngineInputs engineInputs;
        engineInputs.voltageControlOk = lineResult.voltageControlOk;
        engineInputs.fireInputActive = lineResult.fireInputActive;
        engineInputs.manualEmergencyActive = manualEmergencyActive;
        engineInputs.manualTestActive = testResult.manualTestActive;
        engineInputs.scheduledTestActive = testResult.scheduledTestActive;
        engineInputs.testKind = testResult.testKind;
        engineInputs.testSource = testResult.testSource;
        engineInputs.activeTest = testResult.activeTest;
        engineInputs.testJournal = m_storedJournal;
        engineInputs.relayFault = !lineResult.faults.isEmpty() || !m_relayBusStatus.online;
        engineInputs.modbusFault = !m_meteringBusStatus.online;
        engineInputs.leakageFault = hasLeakageFault(testResult.lines);
        engineInputs.inputVoltage = m_inputVoltage;
        engineInputs.inputCurrent = m_inputCurrent;
        engineInputs.inputPower = m_inputPower;
        engineInputs.inputFrequency = m_inputFrequency;
        engineInputs.temperature = m_temperature;
        engineInputs.battery = m_battery;
        engineInputs.lines = snapshotLines;
        engineInputs.maintenance = maintenance;

        const CabinetSnapshot snapshot = m_stateEngine.evaluate(engineInputs);
        m_lastSnapshot = snapshot;
        writeState(snapshot);
        driveRelays(snapshot, lineInputs.modules);
        writeDebugHeartbeat(snapshot);
    }

    void persistTestResults(const QVector<TestJournalEntry> &entries)
    {
        QString error;
        if (!m_journalStore.append(entries, &error)) {
            LOG_WARN(QStringLiteral("Test journal not saved: %1").arg(error));
            return;
        }

        m_storedJournal += entries;
        m_lineManager.applyTestResults(entries);
        if (!m_lineManager.saveConfig(defaultLinesConfigPath(), &error))
            LOG_WARN(QStringLiteral("Line test results not saved: %1").arg(error));
    }

    bool hasLeakageFault(const QVector<LineSnapshot> &lines) const
    {
        for (const LineSnapshot &line : lines) {
            if (line.state == LineState::InsulationBreakdown)
                return true;
        }
        return false;
    }

    void writeState(const CabinetSnapshot &snapshot)
    {
        QString error;
        if (!m_stateStore.write(snapshot, &error)) {
            LOG_CRITICAL(QStringLiteral("State write failed: %1").arg(error));
            return;
        }

        if (snapshot.mode != m_lastLoggedMode || snapshot.health != m_lastLoggedHealth) {
            LOG_INFO(QStringLiteral("%1 / %2 - %3")
                         .arg(modeText(snapshot.mode),
                              healthText(snapshot.health),
                              snapshot.explanation));
            m_lastLoggedMode = snapshot.mode;
            m_lastLoggedHealth = snapshot.health;
        }
    }

    void driveRelays(const CabinetSnapshot &snapshot, const QHash<int, WaveShareModuleState> &modules)
    {
        LineManagerInputs outputInputs;
        outputInputs.modules = modules;
        outputInputs.modeRelayOn = snapshot.mode != CabinetMode::Normal;
        outputInputs.faultLampOn = snapshot.health == SystemHealth::Fault;
        outputInputs.testLampOn = snapshot.activeTest.active;
        outputInputs.forceLinesOn = snapshot.activeTest.active;
        outputInputs.forceLineIndex = m_hmiSetupLineIndex;

        const LineManagerResult outputResult = m_lineManager.evaluate(outputInputs);
        const QList<int> relayModules = outputResult.relayOutputBytes.keys();
        for (int module : relayModules) {
            const quint8 bits = outputResult.relayOutputBytes.value(module);
            if (m_lastRelayBytes.value(module, 0xFF) == bits)
                continue;

            if (!m_demoMode)
                m_relayBus.writeWaveShareRelayByte(module, bits);
            m_lastRelayBytes.insert(module, bits);

            if (m_demoMode) {
                WaveShareModuleState state = m_modules.value(module);
                state.module = module;
                state.relays = bits;
                state.online = true;
                m_modules.insert(module, state);
            }
        }
    }

    void writeDebugHeartbeat(const CabinetSnapshot &snapshot)
    {
        const QDateTime now = QDateTime::currentDateTimeUtc();
        if (m_lastDebugHeartbeat.isValid() && m_lastDebugHeartbeat.msecsTo(now) < 60000)
            return;

        m_lastDebugHeartbeat = now;
        LOG_DEBUG(QStringLiteral("heartbeat: %1 / %2, state=%3")
                      .arg(modeText(snapshot.mode),
                           healthText(snapshot.health),
                           m_stateStore.filePath()));
    }

    AppConfig m_config;
    LineManager m_lineManager;
    LineOperationalMonitor m_operationalMonitor;
    MaintenanceChecker m_maintenanceChecker;
    ManualEmergencyController m_manualEmergencyController;
    TestController m_testController;
    TestScheduleManager m_scheduleManager;
    StateEngine m_stateEngine;
    StateFileStore m_stateStore;
    TestJournalStore m_journalStore;
    QVector<TestJournalEntry> m_storedJournal;

    ModbusController m_relayBus;
    MeteringBusController m_meteringBus;
    ModbusBusStatus m_relayBusStatus;
    ModbusBusStatus m_meteringBusStatus;

    QTimer m_tickTimer;
    QLocalServer m_ipcServer;
    bool m_demoMode = false;
    TestRequest m_manualFunctionalRequest;
    TestRequest m_manualDurationRequest;
    bool m_stopTestRequested = false;
    bool m_hmiManualEmergencyStartRequested = false;
    bool m_hmiManualEmergencyStopRequested = false;
    int m_hmiSetupLineIndex = 0;
    int m_testControllerDefaultDurationSec = 3600;
    PasswordManager m_passwordManager;
    QHash<int, WaveShareModuleState> m_modules;
    QHash<int, quint8> m_lastRelayBytes;
    QHash<int, double> m_linePowers;
    QHash<int, double> m_lineLeakage;

    BatterySnapshot m_battery;
    double m_inputVoltage = std::numeric_limits<double>::quiet_NaN();
    double m_inputCurrent = std::numeric_limits<double>::quiet_NaN();
    double m_inputPower = std::numeric_limits<double>::quiet_NaN();
    double m_inputFrequency = std::numeric_limits<double>::quiet_NaN();
    double m_temperature = std::numeric_limits<double>::quiet_NaN();

    CabinetMode m_lastLoggedMode = static_cast<CabinetMode>(-1);
    SystemHealth m_lastLoggedHealth = static_cast<SystemHealth>(-1);
    CabinetSnapshot m_lastSnapshot;
    QDateTime m_lastDebugHeartbeat;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QStringList args = app.arguments();
    Logger::instance().configure(defaultLogPath(),
                                 10 * 1024 * 1024,
                                 5,
                                 args.contains(QStringLiteral("--log-debug"))
                                     ? Logger::Level::Debug
                                     : Logger::Level::Info);
    Logger::instance().setConsoleOutputEnabled(args.contains(QStringLiteral("--log-debug")));
    Logger::instance().installQtMessageHandler();
    LOG_INFO(QStringLiteral("dialog-g2-engine started"));
    LOG_DEBUG(QStringLiteral("Debug log enabled"));

    const bool demoMode = args.contains(QStringLiteral("--demo"));

    AppConfig config;
    QString configError;
    const QString appConfigPath = defaultAppConfigPath();
    if (!config.load(appConfigPath, &configError)) {
        LOG_WARN(QStringLiteral("App config not loaded: %1. Using defaults").arg(configError));
        config.save(appConfigPath);
    }
    LOG_INFO(QStringLiteral("Relay RTU port: %1, baud=%2")
                 .arg(config.relayRtu().port)
                 .arg(config.relayRtu().baudRate));
    LOG_INFO(QStringLiteral("Metering RTU port: %1, baud=%2")
                 .arg(config.meteringRtu().port)
                 .arg(config.meteringRtu().baudRate));

    QString statePath = defaultStatePath();
    for (const QString &arg : args.mid(1)) {
        if (!arg.startsWith(QStringLiteral("--"))) {
            statePath = arg;
            break;
        }
    }

    EngineRuntime runtime(config, statePath, demoMode);
    runtime.start();

    return app.exec();
}
