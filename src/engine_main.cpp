#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QTimer>

#include <algorithm>
#include <cmath>

#include "engine/Adl200Meter.h"
#include "engine/Amc16zFak24Meter.h"
#include "engine/AppConfig.h"
#include "engine/Asj60Ld16aMonitor.h"
#include "engine/LineManager.h"
#include "engine/LineOperationalMonitor.h"
#include "engine/Logger.h"
#include "engine/ManualEmergencyController.h"
#include "engine/MeteringBusController.h"
#include "engine/ModbusController.h"
#include "engine/StateEngine.h"
#include "engine/StateFileStore.h"
#include "engine/TestController.h"
#include "engine/TestJournalStore.h"
#include "engine/TestScheduleManager.h"
#include "engine/WhdTemperatureHumidityController.h"

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
    EngineRuntime(AppConfig config, QString statePath, QObject *parent = nullptr)
        : QObject(parent)
        , m_config(std::move(config))
        , m_stateStore(std::move(statePath))
        , m_journalStore(defaultTestJournalPath())
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

        setupRelayBus(moduleCount);
        setupMeteringBus();

        connect(&m_tickTimer, &QTimer::timeout, this, &EngineRuntime::tick);
        m_tickTimer.setInterval(500);
    }

    void start()
    {
        m_relayBus.connectDevice();
        m_meteringBus.connectDevice();

        m_relayBus.startPolling();
        m_meteringBus.startPolling();

        m_tickTimer.start();
        tick();
    }

private:
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
        LineManagerResult lineResult = m_lineManager.evaluate(lineInputs);

        QVector<LineSnapshot> lines = lineResult.lines;
        applyMeasurements(&lines);
        lines = m_operationalMonitor.evaluate(lines, now);

        ManualEmergencyInputs manualEmergencyInputs;
        manualEmergencyInputs.startRequested = lineResult.manualFireButtonActive;
        manualEmergencyInputs.stopRequested = lineResult.manualStopButtonActive;
        const bool manualEmergencyActive = m_manualEmergencyController.evaluate(manualEmergencyInputs);

        TestControllerInputs testInputs;
        testInputs.now = now;
        testInputs.voltageControlOk = lineResult.voltageControlOk;
        testInputs.fireInputActive = lineResult.fireInputActive;
        testInputs.stopRequested = lineResult.manualStopButtonActive;
        testInputs.lines = lines;
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
        engineInputs.lines = testResult.lines;

        const CabinetSnapshot snapshot = m_stateEngine.evaluate(engineInputs);
        writeState(snapshot);
        driveRelays(snapshot, lineInputs.modules);
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

        const LineManagerResult outputResult = m_lineManager.evaluate(outputInputs);
        const QList<int> relayModules = outputResult.relayOutputBytes.keys();
        for (int module : relayModules) {
            const quint8 bits = outputResult.relayOutputBytes.value(module);
            if (m_lastRelayBytes.value(module, 0xFF) == bits)
                continue;

            m_relayBus.writeWaveShareRelayByte(module, bits);
            m_lastRelayBytes.insert(module, bits);
        }
    }

    AppConfig m_config;
    LineManager m_lineManager;
    LineOperationalMonitor m_operationalMonitor;
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
    Logger::instance().installQtMessageHandler();
    LOG_INFO(QStringLiteral("dialog-g2-engine started"));
    LOG_DEBUG(QStringLiteral("Debug log enabled"));

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

    EngineRuntime runtime(config, statePath);
    runtime.start();

    return app.exec();
}
