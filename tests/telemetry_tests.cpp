#include <QCoreApplication>
#include <QJsonObject>
#include <QModbusDevice>
#include <QModbusRtuSerialClient>
#include <QTimer>
#include <QVariantMap>
#include "PanelFacade.h"
#include "engine/MeteringBusController.h"
#include "engine/ModbusController.h"
#include <QEventLoop>
#include <cmath>
#include <cstdio>

struct TelemetryTestAccess {
    static bool relayConnectionErrorMarksBusOffline(DialogG2::ModbusController &bus) {
        bus.m_busMonitor.markSuccess();
        bus.m_client->errorOccurred(QModbusDevice::ConnectionError);
        return !bus.busStatus().online;
    }
    static QByteArray modbusFrame(int slave, int function, int count) {
        QByteArray frame;
        frame.append(char(slave)); frame.append(char(function)); frame.append(char(count * 2));
        for (int i = 0; i < count; ++i) { frame.append(char(0)); frame.append(char(i + 1)); }
        quint16 crc = 0xffff;
        for (char c : frame) {
            crc ^= static_cast<quint8>(c);
            for (int i = 0; i < 8; ++i) crc = (crc >> 1) ^ ((crc & 1) ? 0xa001 : 0);
        }
        frame.append(char(crc & 0xff)); frame.append(char(crc >> 8));
        return frame;
    }
    static QByteArray bmsFrame(int command) {
        QByteArray payload(23, char(0));
        payload[4] = char(0x77); // Stop marker can occur inside payload.
        QByteArray frame = QByteArray::fromHex("dd000017");
        frame[1] = char(command);
        frame += payload;
        quint16 sum = 23 + 0x77;
        const quint16 checksum = quint16(0 - sum);
        frame.append(char(checksum >> 8)); frame.append(char(checksum & 0xff)); frame.append(char(0x77));
        return frame;
    }
    static void feed(DialogG2::MeteringBusController &bus, const QByteArray &frame) {
        bus.m_rxBuffer += frame;
        bus.processReceiveBuffer();
    }
    static void beginBms(DialogG2::MeteringBusController &bus) {
        bus.m_currentRequest = {};
        bus.m_currentRequest.type = DialogG2::MeteringBusController::RequestType::JbdBmsBasicInfo;
        bus.m_busy = true;
        bus.m_rxBuffer.clear();
    }
    static void receiveAdl(DialogG2::MeteringBusController &bus, const QByteArray &frame) {
        bus.m_currentRequest = {};
        bus.m_currentRequest.type = DialogG2::MeteringBusController::RequestType::ReadHolding;
        bus.m_currentRequest.meterKind = DialogG2::MeteringBusController::MeterKind::Adl200Input;
        bus.m_currentRequest.slaveAddress = DialogG2::Adl200Meter::DefaultSlaveAddress;
        bus.m_currentRequest.count = 7;
        bus.m_busy = true;
        bus.m_rxBuffer = frame;
        bus.processReceiveBuffer();
    }
    static void receiveAmcPower(DialogG2::MeteringBusController &bus) {
        bus.m_currentRequest = {};
        bus.m_currentRequest.type = DialogG2::MeteringBusController::RequestType::ReadHolding;
        bus.m_currentRequest.meterKind = DialogG2::MeteringBusController::MeterKind::Amc16zFak24BranchPower;
        bus.m_currentRequest.slaveAddress = DialogG2::Amc16zFak24Meter::DefaultSlaveAddress;
        bus.m_currentRequest.start = DialogG2::Amc16zFak24Meter::ActivePowerHoldingStart;
        bus.m_currentRequest.count = DialogG2::Amc16zFak24Meter::ActivePowerHoldingCount;
        bus.m_busy = true;
        bus.m_rxBuffer = modbusFrame(bus.m_currentRequest.slaveAddress, 3, bus.m_currentRequest.count);
        bus.processReceiveBuffer();
    }
    static void seed(PanelFacade &panel) {
        panel.m_connected = true;
        panel.m_state = {{"inletU", 230.0}, {"systemOk", true}, {"batteryPercent", 80}, {"modeCode", "normal"}};
    }
    static void setVoltage(PanelFacade &panel, QJsonValue value) {
        panel.m_connected = true;
        panel.m_state = {{"inletU", value}};
    }
    static void failMeter(DialogG2::MeteringBusController &bus, int kind) {
        bus.m_currentRequest.type = DialogG2::MeteringBusController::RequestType::ReadHolding;
        bus.m_currentRequest.meterKind = static_cast<DialogG2::MeteringBusController::MeterKind>(kind);
        bus.handleRequestFailure(QStringLiteral("test timeout"));
    }
    static bool branchPollsCorrect(DialogG2::MeteringBusController &bus) {
        bus.clearPollTasks();
        bus.addAmc16zFak24BranchPowerPolling();
        const auto &tasks = bus.m_pollTasks;
        return tasks.size() == 3 && tasks[0].request.start == 0xC0
            && tasks[1].request.start == 0x30 && tasks[2].request.start == 0x90
            && tasks[0].request.count == 48 && tasks[1].request.count == 48
            && tasks[2].request.count == 48 && tasks[2].request.slaveAddress == 2
            && tasks[0].intervalMs == 2000 && tasks[1].intervalMs == 2000
            && tasks[2].intervalMs == 2000;
    }
    static bool activeRequestIsNotQueued(DialogG2::MeteringBusController &bus) {
        bus.m_queue.clear();
        bus.m_currentRequest = {};
        bus.m_currentRequest.slaveAddress = DialogG2::Adl200Meter::DefaultSlaveAddress;
        bus.m_currentRequest.start = 0x000b;
        bus.m_currentRequest.count = 7;
        bus.m_currentRequest.meterKind = DialogG2::MeteringBusController::MeterKind::Adl200Input;
        bus.m_busy = true;
        bus.enqueue(bus.m_currentRequest);
        const bool skipped = bus.m_queue.empty();
        bus.m_busy = false;
        return skipped;
    }
    static bool finishedRequestWaitsBeforeNext(DialogG2::MeteringBusController &bus) {
        bus.m_queue.clear();
        bus.m_queue.push_back({});
        bus.m_busy = true;
        bus.finishCurrentRequest();
        const bool waiting = !bus.m_busy && bus.m_queue.size() == 1
            && bus.m_interRequestTimer.isActive()
            && bus.m_interRequestTimer.interval() == 100;
        bus.m_interRequestTimer.stop();
        bus.m_queue.clear();
        return waiting;
    }
    static void batteryCellsBeforeBasic(DialogG2::MeteringBusController &bus) {
        bus.m_currentRequest.type = DialogG2::MeteringBusController::RequestType::JbdBmsCellVoltages;
        bus.invalidateRequest(bus.m_currentRequest);
    }
    static bool basicInfoKeepsCellVoltages(DialogG2::MeteringBusController &bus) {
        bus.m_battery.cellVoltages = {3.91, 3.92};
        bus.m_battery.minCellVoltage = 3.91;
        bus.m_battery.maxCellVoltage = 3.92;
        bus.m_battery.cellVoltageDelta = 0.01;
        bus.m_currentRequest.type = DialogG2::MeteringBusController::RequestType::JbdBmsBasicInfo;
        bus.m_busy = true;
        bus.handleCurrentResponse(bmsFrame(3));
        return bus.m_battery.cellVoltages == QVector<double>({3.91, 3.92})
            && bus.m_battery.minCellVoltage == 3.91
            && bus.m_battery.maxCellVoltage == 3.92
            && bus.m_battery.cellVoltageDelta == 0.01;
    }
    static void failBattery(DialogG2::MeteringBusController &bus) {
        bus.m_currentRequest.type = DialogG2::MeteringBusController::RequestType::JbdBmsBasicInfo;
        bus.handleRequestFailure(QStringLiteral("test timeout"));
    }
};

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    int failures = 0;
    auto check = [&](bool ok, const char *message) {
        if (!ok) { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
    };
    PanelFacade panel(nullptr, QStringLiteral("dialog_g2_test_%1").arg(QCoreApplication::applicationPid()));
    check(!panel.connected(), "test requires no running backend");
    check(!panel.systemOk() && !panel.batteryOk() && !panel.linesOk(), "missing state cannot be healthy");
    check(std::isnan(panel.inputVoltage()), "missing voltage must be unavailable");
    TelemetryTestAccess::seed(panel);
    check(panel.inputVoltage() == 230.0, "valid measurement preserved");
    check(panel.property("modeCode").toString() == QStringLiteral("normal"), "mode code is exposed to QML");
    TelemetryTestAccess::setVoltage(panel, QJsonValue::Null);
    check(std::isnan(panel.inputVoltage()), "JSON null is unavailable, not zero");
    TelemetryTestAccess::setVoltage(panel, 0.0);
    check(panel.inputVoltage() == 0.0, "real zero is a valid measurement");
    TelemetryTestAccess::seed(panel);
    panel.refresh();
    check(std::isnan(panel.inputVoltage()) && panel.batteryPercent() == -1, "disconnect clears cached measurements");
    TelemetryTestAccess::seed(panel);
    check(panel.inputVoltage() == 230.0 && panel.systemOk(), "fresh data restores measurements after disconnect");
    DialogG2::MeteringBusController bus;
    DialogG2::ModbusController relay;
    check(TelemetryTestAccess::relayConnectionErrorMarksBusOffline(relay),
          "relay connection loss must mark the bus offline immediately");
    check(TelemetryTestAccess::branchPollsCorrect(bus), "AMC polls separate P/U/I blocks at slave 2");
    check(TelemetryTestAccess::activeRequestIsNotQueued(bus), "active request must not be queued again");
    check(TelemetryTestAccess::finishedRequestWaitsBeforeNext(bus),
          "next metering request waits 100 ms after completion");

    {
        DialogG2::MeteringBusController lateMetering;
        DialogG2::ModbusController lateRelay;
        DialogG2::ModbusRtuConfig missingPort;
        missingPort.port = QStringLiteral("__dialog_g2_missing_serial_port__");
        lateMetering.configure(missingPort);
        lateRelay.configure(missingPort);

        int meteringAttempts = 0;
        int relayAttempts = 0;
        QObject::connect(&lateMetering, &DialogG2::MeteringBusController::connectedChanged,
                         [&](bool connected) { if (!connected) ++meteringAttempts; });
        QObject::connect(&lateRelay, &DialogG2::ModbusController::busStatusChanged,
                         [&](const auto &) { ++relayAttempts; });

        lateMetering.connectDevice();
        lateRelay.connectDevice();
        lateMetering.startPolling();
        lateRelay.startPolling();

        QEventLoop retryWait;
        QTimer::singleShot(3300, &retryWait, &QEventLoop::quit);
        retryWait.exec();

        check(meteringAttempts >= 2, "metering bus retries a port that appears after startup");
        check(relayAttempts >= 2, "relay bus retries a port that appears after startup");
    }
    int validAdlResponses = 0;
    QObject::connect(&bus, &DialogG2::MeteringBusController::adl200InputMeterUpdated,
                     [&](const auto &m) { if (m.valid) ++validAdlResponses; });
    TelemetryTestAccess::receiveAdl(bus, TelemetryTestAccess::modbusFrame(2, 3, 48));
    check(validAdlResponses == 0, "late AMC response must not become ADL power");
    TelemetryTestAccess::receiveAdl(bus, TelemetryTestAccess::modbusFrame(DialogG2::Adl200Meter::DefaultSlaveAddress, 4, 7));
    check(validAdlResponses == 0, "wrong function must not become ADL power");
    TelemetryTestAccess::receiveAdl(bus, TelemetryTestAccess::modbusFrame(DialogG2::Adl200Meter::DefaultSlaveAddress, 3, 8));
    check(validAdlResponses == 0, "wrong register count must not become ADL power");
    TelemetryTestAccess::receiveAdl(bus, TelemetryTestAccess::modbusFrame(DialogG2::Adl200Meter::DefaultSlaveAddress, 3, 7));
    check(validAdlResponses == 1, "matching ADL response accepted");
    const auto adlFrame = TelemetryTestAccess::modbusFrame(DialogG2::Adl200Meter::DefaultSlaveAddress, 3, 7);
    TelemetryTestAccess::receiveAdl(bus, TelemetryTestAccess::bmsFrame(3) + adlFrame);
    check(validAdlResponses == 2, "BMS packet before ADL is skipped without losing ADL");
    TelemetryTestAccess::receiveAdl(bus, adlFrame.left(8));
    check(validAdlResponses == 2, "partial Modbus response waits for remaining bytes");
    TelemetryTestAccess::feed(bus, adlFrame.mid(8));
    check(validAdlResponses == 3, "fragmented Modbus response accepted once complete");
    int bmsResponses = 0;
    QObject::connect(&bus, &DialogG2::MeteringBusController::jbdBmsBatteryUpdated,
                     [&](const auto &) { ++bmsResponses; });
    TelemetryTestAccess::beginBms(bus);
    TelemetryTestAccess::feed(bus, TelemetryTestAccess::bmsFrame(4));
    check(bmsResponses == 0, "wrong BMS command cannot complete current request");
    const auto basic = TelemetryTestAccess::bmsFrame(3);
    TelemetryTestAccess::feed(bus, basic.left(9));
    check(bmsResponses == 0, "payload 0x77 must not terminate BMS frame early");
    TelemetryTestAccess::feed(bus, basic.mid(9));
    check(bmsResponses == 1, "complete matching BMS response accepted");
    check(TelemetryTestAccess::basicInfoKeepsCellVoltages(bus), "basic BMS refresh must keep the last cell-voltage sample");
    bus.disconnectDevice(); // Reset received BMS state before startup/invalidation scenarios.
    bool voltageInvalid = false, currentInvalid = false;
    QObject::connect(&bus, &DialogG2::MeteringBusController::amc16zBranchVoltagesUpdated, [&](const auto &v) { voltageInvalid = v.isEmpty(); });
    QObject::connect(&bus, &DialogG2::MeteringBusController::amc16zBranchCurrentsUpdated, [&](const auto &v) { currentInvalid = v.isEmpty(); });
    bool adlInvalid = false, branchesInvalid = false, leakageInvalid = false, temperatureInvalid = false, batteryInvalid = false;
    QObject::connect(&bus, &DialogG2::MeteringBusController::adl200InputMeterUpdated, [&](const auto &m) { adlInvalid = !m.valid && std::isnan(m.voltage); });
    QObject::connect(&bus, &DialogG2::MeteringBusController::amc16zFak24BranchPowersUpdated, [&](const auto &m) { branchesInvalid = m.isEmpty(); });
    QObject::connect(&bus, &DialogG2::MeteringBusController::asj60Ld16aLeakageUpdated, [&](const auto &m) { leakageInvalid = m.isEmpty(); });
    QObject::connect(&bus, &DialogG2::MeteringBusController::whdTemperatureHumidityUpdated, [&](const auto &m) { temperatureInvalid = !m.valid && std::isnan(m.temperature); });
    QObject::connect(&bus, &DialogG2::MeteringBusController::jbdBmsBatteryUpdated, [&](const auto &b) { batteryInvalid = !b.communicationOk && !b.connected && b.socPercent == -1 && std::isnan(b.voltage); });
    TelemetryTestAccess::failMeter(bus, 5);
    check(!voltageInvalid && !currentInvalid && !branchesInvalid, "one voltage timeout keeps last valid measurements");
    TelemetryTestAccess::failMeter(bus, 5);
    check(!voltageInvalid, "two voltage timeouts keep last valid measurements");
    TelemetryTestAccess::failMeter(bus, 5);
    check(voltageInvalid && !currentInvalid && !branchesInvalid, "three voltage timeouts invalidate only voltage");
    for (int i = 0; i < 3; ++i)
        TelemetryTestAccess::failMeter(bus, 6);
    check(currentInvalid, "three current timeouts invalidate current");
    voltageInvalid = currentInvalid = false;
    TelemetryTestAccess::batteryCellsBeforeBasic(bus);
    check(batteryInvalid, "cell response cannot establish basic BMS health at startup");
    for (int i = 0; i < 3; ++i)
        TelemetryTestAccess::failMeter(bus, 1);
    check(adlInvalid && !temperatureInvalid, "ADL failure invalidates only ADL");
    for (int i = 0; i < 3; ++i) {
        TelemetryTestAccess::failMeter(bus, 2);
        TelemetryTestAccess::failMeter(bus, 3);
        TelemetryTestAccess::failMeter(bus, 4);
        TelemetryTestAccess::failBattery(bus);
    }
    check(!branchesInvalid && leakageInvalid && temperatureInvalid && batteryInvalid,
          "brief AMC power timeouts retain the last branch powers without delaying other devices");
    if (!branchesInvalid) {
        QEventLoop branchPowerWait;
        QTimer::singleShot(19000, &branchPowerWait, &QEventLoop::quit);
        branchPowerWait.exec();
        TelemetryTestAccess::failMeter(bus, 2);
        check(!branchesInvalid, "AMC branch powers remain available before 20 seconds");
        QTimer::singleShot(1200, &branchPowerWait, &QEventLoop::quit);
        branchPowerWait.exec();
        TelemetryTestAccess::failMeter(bus, 2);
        check(branchesInvalid, "AMC branch powers become unavailable after 20 seconds without a response");
    }
    TelemetryTestAccess::receiveAmcPower(bus);
    check(!branchesInvalid, "a new valid AMC power response restores branch values immediately");
    for (int i = 0; i < 3; ++i)
        TelemetryTestAccess::failMeter(bus, 2);
    check(!branchesInvalid, "a fresh AMC power response restarts the 20-second grace period");
    adlInvalid = branchesInvalid = leakageInvalid = temperatureInvalid = batteryInvalid = false;
    bus.disconnectDevice();
    check(voltageInvalid && currentInvalid && adlInvalid && branchesInvalid && leakageInvalid && temperatureInvalid && batteryInvalid, "port disconnect invalidates all devices");
    bool destructorEmitted = false;
    auto *temporaryBus = new DialogG2::MeteringBusController;
    QObject::connect(temporaryBus, &DialogG2::MeteringBusController::adl200InputMeterUpdated,
                     [&](const auto &) { destructorEmitted = true; });
    delete temporaryBus;
    check(!destructorEmitted, "destruction must not call receivers with partially destroyed state");
    fprintf(stdout, "%d failures\n", failures);
    return failures ? 1 : 0;
}
