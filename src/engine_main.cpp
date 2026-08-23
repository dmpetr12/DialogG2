#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponder>
#include <QHttpServerResponse>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMimeDatabase>
#include <QProcess>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QStringList>
#include <QStorageInfo>
#include <QTcpServer>
#include <QTimer>
#include <QUrlQuery>

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
#include "engine/ModbusTcpServer.h"
#include "engine/StateEngine.h"
#include "engine/StateFileStore.h"
#include "engine/TestController.h"
#include "engine/TestJournalStore.h"
#include "engine/TestScheduleManager.h"
#include "engine/WhdTemperatureHumidityController.h"
#include "PasswordManager.h"

using namespace DialogG2;

static constexpr int SystemLogArchiveCount = 5;
static constexpr int MaxStoredTestJournalEntries = 1000;

static QString defaultStatePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("state/current_state.json"));
}

static QString defaultRuntimeTimingPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("state/runtime_timing.json"));
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

static bool trimTestJournal(QVector<TestJournalEntry> *entries)
{
    if (!entries || entries->size() <= MaxStoredTestJournalEntries)
        return false;

    entries->erase(entries->begin(), entries->end() - MaxStoredTestJournalEntries);
    return true;
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

        if (!m_journalStore.read(&m_storedJournal, &error)) {
            LOG_WARN(QStringLiteral("Test journal not loaded: %1").arg(error));
        } else if (trimTestJournal(&m_storedJournal) && !m_journalStore.write(m_storedJournal, &error)) {
            LOG_WARN(QStringLiteral("Test journal not trimmed: %1").arg(error));
        }

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
        logPreviousRuntimeTiming();

        m_startedAt = QDateTime::currentDateTimeUtc();
        m_lastHeartbeat = m_startedAt;
        LOG_INFO(QStringLiteral("Runtime timing: startedAt=%1, lastHeartbeat=%2")
                     .arg(m_startedAt.toString(Qt::ISODate),
                          m_lastHeartbeat.toString(Qt::ISODate)));
        writeRuntimeTiming(true);

        if (m_demoMode) {
            LOG_INFO(QStringLiteral("Demo mode enabled: hardware polling is disabled"));
        } else {
            m_relayBus.connectDevice();
            m_meteringBus.connectDevice();

            m_relayBus.startPolling();
            m_meteringBus.startPolling();
        }

        m_tickTimer.start();
        startModbusTcpServer();
        startWebServer();
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

    void startModbusTcpServer()
    {
        const ModbusTcpConfig &tcp = m_config.modbusTcp();
        if (!tcp.enabled) {
            LOG_INFO(QStringLiteral("Modbus TCP server disabled"));
            return;
        }

        connect(&m_modbusTcpServer, &ModbusTcpServer::logMessage, this, [](const QString &message) {
            LOG_INFO(message);
        });
        connect(&m_modbusTcpServer, &ModbusTcpServer::manualEmergencyStartRequested, this, [this]() {
            m_hmiManualEmergencyStartRequested = true;
        });
        connect(&m_modbusTcpServer, &ModbusTcpServer::manualEmergencyStopRequested, this, [this]() {
            m_hmiManualEmergencyStopRequested = true;
        });
        connect(&m_modbusTcpServer, &ModbusTcpServer::stopTestRequested, this, [this]() {
            m_stopTestRequested = true;
        });
        connect(&m_modbusTcpServer, &ModbusTcpServer::functionalTestRequested, this, [this]() {
            m_manualFunctionalRequest = {true, 0};
        });
        connect(&m_modbusTcpServer, &ModbusTcpServer::durationTestRequested, this, [this]() {
            m_manualDurationRequest = {true, m_testControllerDefaultDurationSec};
        });

        if (!m_modbusTcpServer.start(tcp.bind, tcp.port, tcp.serverAddress))
            LOG_WARN(QStringLiteral("Modbus TCP server not started"));
    }

    void startWebServer()
    {
        const WebServerConfig &web = m_config.webServer();
        if (!web.enabled) {
            LOG_INFO(QStringLiteral("Web server disabled"));
            return;
        }

        setupWebRoutes();

        auto *tcpServer = new QTcpServer(this);
        const QHostAddress address(web.bind.trimmed().isEmpty()
                                       ? QStringLiteral("0.0.0.0")
                                       : web.bind);
        if (!tcpServer->listen(address, static_cast<quint16>(web.port))) {
            LOG_WARN(QStringLiteral("Web server listen failed on %1:%2: %3")
                         .arg(web.bind)
                         .arg(web.port)
                         .arg(tcpServer->errorString()));
            tcpServer->deleteLater();
            return;
        }

        m_webRoot = QDir::isAbsolutePath(web.root)
            ? QDir::cleanPath(web.root)
            : QDir(QCoreApplication::applicationDirPath()).filePath(web.root);
        m_webServer.bind(tcpServer);
        LOG_INFO(QStringLiteral("Web server started: http://%1:%2, root=%3")
                     .arg(web.bind)
                     .arg(web.port)
                     .arg(m_webRoot));
    }

    void setupWebRoutes()
    {
        m_webServer.route(QStringLiteral("/api/health"), [this]() {
            return webJsonOk({{QStringLiteral("status"), QStringLiteral("ok")}});
        });

        m_webServer.route(QStringLiteral("/api/state"), [this]() {
            return webJsonOk({{QStringLiteral("data"), stateForHmi()}});
        });

        m_webServer.route(QStringLiteral("/api/lines"), [this]() {
            QJsonArray lines;
            for (int i = 0; i < m_lineManager.lines().size(); ++i)
                lines.append(lineAtForHmi(i));
            return webJsonOk({{QStringLiteral("data"), lines}});
        });

        m_webServer.route(QStringLiteral("/api/lines/<arg>"), [this](int index) {
            const QJsonObject line = lineAtForHmi(index);
            if (line.isEmpty())
                return webJsonError(QStringLiteral("line not found"), QHttpServerResponder::StatusCode::NotFound);
            return webJsonOk({{QStringLiteral("data"), line}});
        });

        m_webServer.route(QStringLiteral("/api/login"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            QJsonObject body;
            if (!webRequestJson(request, &body))
                return webJsonError(QStringLiteral("invalid json body"));

            const QString password = body.value(QStringLiteral("password")).toString();
            if (password.isEmpty() || m_passwordManager.password() != password)
                return webJsonError(QStringLiteral("invalid password"), QHttpServerResponder::StatusCode::Unauthorized);

            const QByteArray seed = password.toUtf8()
                + QByteArray::number(QDateTime::currentMSecsSinceEpoch())
                + QByteArray::number(QRandomGenerator::global()->generate64());
            m_webAuthToken = QString::fromLatin1(
                QCryptographicHash::hash(seed, QCryptographicHash::Sha256).toHex());
            return webJsonOk({{QStringLiteral("token"), m_webAuthToken}});
        });

        m_webServer.route(QStringLiteral("/api/manual-emergency/start"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            m_hmiManualEmergencyStartRequested = true;
            return webJsonOk();
        });

        m_webServer.route(QStringLiteral("/api/manual-emergency/stop"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            m_hmiManualEmergencyStopRequested = true;
            return webJsonOk();
        });

        m_webServer.route(QStringLiteral("/api/test/stop"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            m_stopTestRequested = true;
            return webJsonOk();
        });

        m_webServer.route(QStringLiteral("/api/test/start-functional"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            QJsonObject body;
            webRequestJson(request, &body);
            const int warmupSec = std::clamp(body.value(QStringLiteral("warmupSec")).toInt(0), 0, 59 * 60);
            m_manualFunctionalRequest = {true, warmupSec};
            return webJsonOk();
        });

        m_webServer.route(QStringLiteral("/api/test/start-duration"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            QJsonObject body;
            webRequestJson(request, &body);
            const int durationSec = std::clamp(body.value(QStringLiteral("durationSec")).toInt(m_testControllerDefaultDurationSec),
                                               1,
                                               3 * 3600);
            m_manualDurationRequest = {true, durationSec};
            return webJsonOk();
        });

        m_webServer.route(QStringLiteral("/api/journal"), [this]() {
            return webJsonOk({{QStringLiteral("data"), journalForHmi()}});
        });

        m_webServer.route(QStringLiteral("/api/logs"), [this](const QHttpServerRequest &request) {
            const QUrlQuery query(request.url());
            const int offset = query.queryItemValue(QStringLiteral("offset")).toInt();
            const int limit = query.queryItemValue(QStringLiteral("limit")).toInt();
            return webJsonOk({{QStringLiteral("data"), logsForHmi(offset, limit > 0 ? limit : 200)}});
        });

        m_webServer.route(QStringLiteral("/api/schedule"), [this]() {
            QJsonArray entries;
            for (const TestScheduleEntry &entry : m_scheduleManager.entries())
                entries.append(scheduleEntryToHmi(entry));
            return webJsonOk({{QStringLiteral("data"), entries}});
        });

        m_webServer.route(QStringLiteral("/api/schedule/add"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            QJsonObject body;
            if (!webRequestJson(request, &body))
                return webJsonError(QStringLiteral("invalid json body"));

            QString error;
            const TestScheduleEntry entry = scheduleEntryFromHmi(body);
            const bool ok = m_scheduleManager.addEntry(entry, &error)
                && m_scheduleManager.save(defaultTestSchedulePath(), &error);
            if (!ok)
                return webJsonError(error.isEmpty() ? QStringLiteral("schedule add failed") : error);
            return webJsonOk();
        });

        m_webServer.route(QStringLiteral("/api/schedule/<arg>/update"), QHttpServerRequest::Method::Post,
                          [this](int index, const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            QJsonObject body;
            if (!webRequestJson(request, &body))
                return webJsonError(QStringLiteral("invalid json body"));

            bool ok = true;
            QString error;
            for (auto it = body.constBegin(); it != body.constEnd(); ++it)
                ok = ok && m_scheduleManager.updateEntryProperty(index, it.key(), it.value().toVariant(), &error);
            ok = ok && m_scheduleManager.save(defaultTestSchedulePath(), &error);
            return ok ? webJsonOk() : webJsonError(error.isEmpty() ? QStringLiteral("schedule update failed") : error);
        });

        m_webServer.route(QStringLiteral("/api/schedule/<arg>/remove"), QHttpServerRequest::Method::Post,
                          [this](int index, const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            QString error;
            const bool ok = m_scheduleManager.removeEntry(index, &error)
                && m_scheduleManager.save(defaultTestSchedulePath(), &error);
            return ok ? webJsonOk() : webJsonError(error.isEmpty() ? QStringLiteral("schedule remove failed") : error);
        });

        m_webServer.route(QStringLiteral("/api/password/change"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            QJsonObject body;
            if (!webRequestJson(request, &body))
                return webJsonError(QStringLiteral("invalid json body"));
            const QString password = body.value(QStringLiteral("password")).toString().trimmed();
            if (password.isEmpty())
                return webJsonError(QStringLiteral("empty password"));
            m_passwordManager.setPassword(password);
            m_webAuthToken.clear();
            return webJsonOk();
        });

        m_webServer.route(QStringLiteral("/api/system/time"), QHttpServerRequest::Method::Post,
                          [this](const QHttpServerRequest &request) {
            if (!webCheckAuth(request))
                return webUnauthorized();
            QJsonObject body;
            if (!webRequestJson(request, &body))
                return webJsonError(QStringLiteral("invalid json body"));
            const qint64 msec = body.value(QStringLiteral("msec")).toVariant().toLongLong();
            return setSystemTimeFromHmi(msec)
                ? webJsonOk()
                : webJsonError(QStringLiteral("failed to set system time"));
        });

        m_webServer.route(QStringLiteral("/"), [this]() {
            return webStaticFile(QStringLiteral("index.html"));
        });

        m_webServer.route(QStringLiteral("/<arg>"), [this](const QString &fileName) {
            return webStaticFile(fileName);
        });
    }

    QHttpServerResponse webJsonOk(const QJsonObject &payload = {}) const
    {
        QJsonObject object = payload;
        object.insert(QStringLiteral("ok"), true);
        return QHttpServerResponse(object);
    }

    QHttpServerResponse webJsonError(const QString &message,
                                     QHttpServerResponder::StatusCode status = QHttpServerResponder::StatusCode::BadRequest) const
    {
        const QJsonObject object = {
            {QStringLiteral("ok"), false},
            {QStringLiteral("error"), message}
        };
        return QHttpServerResponse(QJsonDocument(object).toJson(QJsonDocument::Compact),
                                   QByteArrayLiteral("application/json"),
                                   status);
    }

    QHttpServerResponse webUnauthorized() const
    {
        return webJsonError(QStringLiteral("unauthorized"), QHttpServerResponder::StatusCode::Unauthorized);
    }

    bool webRequestJson(const QHttpServerRequest &request, QJsonObject *object) const
    {
        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(request.body(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject())
            return false;
        if (object)
            *object = doc.object();
        return true;
    }

    QString webBearerToken(const QHttpServerRequest &request) const
    {
        const QByteArray authorization = request.value(QByteArrayLiteral("Authorization"));
        const QByteArray prefix = QByteArrayLiteral("Bearer ");
        if (!authorization.startsWith(prefix))
            return {};
        return QString::fromLatin1(authorization.mid(prefix.size()));
    }

    bool webCheckAuth(const QHttpServerRequest &request) const
    {
        return !m_webAuthToken.isEmpty() && webBearerToken(request) == m_webAuthToken;
    }

    QHttpServerResponse webStaticFile(const QString &fileName) const
    {
        QString safeName = fileName;
        if (safeName.isEmpty() || safeName == QStringLiteral("/"))
            safeName = QStringLiteral("index.html");
        if (safeName.contains(QStringLiteral(".."))
            || safeName.startsWith(QLatin1Char('/'))
            || safeName.startsWith(QLatin1Char('\\'))) {
            return QHttpServerResponse(QByteArrayLiteral("not found"),
                                       QHttpServerResponder::StatusCode::NotFound);
        }

        const QString fullPath = QDir(m_webRoot).filePath(safeName);
        QFileInfo info(fullPath);
        if (!info.exists() || !info.isFile())
            return QHttpServerResponse(QByteArrayLiteral("not found"),
                                       QHttpServerResponder::StatusCode::NotFound);

        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly))
            return QHttpServerResponse(QByteArrayLiteral("cannot open file"),
                                       QHttpServerResponder::StatusCode::InternalServerError);

        QMimeDatabase mimeDatabase;
        const QByteArray mime = mimeDatabase.mimeTypeForFile(info).name().toUtf8();
        return QHttpServerResponse(mime, file.readAll(), QHttpServerResponder::StatusCode::Ok);
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
            if (ok)
                markMaintenanceDirty();
            else
                LOG_WARN(QStringLiteral("Line not added: %1").arg(error));
            sendIpcJson(socket, {{QStringLiteral("ok"), ok}});
            return;
        }

        if (command == QStringLiteral("removeLine")) {
            const int index = request.value(QStringLiteral("index")).toInt(-1);
            sendIpcJson(socket, {{QStringLiteral("ok"), removeLineFromHmi(index)}});
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
            const bool validLine = hmiIndex >= 0 && hmiIndex < m_lineManager.lines().size();
            const int lineIndex = validLine ? m_lineManager.lines().at(hmiIndex).index : 0;
            if (request.value(QStringLiteral("active")).toBool(false) && validLine)
                m_hmiSetupLineIndex = lineIndex;
            else if (m_hmiSetupLineIndex == lineIndex || hmiIndex < 0)
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

        if (command == QStringLiteral("exportSystemLog")) {
            int exportedCount = 0;
            QString error;
            const bool ok = exportSystemLogsToUsb(&exportedCount, &error);
            sendIpcJson(socket, ok
                                    ? QJsonObject{{QStringLiteral("ok"), true}, {QStringLiteral("fileCount"), exportedCount}}
                                    : errorResponse(error));
            return;
        }

        if (command == QStringLiteral("exportTestJournal")) {
            QString exportedFileName;
            QString error;
            const bool ok = exportFileToUsb(defaultTestJournalPath(), QStringLiteral("test-journal"), QStringLiteral("json"), &exportedFileName, &error);
            sendIpcJson(socket, ok
                                    ? QJsonObject{{QStringLiteral("ok"), true}, {QStringLiteral("fileName"), exportedFileName}}
                                    : errorResponse(error));
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
        if (hmiIndex < 0 || hmiIndex >= m_lineManager.lines().size())
            return {};

        const LineConfig *line = &m_lineManager.lines().at(hmiIndex);
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
        if (hmiIndex < 0 || hmiIndex >= m_lineManager.lines().size())
            return false;

        const LineConfig *existing = &m_lineManager.lines().at(hmiIndex);
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
        if (ok)
            markMaintenanceDirty();
        else
            LOG_WARN(QStringLiteral("Line update failed: %1").arg(error));
        return ok;
    }

    bool removeLineFromHmi(int hmiIndex)
    {
        if (hmiIndex < 0 || hmiIndex >= m_lineManager.lines().size())
            return false;

        const int lineIndex = m_lineManager.lines().at(hmiIndex).index;
        if (m_hmiSetupLineIndex == lineIndex)
            m_hmiSetupLineIndex = 0;

        QString error;
        const bool ok = m_lineManager.removeLine(lineIndex, &error)
            && m_lineManager.saveConfig(defaultLinesConfigPath(), &error);
        if (ok) {
            markMaintenanceDirty();
        } else {
            LOG_WARN(QStringLiteral("Line remove failed: %1").arg(error));
        }
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
            {QStringLiteral("testKind"), static_cast<int>(m_lastSnapshot.testKind)},
            {QStringLiteral("testKindCode"), testKindCode(m_lastSnapshot.testKind)},
            {QStringLiteral("testKindText"), testKindText(m_lastSnapshot.testKind)},
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
            {QStringLiteral("maintenance"), toJson(m_lastSnapshot.maintenance)},
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
            {QStringLiteral("logLevel"), m_config.logging().level}
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
        for (int i = end - 1; i >= start; --i) {
            const QString line = QString::fromUtf8(allLines.at(i)).trimmed();
            if (!line.isEmpty())
                lines.append(line);
        }
        return lines;
    }

    QString usbMountPath() const
    {
        const QString applicationRoot = QFileInfo(QCoreApplication::applicationDirPath()).absoluteDir().rootPath();
        const auto volumes = QStorageInfo::mountedVolumes();
        for (const QStorageInfo &volume : volumes) {
            if (!volume.isValid() || !volume.isReady() || volume.isReadOnly())
                continue;

            const QString rootPath = QDir::cleanPath(volume.rootPath());
            const bool likelyUnixUsb = rootPath.startsWith(QStringLiteral("/media/"))
                || rootPath.startsWith(QStringLiteral("/run/media/"))
                || rootPath.startsWith(QStringLiteral("/mnt/"));
            const bool likelyWindowsUsb = rootPath.endsWith(QStringLiteral(":/")) && rootPath != applicationRoot;
            if (likelyUnixUsb || likelyWindowsUsb)
                return rootPath;
        }

        return {};
    }

    QStringList systemLogPaths() const
    {
        QStringList paths;
        const QString logPath = defaultLogPath();
        paths.append(logPath);

        const QFileInfo info(logPath);
        const QString dir = info.absolutePath();
        const QString baseName = info.completeBaseName();
        const QString suffix = info.suffix().isEmpty() ? QStringLiteral("log") : info.suffix();
        for (int index = 1; index <= SystemLogArchiveCount; ++index)
            paths.append(QDir(dir).filePath(QStringLiteral("%1_%2.%3").arg(baseName).arg(index).arg(suffix)));

        return paths;
    }

    bool exportSystemLogsToUsb(int *exportedCount, QString *error) const
    {
        const QString mountPath = usbMountPath();
        if (mountPath.isEmpty()) {
            if (error)
                *error = QStringLiteral("Флешка не найдена");
            return false;
        }

        const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
        int count = 0;
        const QStringList paths = systemLogPaths();
        for (int index = 0; index < paths.size(); ++index) {
            const QString sourcePath = paths.at(index);
            if (!QFile::exists(sourcePath))
                continue;

            const QString fileName = index == 0
                ? QStringLiteral("system-log_%1.log").arg(timestamp)
                : QStringLiteral("system-log_%1_%2.log").arg(timestamp).arg(index);
            const QString destinationPath = QDir(mountPath).filePath(fileName);
            QFile::remove(destinationPath);
            if (!QFile::copy(sourcePath, destinationPath)) {
                if (error)
                    *error = QStringLiteral("Ошибка записи на флешку");
                return false;
            }

            ++count;
            LOG_INFO(QStringLiteral("Exported %1 to USB: %2").arg(sourcePath, destinationPath));
        }

        if (count == 0) {
            if (error)
                *error = QStringLiteral("Файлы системного лога не найдены");
            return false;
        }

        if (exportedCount)
            *exportedCount = count;
        return true;
    }

    bool exportFileToUsb(const QString &sourcePath,
                         const QString &filePrefix,
                         const QString &extension,
                         QString *exportedFileName,
                         QString *error) const
    {
        if (!QFile::exists(sourcePath)) {
            if (error)
                *error = QStringLiteral("Файл лога не найден");
            return false;
        }

        const QString mountPath = usbMountPath();
        if (mountPath.isEmpty()) {
            if (error)
                *error = QStringLiteral("Флешка не найдена");
            return false;
        }

        const QString fileName = QStringLiteral("%1_%2.%3")
            .arg(filePrefix, QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")), extension);
        const QString destinationPath = QDir(mountPath).filePath(fileName);

        QFile::remove(destinationPath);
        if (!QFile::copy(sourcePath, destinationPath)) {
            if (error)
                *error = QStringLiteral("Ошибка записи на флешку");
            return false;
        }

        if (exportedFileName)
            *exportedFileName = fileName;
        LOG_INFO(QStringLiteral("Exported %1 to USB: %2").arg(sourcePath, destinationPath));
        return true;
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
        const MaintenanceSnapshot maintenance = maintenanceSnapshot(snapshotLines, now);

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
        m_modbusTcpServer.updateSnapshot(snapshot);
        writeState(snapshot);
        driveRelays(snapshot, lineInputs.modules);
        rememberHeartbeat();
    }

    void persistTestResults(const QVector<TestJournalEntry> &entries)
    {
        if (entries.isEmpty())
            return;

        m_storedJournal += entries;
        trimTestJournal(&m_storedJournal);

        QString error;
        if (!m_journalStore.write(m_storedJournal, &error)) {
            LOG_WARN(QStringLiteral("Test journal not saved: %1").arg(error));
            return;
        }

        m_lineManager.applyTestResults(entries);
        markMaintenanceDirty();
        if (!m_lineManager.saveConfig(defaultLinesConfigPath(), &error))
            LOG_WARN(QStringLiteral("Line test results not saved: %1").arg(error));
    }

    MaintenanceSnapshot maintenanceSnapshot(const QVector<LineSnapshot> &lines, const QDateTime &now)
    {
        if (!m_maintenanceDirty && m_nextMaintenanceCheckAt.isValid() && now < m_nextMaintenanceCheckAt)
            return m_cachedMaintenance;

        m_cachedMaintenance = m_maintenanceChecker.evaluate(lines, m_storedJournal, now);
        m_nextMaintenanceCheckAt = now.addDays(1);
        m_maintenanceDirty = false;
        LOG_INFO(QStringLiteral("Maintenance check: %1").arg(m_cachedMaintenance.summary));
        return m_cachedMaintenance;
    }

    void markMaintenanceDirty()
    {
        m_maintenanceDirty = true;
        m_nextMaintenanceCheckAt = {};
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

    void rememberHeartbeat()
    {
        m_lastHeartbeat = QDateTime::currentDateTimeUtc();
        writeRuntimeTiming(false);
    }

    void logPreviousRuntimeTiming() const
    {
        QFile file(defaultRuntimeTimingPath());
        if (!file.open(QIODevice::ReadOnly))
            return;

        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            LOG_WARN(QStringLiteral("Previous runtime timing not readable: %1").arg(error.errorString()));
            return;
        }

        const QJsonObject object = doc.object();
        LOG_INFO(QStringLiteral("Previous runtime timing: startedAt=%1, lastHeartbeat=%2")
                     .arg(object.value(QStringLiteral("startedAt")).toString(QStringLiteral("-")),
                          object.value(QStringLiteral("lastHeartbeat")).toString(QStringLiteral("-"))));
    }

    void writeRuntimeTiming(bool force)
    {
        if (!force && m_lastRuntimeTimingWrite.isValid() && m_lastRuntimeTimingWrite.msecsTo(m_lastHeartbeat) < 60000)
            return;

        const QString filePath = defaultRuntimeTimingPath();
        const QFileInfo info(filePath);
        if (!QDir().mkpath(info.absolutePath())) {
            LOG_WARN(QStringLiteral("Runtime timing directory not created: %1").arg(info.absolutePath()));
            return;
        }

        const QJsonObject object = {
            {QStringLiteral("schemaVersion"), 1},
            {QStringLiteral("startedAt"), m_startedAt.toString(Qt::ISODate)},
            {QStringLiteral("lastHeartbeat"), m_lastHeartbeat.toString(Qt::ISODate)}
        };

        QSaveFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            LOG_WARN(QStringLiteral("Runtime timing not opened: %1").arg(file.errorString()));
            return;
        }

        file.write(QJsonDocument(object).toJson(QJsonDocument::Compact));
        if (!file.commit()) {
            LOG_WARN(QStringLiteral("Runtime timing not saved: %1").arg(file.errorString()));
            return;
        }

        m_lastRuntimeTimingWrite = m_lastHeartbeat;
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
    MaintenanceSnapshot m_cachedMaintenance;
    QDateTime m_nextMaintenanceCheckAt;
    bool m_maintenanceDirty = true;

    ModbusController m_relayBus;
    MeteringBusController m_meteringBus;
    ModbusTcpServer m_modbusTcpServer;
    ModbusBusStatus m_relayBusStatus;
    ModbusBusStatus m_meteringBusStatus;

    QTimer m_tickTimer;
    QLocalServer m_ipcServer;
    QHttpServer m_webServer;
    QString m_webRoot;
    QString m_webAuthToken;
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
    QDateTime m_startedAt;
    QDateTime m_lastHeartbeat;
    QDateTime m_lastRuntimeTimingWrite;
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
