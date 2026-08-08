#include "engine/Adl200Meter.h"
#include "engine/Amc16zFak24Meter.h"
#include "engine/Asj60Ld16aMonitor.h"
#include "engine/JbdBmsProtocol.h"
#include "engine/TestController.h"
#include "engine/TestJournalStore.h"
#include "engine/TestScheduleManager.h"
#include "engine/LineManager.h"
#include "engine/LineOperationalMonitor.h"
#include "engine/ManualEmergencyController.h"
#include "engine/ModbusRtuCodec.h"
#include "engine/WhdTemperatureHumidityController.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryDir>

#include <cstdio>
#include <cstring>

using namespace DialogG2;

static LineSnapshot testLine(int index, double measuredPower)
{
    LineSnapshot line;
    line.index = index;
    line.name = QStringLiteral("Линия %1").arg(index);
    line.enabled = true;
    line.nominalPower = 100.0;
    line.powerTestTolerancePercent = 5.0;
    line.outputPower = measuredPower;
    return line;
}

static bool expect(bool condition, const QString &message)
{
    if (condition)
        return true;

    fprintf(stderr, "%s\n", message.toLocal8Bit().constData());
    qCritical() << message;
    return false;
}

static bool adl200DecoderScalesRealtimeRegisters()
{
    const QVector<quint16> registers = {
        2301,
        123,
        456,
        78,
        500,
        987,
        5000
    };

    const Adl200Measurement measurement = Adl200Meter::decodeRealtimeHoldingRegisters(registers);
    return expect(measurement.valid, QStringLiteral("ADL200 measurement should be valid"))
        && expect(qFuzzyCompare(measurement.voltage, 230.1), QStringLiteral("ADL200 voltage scale should be 0.1 V"))
        && expect(qFuzzyCompare(measurement.current, 1.23), QStringLiteral("ADL200 current scale should be 0.01 A"))
        && expect(qFuzzyCompare(measurement.activePower, 456.0), QStringLiteral("ADL200 active power should be W"))
        && expect(qFuzzyCompare(measurement.frequency, 50.0), QStringLiteral("ADL200 frequency scale should be 0.01 Hz"));
}

static void putFloatRegisters(QVector<quint16> *registers, int valueIndex, float value)
{
    quint32 raw = 0;
    std::memcpy(&raw, &value, sizeof(raw));
    (*registers)[valueIndex * 2] = static_cast<quint16>((raw >> 16) & 0xFFFF);
    (*registers)[valueIndex * 2 + 1] = static_cast<quint16>(raw & 0xFFFF);
}

static bool amc16zFak24DecoderScalesBranchPowers()
{
    QVector<quint16> registers(Amc16zFak24Meter::ActivePowerHoldingCount, 0);
    putFloatRegisters(&registers, 0, 0.125f);
    putFloatRegisters(&registers, 23, 2.5f);

    const QVector<Amc16zBranchMeasurement> measurements =
        Amc16zFak24Meter::decodeActivePowerHoldingRegisters(registers);

    return expect(measurements.size() == Amc16zFak24Meter::ActivePowerBranchCount,
                  QStringLiteral("AMC16Z-FAK24 should decode 24 branch power values"))
        && expect(measurements.first().channel == 1,
                  QStringLiteral("AMC16Z-FAK24 first decoded channel should be 1"))
        && expect(qFuzzyCompare(measurements.first().activePower, 125.0),
                  QStringLiteral("AMC16Z-FAK24 active power should be converted from kW to W"))
        && expect(measurements.last().channel == 24,
                  QStringLiteral("AMC16Z-FAK24 last decoded channel should be 24"))
        && expect(qFuzzyCompare(measurements.last().activePower, 2500.0),
                  QStringLiteral("AMC16Z-FAK24 channel 24 power should be decoded"));
}

static bool asj60Ld16aDecoderReadsChannelStatusesAndLeakage()
{
    QVector<quint16> registers(Asj60Ld16aMonitor::ChannelDataHoldingCount, 0);
    registers[0] = 0;
    registers[1] = 12;
    registers[2] = 1;
    registers[3] = 123;
    registers[30] = 2;
    registers[31] = 30000;

    const QVector<Asj60LeakageChannel> channels =
        Asj60Ld16aMonitor::decodeChannelHoldingRegisters(registers);

    return expect(channels.size() == Asj60Ld16aMonitor::ChannelCount,
                  QStringLiteral("ASJ60-LD16A should decode 16 leakage channels"))
        && expect(channels.first().channel == 1,
                  QStringLiteral("ASJ60-LD16A first decoded channel should be 1"))
        && expect(channels.first().status == Asj60ChannelStatus::Normal,
                  QStringLiteral("ASJ60-LD16A status 0 should be normal"))
        && expect(qFuzzyCompare(channels.first().leakageCurrent, 12.0),
                  QStringLiteral("ASJ60-LD16A leakage current should be mA"))
        && expect(channels.at(1).status == Asj60ChannelStatus::Warning,
                  QStringLiteral("ASJ60-LD16A status 1 should be warning"))
        && expect(channels.last().status == Asj60ChannelStatus::Alarm,
                  QStringLiteral("ASJ60-LD16A status 2 should be alarm"))
        && expect(qFuzzyCompare(channels.last().leakageCurrent, 30000.0),
                  QStringLiteral("ASJ60-LD16A channel 16 leakage should be decoded"));
}

static bool whdDecoderScalesTemperatureAndHumidity()
{
    const QVector<quint16> registers = {
        0x0120,
        0x025E
    };

    const WhdMeasurement measurement =
        WhdTemperatureHumidityController::decodeChannel1RealtimeRegisters(registers);

    return expect(measurement.valid, QStringLiteral("WHD measurement should be valid"))
        && expect(qFuzzyCompare(measurement.temperature, 28.8),
                  QStringLiteral("WHD temperature scale should be 0.1 C"))
        && expect(qFuzzyCompare(measurement.humidity, 60.6),
                  QStringLiteral("WHD humidity scale should be 0.1 %RH"));
}

static bool jbdBmsBuildsReadCommands()
{
    return expect(JbdBmsProtocol::buildReadCommand(JbdBmsProtocol::CommandBasicInfo).toHex(' ').toUpper()
                      == QByteArrayLiteral("DD A5 03 00 FF FD 77"),
                  QStringLiteral("JBD BMS basic info request should match protocol example"))
        && expect(JbdBmsProtocol::buildReadCommand(JbdBmsProtocol::CommandCellVoltages).toHex(' ').toUpper()
                      == QByteArrayLiteral("DD A5 04 00 FF FC 77"),
                  QStringLiteral("JBD BMS cell voltage request should match protocol example"));
}

static bool jbdBmsDecodesBasicInfoFrame()
{
    const QByteArray frame = QByteArray::fromHex(
        "DD03001B1700000002D003E8000020780000000000001048030F020B760B82AABBCCFBFF77");

    JbdBmsResponse response;
    QString error;
    const bool parsed = JbdBmsProtocol::parseResponse(frame, &response, &error);
    const BatterySnapshot battery = JbdBmsProtocol::decodeBasicInfo(response.data);

    return expect(parsed, QStringLiteral("JBD BMS basic info response should parse: %1").arg(error))
        && expect(response.command == JbdBmsProtocol::CommandBasicInfo,
                  QStringLiteral("JBD BMS response command should be 0x03"))
        && expect(response.callbackId == QByteArray::fromHex("AABBCC"),
                  QStringLiteral("JBD BMS parser should preserve callback id"))
        && expect(qFuzzyCompare(battery.voltage, 58.88),
                  QStringLiteral("JBD BMS total voltage should be converted from 10 mV"))
        && expect(qFuzzyCompare(battery.remainingCapacityAh, 7.2),
                  QStringLiteral("JBD BMS remaining capacity should be converted from 10 mAh"))
        && expect(qFuzzyCompare(battery.nominalCapacityAh, 10.0),
                  QStringLiteral("JBD BMS nominal capacity should be converted from 10 mAh"))
        && expect(battery.socPercent == 72,
                  QStringLiteral("JBD BMS RSOC should be decoded"))
        && expect(battery.cellCount == 15,
                  QStringLiteral("JBD BMS cell count should be decoded"))
        && expect(battery.temperatures.size() == 2,
                  QStringLiteral("JBD BMS NTC count should control temperature decoding"))
        && expect(qFuzzyCompare(battery.temperatures.first(), 20.3),
                  QStringLiteral("JBD BMS first NTC should be converted from 0.1 K"));
}

static bool jbdBmsDecodesCellVoltageFrame()
{
    const QByteArray frame = QByteArray::fromHex(
        "DD04001E0F660F630F630F640F3E0F630F370F5B0F650F3B0F630F630F3C0F660F3DF9F977");

    JbdBmsResponse response;
    QString error;
    BatterySnapshot battery;
    const bool parsed = JbdBmsProtocol::parseResponse(frame, &response, &error);
    JbdBmsProtocol::applyCellVoltages(&battery, response.data);

    return expect(parsed, QStringLiteral("JBD BMS cell voltage response should parse: %1").arg(error))
        && expect(response.command == JbdBmsProtocol::CommandCellVoltages,
                  QStringLiteral("JBD BMS response command should be 0x04"))
        && expect(battery.cellVoltages.size() == 15,
                  QStringLiteral("JBD BMS should decode one voltage per 2 bytes"))
        && expect(qFuzzyCompare(battery.cellVoltages.first(), 3.942),
                  QStringLiteral("JBD BMS cell voltage should be converted from mV"))
        && expect(qFuzzyCompare(battery.minCellVoltage, 3.895),
                  QStringLiteral("JBD BMS min cell voltage should be calculated"))
        && expect(qFuzzyCompare(battery.maxCellVoltage, 3.942),
                  QStringLiteral("JBD BMS max cell voltage should be calculated"));
}

static bool modbusRtuCodecBuildsReadRequest()
{
    return expect(ModbusRtuCodec::readRequest(1, 0x03, 0x000B, 7).toHex(' ').toUpper()
                      == QByteArrayLiteral("01 03 00 0B 00 07 75 CA"),
                  QStringLiteral("Modbus RTU read request should include CRC low byte first"));
}

static bool modbusRtuCodecDecodesRegisterResponse()
{
    QByteArray frame;
    frame.append(char(0x01));
    frame.append(char(0x03));
    frame.append(char(0x04));
    frame.append(char(0x01));
    frame.append(char(0x20));
    frame.append(char(0x02));
    frame.append(char(0x5E));
    frame.append(char(0x7A));
    frame.append(char(0x9D));

    const QVector<quint16> values = ModbusRtuCodec::registersFromReadResponse(frame);
    return expect(ModbusRtuCodec::validateCrc(frame),
                  QStringLiteral("Modbus RTU response CRC should validate"))
        && expect(values.size() == 2,
                  QStringLiteral("Modbus RTU response should decode two registers"))
        && expect(values.first() == 0x0120 && values.last() == 0x025E,
                  QStringLiteral("Modbus RTU registers should be big-endian"));
}

static bool testScheduleStartsDailyFunctionalOnce()
{
    TestScheduleManager manager;
    TestScheduleEntry entry;
    entry.enabled = true;
    entry.period = QStringLiteral("ежедневно");
    entry.startDate = QDate(2026, 8, 1);
    entry.startTime = QTime(10, 30);
    entry.testType = QStringLiteral("Функциональный тест");

    QString error;
    if (!expect(manager.addEntry(entry, &error), QStringLiteral("schedule entry should be accepted: %1").arg(error)))
        return false;

    const QDateTime now(QDate(2026, 8, 9), QTime(10, 30, 30));
    const TestScheduleRequest first = manager.evaluate(now);
    const TestScheduleRequest second = manager.evaluate(now.addSecs(10));

    return expect(first.functional.active, QStringLiteral("daily schedule should request functional test in window"))
        && expect(!second.functional.active && !second.duration.active,
                  QStringLiteral("daily schedule should not trigger twice for same planned time"));
}

static bool testScheduleStartsWeekdayDuration()
{
    TestScheduleManager manager;
    TestScheduleEntry entry;
    entry.enabled = true;
    entry.period = QStringLiteral("дни недели");
    entry.startDate = QDate(2026, 8, 1);
    entry.startTime = QTime(11, 0);
    entry.testType = QStringLiteral("Тест на время");
    entry.weekDays = {QStringLiteral("Sun")};

    QString error;
    if (!expect(manager.addEntry(entry, &error), QStringLiteral("weekday schedule entry should be accepted: %1").arg(error)))
        return false;

    const TestScheduleRequest request = manager.evaluate(QDateTime(QDate(2026, 8, 9), QTime(11, 1, 0)));
    return expect(request.duration.active, QStringLiteral("weekday schedule should request duration test"))
        && expect(request.entryIndex == 0, QStringLiteral("weekday schedule should report entry index"));
}

static bool testSchedulePersistsLegacyArrayFormat()
{
    QTemporaryDir dir;
    if (!expect(dir.isValid(), QStringLiteral("temporary directory should be valid")))
        return false;

    TestScheduleManager manager;
    TestScheduleEntry entry;
    entry.period = QStringLiteral("раз в месяц");
    entry.startDate = QDate(2026, 8, 9);
    entry.startTime = QTime(12, 15);
    entry.testType = QStringLiteral("functional");

    QString error;
    const QString path = dir.filePath(QStringLiteral("schedule.json"));
    if (!expect(manager.addEntry(entry, &error), QStringLiteral("monthly schedule should be accepted: %1").arg(error))
        || !expect(manager.save(path, &error), QStringLiteral("schedule should save: %1").arg(error))) {
        return false;
    }

    QFile file(path);
    if (!expect(file.open(QIODevice::ReadOnly), QStringLiteral("saved schedule should open")))
        return false;

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    return expect(parseError.error == QJsonParseError::NoError && doc.isArray(),
                  QStringLiteral("schedule should be saved as legacy JSON array"))
        && expect(doc.array().first().toObject().contains(QStringLiteral("startTime")),
                  QStringLiteral("schedule entry should contain legacy startTime field"));
}

static bool functionalTestPasses()
{
    TestControllerConfig config;
    config.functionalWarmupSeconds = 1;

    TestController controller(config);
    TestControllerInputs inputs;
    inputs.now = QDateTime::fromString(QStringLiteral("2026-08-08T08:00:00.000Z"), Qt::ISODateWithMs);
    inputs.lines = {testLine(1, 100.0), testLine(2, 104.0)};
    inputs.manualFunctional.active = true;

    TestControllerResult result = controller.evaluate(inputs);
    inputs.now = inputs.now.addSecs(2);
    result = controller.evaluate(inputs);

    return expect(result.journalEntries.size() == 1, QStringLiteral("functional test should write one journal entry"))
        && expect(result.journalEntries.first().status == TestRunStatus::Passed,
                  QStringLiteral("functional test should pass"))
        && expect(result.lines.first().lastFunctionalTest.status == TestRunStatus::Passed,
                  QStringLiteral("line should store last functional result"));
}

static bool durationTestUsesExpandedTolerance()
{
    TestControllerConfig config;
    config.defaultDurationSeconds = 1;
    config.durationToleranceMultiplier = 2.0;

    TestController controller(config);
    TestControllerInputs inputs;
    inputs.now = QDateTime::fromString(QStringLiteral("2026-08-08T08:00:00.000Z"), Qt::ISODateWithMs);
    inputs.lines = {testLine(1, 91.0), testLine(2, 89.0)};
    inputs.manualDuration.active = true;

    TestControllerResult result = controller.evaluate(inputs);
    inputs.now = inputs.now.addSecs(2);
    result = controller.evaluate(inputs);

    return expect(result.journalEntries.size() == 1, QStringLiteral("duration test should write one journal entry"))
        && expect(result.journalEntries.first().status == TestRunStatus::Failed,
                  QStringLiteral("duration test should fail when one line is outside expanded tolerance"))
        && expect(result.journalEntries.first().lines.first().status == TestRunStatus::Passed,
                  QStringLiteral("line 1 should pass within 10 percent expanded tolerance"))
        && expect(result.journalEntries.first().lines.at(1).status == TestRunStatus::Failed,
                  QStringLiteral("line 2 should fail outside 10 percent expanded tolerance"));
}

static bool testInterruptedByVoltagePriority()
{
    TestControllerConfig config;
    config.functionalWarmupSeconds = 120;

    TestController controller(config);
    TestControllerInputs inputs;
    inputs.now = QDateTime::fromString(QStringLiteral("2026-08-08T08:00:00.000Z"), Qt::ISODateWithMs);
    inputs.lines = {testLine(1, 100.0)};
    inputs.scheduledFunctional.active = true;

    TestControllerResult result = controller.evaluate(inputs);
    if (!expect(result.activeTest.active, QStringLiteral("scheduled test should start")))
        return false;

    inputs.now = inputs.now.addSecs(1);
    inputs.voltageControlOk = false;
    result = controller.evaluate(inputs);

    return expect(!result.activeTest.active, QStringLiteral("test should stop after voltage priority interruption"))
        && expect(result.journalEntries.size() == 1, QStringLiteral("interruption should write journal entry"))
        && expect(result.journalEntries.first().status == TestRunStatus::InterruptedByPriority,
                  QStringLiteral("journal status should be interrupted_by_priority"));
}

static bool testStoppedByOperator()
{
    TestControllerConfig config;
    config.functionalWarmupSeconds = 120;

    TestController controller(config);
    TestControllerInputs inputs;
    inputs.now = QDateTime::fromString(QStringLiteral("2026-08-08T08:00:00.000Z"), Qt::ISODateWithMs);
    inputs.lines = {testLine(1, 100.0)};
    inputs.manualFunctional.active = true;

    TestControllerResult result = controller.evaluate(inputs);
    if (!expect(result.activeTest.active, QStringLiteral("manual test should start before operator stop")))
        return false;

    inputs.now = inputs.now.addSecs(1);
    inputs.stopRequested = true;
    result = controller.evaluate(inputs);

    return expect(!result.activeTest.active, QStringLiteral("test should stop after operator command"))
        && expect(result.journalEntries.size() == 1, QStringLiteral("operator stop should write journal entry"))
        && expect(result.journalEntries.first().status == TestRunStatus::StoppedByOperator,
                  QStringLiteral("journal status should be stopped_by_operator"))
        && expect(result.journalEntries.first().lines.isEmpty(),
                  QStringLiteral("operator-stopped test should not write line measurements"));
}

static void setModuleInput(WaveShareModuleState *module, int channel, bool on)
{
    const quint8 mask = static_cast<quint8>(1u << (channel - 1));
    if (on)
        module->inputs = static_cast<quint8>(module->inputs | mask);
    else
        module->inputs = static_cast<quint8>(module->inputs & ~mask);
}

static bool lineManagerReadsManualStopButton()
{
    LineManager manager;
    LineManagerInputs inputs;
    WaveShareModuleState module;
    module.module = 1;
    setModuleInput(&module, 4, true);
    setModuleInput(&module, 3, true);
    inputs.modules.insert(module.module, module);

    const LineManagerResult result = manager.evaluate(inputs);
    return expect(result.manualStopButtonActive, QStringLiteral("stop button input should be active"));
}

static bool lineManagerDrivesNormallyClosedFaultLamp()
{
    LineManager manager;
    LineManagerInputs inputs;
    WaveShareModuleState module;
    module.module = 1;
    inputs.modules.insert(module.module, module);

    LineManagerResult result = manager.evaluate(inputs);
    const quint8 faultLampMask = static_cast<quint8>(1u << 1);
    if (!expect((result.relayOutputBytes.value(1) & faultLampMask) != 0,
                QStringLiteral("normally closed fault lamp relay should be energized in normal state")))
        return false;

    inputs.faultLampOn = true;
    result = manager.evaluate(inputs);
    return expect((result.relayOutputBytes.value(1) & faultLampMask) == 0,
                  QStringLiteral("normally closed fault lamp relay should drop on fault"));
}

static bool manualEmergencyControllerLatchesUntilStop()
{
    ManualEmergencyController controller;
    ManualEmergencyInputs inputs;
    inputs.startRequested = true;

    bool active = controller.evaluate(inputs);
    if (!expect(active, QStringLiteral("manual fire start should latch emergency mode")))
        return false;

    inputs.startRequested = false;
    active = controller.evaluate(inputs);
    if (!expect(active, QStringLiteral("manual fire should stay active after start request")))
        return false;

    inputs.stopRequested = true;
    active = controller.evaluate(inputs);
    return expect(!active, QStringLiteral("manual fire stop should reset emergency mode"));
}

static TestJournalEntry sampleJournalEntry()
{
    TestJournalEntry entry;
    entry.kind = TestKind::Functional;
    entry.source = TestSource::Manual;
    entry.startedAt = QDateTime::fromString(QStringLiteral("2026-08-08T08:00:00.000Z"), Qt::ISODateWithMs);
    entry.finishedAt = entry.startedAt.addSecs(120);
    entry.status = TestRunStatus::Passed;

    TestLineMeasurement line;
    line.lineIndex = 1;
    line.lineName = QStringLiteral("Линия 1");
    line.measuredPower = 100.0;
    line.nominalPower = 100.0;
    line.tolerancePercent = 5.0;
    line.status = TestRunStatus::Passed;
    line.details = QStringLiteral("мощность в допуске");
    entry.lines.append(line);
    return entry;
}

static bool journalStorePersistsEntries()
{
    QTemporaryDir dir;
    if (!expect(dir.isValid(), QStringLiteral("temporary dir should be valid")))
        return false;

    TestJournalStore store(dir.filePath(QStringLiteral("test_journal.json")));
    const QVector<TestJournalEntry> entries = {sampleJournalEntry()};

    QString error;
    if (!expect(store.append(entries, &error), QStringLiteral("journal append should succeed: %1").arg(error)))
        return false;

    QVector<TestJournalEntry> loaded;
    if (!expect(store.read(&loaded, &error), QStringLiteral("journal read should succeed: %1").arg(error)))
        return false;

    return expect(loaded.size() == 1, QStringLiteral("journal should contain one entry"))
        && expect(loaded.first().lines.size() == 1, QStringLiteral("journal entry should contain line measurement"))
        && expect(loaded.first().lines.first().status == TestRunStatus::Passed,
                  QStringLiteral("journal line result should survive roundtrip"));
}

static bool lineManagerPersistsLastTestResults()
{
    QTemporaryDir dir;
    if (!expect(dir.isValid(), QStringLiteral("temporary dir should be valid")))
        return false;

    const QString path = dir.filePath(QStringLiteral("lines.json"));
    LineManager manager;
    const QVector<TestJournalEntry> entries = {sampleJournalEntry()};
    manager.applyTestResults(entries);

    QString error;
    if (!expect(manager.saveConfig(path, &error), QStringLiteral("line config save should succeed: %1").arg(error)))
        return false;

    LineManager loaded;
    if (!expect(loaded.loadConfig(path, &error), QStringLiteral("line config load should succeed: %1").arg(error)))
        return false;

    const LineConfig *line = loaded.line(1);
    return expect(line != nullptr, QStringLiteral("line 1 should exist"))
        && expect(line->lastFunctionalTest.status == TestRunStatus::Passed,
                  QStringLiteral("last functional test should survive line config roundtrip"));
}

static bool operationalMonitorWaitsWarmup()
{
    LineOperationalMonitorConfig config;
    config.warmupSeconds = 120;
    LineOperationalMonitor monitor(config);

    const QDateTime start = QDateTime::fromString(QStringLiteral("2026-08-08T08:00:00.000Z"), Qt::ISODateWithMs);
    QVector<LineSnapshot> lines = {testLine(1, 80.0)};

    QVector<LineSnapshot> result = monitor.evaluate(lines, start);
    if (!expect(result.first().operationalCheck.state == LineOperationalState::WarmingUp,
                QStringLiteral("line should be warming up immediately after turning on")))
        return false;
    if (!expect(result.first().state == LineState::Normal,
                QStringLiteral("warming line should not be marked faulty")))
        return false;

    result = monitor.evaluate(lines, start.addSecs(121));
    return expect(result.first().operationalCheck.state == LineOperationalState::Fault,
                  QStringLiteral("line should fail after warmup when power is outside tolerance"))
        && expect(result.first().state == LineState::Fault,
                  QStringLiteral("failed operational check should mark line fault"));
}

static bool operationalMonitorIgnoresOffLine()
{
    LineOperationalMonitor monitor;
    LineSnapshot line = testLine(1, std::numeric_limits<double>::quiet_NaN());
    line.kind = LineKind::NonConstant;
    line.outputState = LineOutputState::Off;

    const QVector<LineSnapshot> result = monitor.evaluate({line}, QDateTime::currentDateTimeUtc());
    return expect(result.first().operationalCheck.state == LineOperationalState::NotChecked,
                  QStringLiteral("off non-constant line should not be checked"))
        && expect(result.first().state == LineState::Normal,
                  QStringLiteral("off non-constant line should not become faulty"));
}

static bool operationalMonitorFailsNoMeasurementAfterWarmup()
{
    LineOperationalMonitorConfig config;
    config.warmupSeconds = 1;
    LineOperationalMonitor monitor(config);

    const QDateTime start = QDateTime::fromString(QStringLiteral("2026-08-08T08:00:00.000Z"), Qt::ISODateWithMs);
    QVector<LineSnapshot> lines = {testLine(1, std::numeric_limits<double>::quiet_NaN())};

    monitor.evaluate(lines, start);
    const QVector<LineSnapshot> result = monitor.evaluate(lines, start.addSecs(2));
    return expect(result.first().operationalCheck.state == LineOperationalState::NoMeasurement,
                  QStringLiteral("line should fail with no measurement after warmup"))
        && expect(result.first().state == LineState::Fault,
                  QStringLiteral("no measurement after warmup should mark line fault"));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const bool ok = adl200DecoderScalesRealtimeRegisters()
        && amc16zFak24DecoderScalesBranchPowers()
        && asj60Ld16aDecoderReadsChannelStatusesAndLeakage()
        && whdDecoderScalesTemperatureAndHumidity()
        && jbdBmsBuildsReadCommands()
        && jbdBmsDecodesBasicInfoFrame()
        && jbdBmsDecodesCellVoltageFrame()
        && modbusRtuCodecBuildsReadRequest()
        && modbusRtuCodecDecodesRegisterResponse()
        && testScheduleStartsDailyFunctionalOnce()
        && testScheduleStartsWeekdayDuration()
        && testSchedulePersistsLegacyArrayFormat()
        && functionalTestPasses()
        && durationTestUsesExpandedTolerance()
        && testInterruptedByVoltagePriority()
        && testStoppedByOperator()
        && lineManagerReadsManualStopButton()
        && lineManagerDrivesNormallyClosedFaultLamp()
        && manualEmergencyControllerLatchesUntilStop()
        && journalStorePersistsEntries()
        && lineManagerPersistsLastTestResults()
        && operationalMonitorWaitsWarmup()
        && operationalMonitorIgnoresOffLine()
        && operationalMonitorFailsNoMeasurementAfterWarmup();

    if (ok)
        qInfo() << "TestController checks passed";

    return ok ? 0 : 1;
}
