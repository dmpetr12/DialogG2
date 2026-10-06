#include "MeteringBusController.h"

#include "ModbusRtuCodec.h"

#include <QSerialPort>
#include <QDebug>

#include <algorithm>

namespace DialogG2 {

static constexpr int ReconnectIntervalMs = 3000;
static constexpr int AmcPowerNoResponseGraceMs = 20000;
static constexpr int InterRequestGapMs = 100;

static QSerialPort::Parity parityFromConfig(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("even") || normalized == QStringLiteral("e"))
        return QSerialPort::EvenParity;
    if (normalized == QStringLiteral("odd") || normalized == QStringLiteral("o"))
        return QSerialPort::OddParity;
    return QSerialPort::NoParity;
}

static QSerialPort::DataBits dataBitsFromConfig(int value)
{
    switch (value) {
    case 5: return QSerialPort::Data5;
    case 6: return QSerialPort::Data6;
    case 7: return QSerialPort::Data7;
    case 8:
    default:
        return QSerialPort::Data8;
    }
}

static QSerialPort::StopBits stopBitsFromConfig(int value)
{
    return value == 2 ? QSerialPort::TwoStop : QSerialPort::OneStop;
}

static quint8 byteAt(const QByteArray &data, int index)
{
    return static_cast<quint8>(data.at(index));
}

MeteringBusController::MeteringBusController(QObject *parent)
    : QObject(parent)
    , m_port(new QSerialPort(this))
{
    m_battery.connected = false;
    m_battery.communicationOk = false;
    m_battery.state = BatteryState::Disconnected;

    connect(&m_scheduler, &QTimer::timeout, this, &MeteringBusController::pollTick);
    m_scheduler.setInterval(50);

    connect(&m_timeoutTimer, &QTimer::timeout, this, &MeteringBusController::onRequestTimeout);
    m_timeoutTimer.setSingleShot(true);

    connect(&m_interRequestTimer, &QTimer::timeout, this, &MeteringBusController::pump);
    m_interRequestTimer.setSingleShot(true);
    m_interRequestTimer.setTimerType(Qt::PreciseTimer);

    connect(m_port, &QSerialPort::readyRead, this, &MeteringBusController::onReadyRead);
    connect(m_port, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError)
            return;
        const QString message = m_port->errorString().isEmpty()
            ? QStringLiteral("Serial port error")
            : m_port->errorString();
        invalidateMeasurements();
        reportError(message);
        updateBusMonitorFailure(message);

        if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError
            || error == QSerialPort::PermissionError) {
            m_timeoutTimer.stop();
            m_interRequestTimer.stop();
            m_queue.clear();
            m_rxBuffer.clear();
            m_busy = false;
            m_port->close();
            m_nextConnectAttemptMsec = QDateTime::currentMSecsSinceEpoch() + ReconnectIntervalMs;
            emit connectedChanged(false);
        }
    });
}

MeteringBusController::~MeteringBusController()
{
    // Receivers may already be tearing down their measurement storage.
    QObject::disconnect(this, nullptr, nullptr, nullptr);
    disconnectDevice();
}

void MeteringBusController::configure(const ModbusRtuConfig &config)
{
    const bool reconnect = isConnected();
    if (reconnect)
        disconnectDevice();

    m_config = config;
    m_busMonitor.setOfflineFailureThreshold(config.busOfflineFailureThreshold);
    setupPort();

    if (reconnect)
        connectDevice();
}

bool MeteringBusController::isConnected() const
{
    return m_port && m_port->isOpen();
}

ModbusBusStatus MeteringBusController::busStatus() const
{
    return m_busMonitor.status();
}

void MeteringBusController::connectDevice()
{
    if (!m_port || m_port->isOpen())
        return;

    m_nextConnectAttemptMsec = QDateTime::currentMSecsSinceEpoch() + ReconnectIntervalMs;
    setupPort();
    if (!m_port->open(QIODevice::ReadWrite)) {
        updateBusMonitorFailure(m_port->errorString());
        emit connectedChanged(false);
        return;
    }

    m_nextConnectAttemptMsec = 0;
    emit connectedChanged(true);
    updateBusMonitorSuccess();
    pump();
}

void MeteringBusController::disconnectDevice()
{
    stopPolling();
    m_timeoutTimer.stop();
    m_interRequestTimer.stop();
    m_queue.clear();
    m_rxBuffer.clear();
    m_busy = false;

    if (m_port && m_port->isOpen())
        m_port->close();
    m_busMonitor.reset();
    invalidateMeasurements();
    emit busStatusChanged(m_busMonitor.status());
    emit connectedChanged(false);
}

void MeteringBusController::startPolling()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (PollTask &task : m_pollTasks)
        task.nextDueMsec = now;
    m_scheduler.start();
    pump();
}

void MeteringBusController::stopPolling()
{
    m_scheduler.stop();
}

void MeteringBusController::clearPollTasks()
{
    m_pollTasks.clear();
}

void MeteringBusController::addAdl200InputMeterPolling(int intervalMs, int slaveAddress)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = std::max(1, slaveAddress);
    request.start = Adl200Meter::RealtimeHoldingStart;
    request.count = Adl200Meter::RealtimeHoldingCount;
    request.meterKind = MeterKind::Adl200Input;
    m_pollTasks.append({request, std::max(50, intervalMs), 0});
}

void MeteringBusController::addAmc16zFak24BranchPowerPolling(int intervalMs, int slaveAddress)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = std::max(1, slaveAddress);
    request.start = Amc16zFak24Meter::ActivePowerHoldingStart;
    request.count = Amc16zFak24Meter::ActivePowerHoldingCount;
    request.meterKind = MeterKind::Amc16zFak24BranchPower;
    m_pollTasks.append({request, std::max(50, intervalMs), 0});
    request.start = Amc16zFak24Meter::VoltageHoldingStart;
    request.meterKind = MeterKind::Amc16zBranchVoltage;
    m_pollTasks.append({request, std::max(50, intervalMs), 0});
    request.start = Amc16zFak24Meter::CurrentHoldingStart;
    request.meterKind = MeterKind::Amc16zBranchCurrent;
    m_pollTasks.append({request, std::max(50, intervalMs), 0});
}

void MeteringBusController::addAsj60Ld16aLeakagePolling(int intervalMs, int slaveAddress)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = std::max(1, slaveAddress);
    request.start = Asj60Ld16aMonitor::ChannelDataHoldingStart;
    request.count = Asj60Ld16aMonitor::ChannelDataHoldingCount;
    request.meterKind = MeterKind::Asj60Ld16aLeakage;
    m_pollTasks.append({request, std::max(50, intervalMs), 0});
}

void MeteringBusController::addWhdTemperatureHumidityPolling(int intervalMs, int slaveAddress)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = std::max(1, slaveAddress);
    request.start = WhdTemperatureHumidityController::Channel1RealtimeRegisterStart;
    request.count = WhdTemperatureHumidityController::Channel1RealtimeRegisterCount;
    request.meterKind = MeterKind::WhdTemperatureHumidity;
    m_pollTasks.append({request, std::max(50, intervalMs), 0});
}

void MeteringBusController::addJbdBmsPolling(int basicInfoIntervalMs, int cellVoltagesIntervalMs)
{
    Request basic;
    basic.type = RequestType::JbdBmsBasicInfo;
    m_pollTasks.append({basic, std::max(50, basicInfoIntervalMs), 0});

    Request cells;
    cells.type = RequestType::JbdBmsCellVoltages;
    m_pollTasks.append({cells, std::max(50, cellVoltagesIntervalMs), 0});
}

void MeteringBusController::setupPort()
{
    if (!m_port)
        return;

    m_port->setPortName(m_config.port);
    m_port->setBaudRate(m_config.baudRate);
    m_port->setParity(parityFromConfig(m_config.parity));
    m_port->setDataBits(dataBitsFromConfig(m_config.dataBits));
    m_port->setStopBits(stopBitsFromConfig(m_config.stopBits));
    m_port->setFlowControl(QSerialPort::NoFlowControl);
}

void MeteringBusController::pollTick()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (!isConnected()) {
        if (now >= m_nextConnectAttemptMsec)
            connectDevice();
        return;
    }

    for (PollTask &task : m_pollTasks) {
        if (task.nextDueMsec > now)
            continue;
        enqueue(task.request);
        task.nextDueMsec = now + task.intervalMs;
    }
}

void MeteringBusController::enqueue(const Request &request)
{
    if (m_busy && sameRequest(m_currentRequest, request))
        return;

    for (const Request &queued : m_queue) {
        if (sameRequest(queued, request))
            return;
    }

    m_queue.push_back(request);
    pump();
}

void MeteringBusController::pump()
{
    if (!isConnected() || m_busy || m_interRequestTimer.isActive() || m_queue.empty())
        return;

    m_currentRequest = m_queue.front();
    m_queue.pop_front();
    m_busy = true;
    sendRequest(m_currentRequest);
}

void MeteringBusController::sendRequest(const Request &request)
{
    QByteArray frame;
    switch (request.type) {
    case RequestType::ReadHolding:
        frame = ModbusRtuCodec::readRequest(request.slaveAddress, 0x03, request.start, request.count);
        break;
    case RequestType::ReadInputRegs:
        frame = ModbusRtuCodec::readRequest(request.slaveAddress, 0x04, request.start, request.count);
        break;
    case RequestType::JbdBmsBasicInfo:
        frame = JbdBmsProtocol::buildReadCommand(JbdBmsProtocol::CommandBasicInfo);
        break;
    case RequestType::JbdBmsCellVoltages:
        frame = JbdBmsProtocol::buildReadCommand(JbdBmsProtocol::CommandCellVoltages);
        break;
    }

    m_rxBuffer.clear();
    qDebug().noquote() << "Metering TX:" << frame.toHex(' ');
    if (m_port->write(frame) != frame.size()) {
        handleRequestFailure(m_port->errorString());
        finishCurrentRequest();
        return;
    }

    m_port->flush();
    m_timeoutTimer.start(std::max(50, m_config.timeoutMs));
}

void MeteringBusController::onReadyRead()
{
    m_rxBuffer.append(m_port->readAll());
    processReceiveBuffer();
}

void MeteringBusController::processReceiveBuffer()
{
    if (!m_busy) {
        m_rxBuffer.clear();
        return;
    }
    const bool expectingBms = m_currentRequest.type == RequestType::JbdBmsBasicInfo
        || m_currentRequest.type == RequestType::JbdBmsCellVoltages;
    while (m_rxBuffer.size() >= 2) {
        const bool bmsFrame = byteAt(m_rxBuffer, 0) == 0xDD;
        int frameSize = 0;
        if (bmsFrame) {
            if (m_rxBuffer.size() < 4) return;
            // No callbackId is sent by this controller: response length is payload + 7.
            frameSize = 7 + byteAt(m_rxBuffer, 3);
        } else {
            const quint8 function = byteAt(m_rxBuffer, 1);
            if (function == 0x83 || function == 0x84) frameSize = 5;
            else if (function == 0x03 || function == 0x04) {
                if (m_rxBuffer.size() < 3) return;
                frameSize = 5 + byteAt(m_rxBuffer, 2);
            } else {
                m_rxBuffer.remove(0, 1);
                continue;
            }
        }
        if (m_rxBuffer.size() < frameSize) return;
        const QByteArray frame = m_rxBuffer.left(frameSize);
        JbdBmsResponse bms;
        const bool valid = bmsFrame ? JbdBmsProtocol::parseResponse(frame, &bms)
                                    : ModbusRtuCodec::validateCrc(frame);
        if (!valid) {
            m_rxBuffer.remove(0, 1);
            continue;
        }
        m_rxBuffer.remove(0, frameSize);
        const quint8 expectedCommand = m_currentRequest.type == RequestType::JbdBmsBasicInfo
            ? JbdBmsProtocol::CommandBasicInfo : JbdBmsProtocol::CommandCellVoltages;
        const quint8 expectedFunction = m_currentRequest.type == RequestType::ReadHolding ? 0x03 : 0x04;
        const bool matches = bmsFrame
            ? expectingBms && bms.command == expectedCommand
            : !expectingBms && byteAt(frame, 0) == m_currentRequest.slaveAddress
                && (byteAt(frame, 1) == (expectedFunction | 0x80)
                    || (byteAt(frame, 1) == expectedFunction && byteAt(frame, 2) == m_currentRequest.count * 2));
        if (!matches) {
            qDebug().noquote() << "Metering RX ignored (not current request):" << frame.toHex(' ');
            continue;
        }
        handleCurrentResponse(frame);
        return;
    }
}

void MeteringBusController::onRequestTimeout()
{
    if (!m_busy)
        return;

    handleRequestFailure(QStringLiteral("Metering timeout: type=%1 slave=%2 start=0x%3 count=%4 pending=%5")
                             .arg(static_cast<int>(m_currentRequest.type)).arg(m_currentRequest.slaveAddress)
                             .arg(m_currentRequest.start, 0, 16).arg(m_currentRequest.count)
                             .arg(QString::fromLatin1(m_rxBuffer.toHex(' '))));
    finishCurrentRequest();
}

void MeteringBusController::handleCurrentResponse(const QByteArray &frame)
{
    switch (m_currentRequest.type) {
    case RequestType::JbdBmsBasicInfo:
    case RequestType::JbdBmsCellVoltages:
    {
        JbdBmsResponse response;
        QString error;
        if (!JbdBmsProtocol::parseResponse(frame, &response, &error)) {
            handleRequestFailure(error);
            break;
        }
        const quint8 expectedCommand = m_currentRequest.type == RequestType::JbdBmsBasicInfo
            ? JbdBmsProtocol::CommandBasicInfo : JbdBmsProtocol::CommandCellVoltages;
        if (response.command != expectedCommand) return;
        if (!response.ok()) {
            handleRequestFailure(QStringLiteral("BMS returned status 0x%1")
                                     .arg(response.status, 2, 16, QLatin1Char('0')));
            break;
        }

        updateBusMonitorSuccess();
        m_requestFailures.remove(requestKey(m_currentRequest));
        if (response.command == JbdBmsProtocol::CommandBasicInfo) {
            BatterySnapshot battery = JbdBmsProtocol::decodeBasicInfo(response.data);
            battery.cellVoltages = m_battery.cellVoltages;
            battery.minCellVoltage = m_battery.minCellVoltage;
            battery.maxCellVoltage = m_battery.maxCellVoltage;
            battery.cellVoltageDelta = m_battery.cellVoltageDelta;
            m_battery = battery;
        } else if (response.command == JbdBmsProtocol::CommandCellVoltages) {
            JbdBmsProtocol::applyCellVoltages(&m_battery, response.data);
        }
        emit jbdBmsBatteryUpdated(m_battery);
        break;
    }
    case RequestType::ReadHolding:
    case RequestType::ReadInputRegs:
    {
        if (!ModbusRtuCodec::validateCrc(frame)) {
            handleRequestFailure(QStringLiteral("Modbus CRC mismatch"));
            break;
        }

        const quint8 function = m_currentRequest.type == RequestType::ReadHolding ? 0x03 : 0x04;
        if (byteAt(frame, 0) != m_currentRequest.slaveAddress) return;
        if (byteAt(frame, 1) == (function | 0x80) && frame.size() == 5) {
            handleRequestFailure(QStringLiteral("Modbus exception %1").arg(byteAt(frame, 2)));
            break;
        }
        if (byteAt(frame, 1) != function || frame.size() != expectedResponseSize()
            || byteAt(frame, 2) != m_currentRequest.count * 2) return;
        updateBusMonitorSuccess();
        m_requestFailures.remove(requestKey(m_currentRequest));
        if (m_currentRequest.meterKind == MeterKind::Amc16zFak24BranchPower)
            m_lastAmcPowerResponse.restart();
        const QVector<quint16> values = ModbusRtuCodec::registersFromReadResponse(frame);
        if (m_currentRequest.meterKind == MeterKind::Adl200Input)
            emit adl200InputMeterUpdated(Adl200Meter::decodeRealtimeHoldingRegisters(values));
        else if (m_currentRequest.meterKind == MeterKind::Amc16zFak24BranchPower)
            emit amc16zFak24BranchPowersUpdated(Amc16zFak24Meter::decodeActivePowerHoldingRegisters(values));
        else if (m_currentRequest.meterKind == MeterKind::Amc16zBranchVoltage)
            emit amc16zBranchVoltagesUpdated(Amc16zFak24Meter::decodeRmsHoldingRegisters(values));
        else if (m_currentRequest.meterKind == MeterKind::Amc16zBranchCurrent)
            emit amc16zBranchCurrentsUpdated(Amc16zFak24Meter::decodeRmsHoldingRegisters(values));
        else if (m_currentRequest.meterKind == MeterKind::Asj60Ld16aLeakage)
            emit asj60Ld16aLeakageUpdated(Asj60Ld16aMonitor::decodeChannelHoldingRegisters(values));
        else if (m_currentRequest.meterKind == MeterKind::WhdTemperatureHumidity)
            emit whdTemperatureHumidityUpdated(WhdTemperatureHumidityController::decodeChannel1RealtimeRegisters(values));
        break;
    }
    }

    finishCurrentRequest();
}

void MeteringBusController::finishCurrentRequest()
{
    m_timeoutTimer.stop();
    m_rxBuffer.clear();
    m_busy = false;
    m_interRequestTimer.start(InterRequestGapMs);
}

void MeteringBusController::handleRequestFailure(const QString &error)
{
    const QString key = requestKey(m_currentRequest);
    const int failures = ++m_requestFailures[key];
    const bool amcPowerRequest = m_currentRequest.meterKind == MeterKind::Amc16zFak24BranchPower;
    if (amcPowerRequest && !m_lastAmcPowerResponse.isValid())
        m_lastAmcPowerResponse.start();
    if (amcPowerRequest ? m_lastAmcPowerResponse.elapsed() >= AmcPowerNoResponseGraceMs
                        : failures >= std::max(1, m_config.busOfflineFailureThreshold))
        invalidateRequest(m_currentRequest);
    const QString message = error.isEmpty() ? QStringLiteral("Metering bus request failed") : error;
    reportError(message);
    updateBusMonitorFailure(message);
}

void MeteringBusController::reportError(const QString &message)
{
    if (message == m_lastReportedError) {
        ++m_suppressedErrorCount;
        return;
    }
    flushRepeatedErrors();
    m_lastReportedError = message;
    emit errorOccurred(message);
}

void MeteringBusController::flushRepeatedErrors()
{
    if (m_suppressedErrorCount > 0)
        emit errorOccurred(QStringLiteral("%1 (ещё %2 повторов)")
                               .arg(m_lastReportedError).arg(m_suppressedErrorCount));
    m_lastReportedError.clear();
    m_suppressedErrorCount = 0;
}

void MeteringBusController::invalidateRequest(const Request &request)
{
    if (request.type == RequestType::JbdBmsBasicInfo) {
        m_battery = BatterySnapshot{};
        m_battery.connected = false;
        m_battery.communicationOk = false;
        m_battery.state = BatteryState::Disconnected;
        m_battery.faults = {QStringLiteral("нет данных BMS")};
        emit jbdBmsBatteryUpdated(m_battery);
    } else if (request.type == RequestType::JbdBmsCellVoltages) {
        m_battery.cellVoltages.clear();
        m_battery.minCellVoltage = std::numeric_limits<double>::quiet_NaN();
        m_battery.maxCellVoltage = std::numeric_limits<double>::quiet_NaN();
        m_battery.cellVoltageDelta = std::numeric_limits<double>::quiet_NaN();
        emit jbdBmsBatteryUpdated(m_battery);
    } else {
        switch (request.meterKind) {
        case MeterKind::Adl200Input: emit adl200InputMeterUpdated({}); break;
        case MeterKind::Amc16zFak24BranchPower: emit amc16zFak24BranchPowersUpdated({}); break;
        case MeterKind::Asj60Ld16aLeakage: emit asj60Ld16aLeakageUpdated({}); break;
        case MeterKind::WhdTemperatureHumidity: emit whdTemperatureHumidityUpdated({}); break;
        case MeterKind::Amc16zBranchVoltage: emit amc16zBranchVoltagesUpdated({}); break;
        case MeterKind::Amc16zBranchCurrent: emit amc16zBranchCurrentsUpdated({}); break;
        case MeterKind::None: break;
        }
    }
}

void MeteringBusController::invalidateMeasurements()
{
    m_requestFailures.clear();
    m_lastAmcPowerResponse.invalidate();
    emit adl200InputMeterUpdated({});
    emit amc16zFak24BranchPowersUpdated({});
    emit amc16zBranchVoltagesUpdated({});
    emit amc16zBranchCurrentsUpdated({});
    emit asj60Ld16aLeakageUpdated({});
    emit whdTemperatureHumidityUpdated({});
    Request request;
    request.type = RequestType::JbdBmsBasicInfo;
    invalidateRequest(request);
}

void MeteringBusController::updateBusMonitorSuccess()
{
    flushRepeatedErrors();
    const bool wasOnline = m_busMonitor.status().online;
    m_busMonitor.markSuccess();
    emit busStatusChanged(m_busMonitor.status());
    if (!wasOnline)
        emit busOnline();
}

void MeteringBusController::updateBusMonitorFailure(const QString &error)
{
    const bool wasOnline = m_busMonitor.status().online;
    m_busMonitor.markFailure(error);
    emit busStatusChanged(m_busMonitor.status());
    if (wasOnline && !m_busMonitor.status().online)
        emit busOffline(error);
}

bool MeteringBusController::sameRequest(const Request &a, const Request &b)
{
    return a.type == b.type
        && a.slaveAddress == b.slaveAddress
        && a.start == b.start
        && a.count == b.count
        && a.meterKind == b.meterKind;
}

QString MeteringBusController::requestKey(const Request &request)
{
    return QStringLiteral("%1:%2:%3:%4:%5")
        .arg(static_cast<int>(request.type))
        .arg(request.slaveAddress)
        .arg(request.start)
        .arg(request.count)
        .arg(static_cast<int>(request.meterKind));
}

int MeteringBusController::expectedResponseSize() const
{
    return ModbusRtuCodec::expectedReadResponseSize(m_currentRequest.count, false);
}

} // namespace DialogG2
