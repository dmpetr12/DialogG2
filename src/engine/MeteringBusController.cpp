#include "MeteringBusController.h"

#include "ModbusRtuCodec.h"

#include <QSerialPort>

#include <algorithm>

namespace DialogG2 {

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
    connect(&m_scheduler, &QTimer::timeout, this, &MeteringBusController::pollTick);
    m_scheduler.setInterval(50);

    connect(&m_timeoutTimer, &QTimer::timeout, this, &MeteringBusController::onRequestTimeout);
    m_timeoutTimer.setSingleShot(true);

    connect(m_port, &QSerialPort::readyRead, this, &MeteringBusController::onReadyRead);
    connect(m_port, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError)
            return;
        const QString message = m_port->errorString().isEmpty()
            ? QStringLiteral("Serial port error")
            : m_port->errorString();
        emit errorOccurred(message);
        updateBusMonitorFailure(message);
    });
}

MeteringBusController::~MeteringBusController()
{
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
    if (!m_port)
        return;

    setupPort();
    if (!m_port->open(QIODevice::ReadWrite)) {
        updateBusMonitorFailure(m_port->errorString());
        emit connectedChanged(false);
        return;
    }

    emit connectedChanged(true);
    updateBusMonitorSuccess();
    pump();
}

void MeteringBusController::disconnectDevice()
{
    stopPolling();
    m_timeoutTimer.stop();
    m_queue.clear();
    m_rxBuffer.clear();
    m_busy = false;

    if (m_port && m_port->isOpen())
        m_port->close();
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
    if (!isConnected())
        return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (PollTask &task : m_pollTasks) {
        if (task.nextDueMsec > now)
            continue;
        enqueue(task.request);
        task.nextDueMsec = now + task.intervalMs;
    }
}

void MeteringBusController::enqueue(const Request &request)
{
    for (const Request &queued : m_queue) {
        if (sameRequest(queued, request))
            return;
    }

    m_queue.push_back(request);
    pump();
}

void MeteringBusController::pump()
{
    if (!isConnected() || m_busy || m_queue.empty())
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
    if (!m_busy)
        return;

    switch (m_currentRequest.type) {
    case RequestType::JbdBmsBasicInfo:
    case RequestType::JbdBmsCellVoltages:
        if (m_rxBuffer.size() >= 7 && byteAt(m_rxBuffer, m_rxBuffer.size() - 1) == 0x77)
            handleCurrentResponse();
        break;
    case RequestType::ReadHolding:
    case RequestType::ReadInputRegs:
        if (m_rxBuffer.size() >= expectedResponseSize())
            handleCurrentResponse();
        break;
    }
}

void MeteringBusController::onRequestTimeout()
{
    if (!m_busy)
        return;

    handleRequestFailure(QStringLiteral("Metering bus request timeout"));
    finishCurrentRequest();
}

void MeteringBusController::handleCurrentResponse()
{
    switch (m_currentRequest.type) {
    case RequestType::JbdBmsBasicInfo:
    case RequestType::JbdBmsCellVoltages:
    {
        JbdBmsResponse response;
        QString error;
        if (!JbdBmsProtocol::parseResponse(m_rxBuffer, &response, &error)) {
            handleRequestFailure(error);
            break;
        }
        if (!response.ok()) {
            handleRequestFailure(QStringLiteral("BMS returned status 0x%1")
                                     .arg(response.status, 2, 16, QLatin1Char('0')));
            break;
        }

        updateBusMonitorSuccess();
        if (response.command == JbdBmsProtocol::CommandBasicInfo)
            m_battery = JbdBmsProtocol::decodeBasicInfo(response.data);
        else if (response.command == JbdBmsProtocol::CommandCellVoltages)
            JbdBmsProtocol::applyCellVoltages(&m_battery, response.data);
        emit jbdBmsBatteryUpdated(m_battery);
        break;
    }
    case RequestType::ReadHolding:
    case RequestType::ReadInputRegs:
    {
        if (!ModbusRtuCodec::validateCrc(m_rxBuffer)) {
            handleRequestFailure(QStringLiteral("Modbus CRC mismatch"));
            break;
        }

        updateBusMonitorSuccess();
        const QVector<quint16> values = ModbusRtuCodec::registersFromReadResponse(m_rxBuffer);
        if (m_currentRequest.meterKind == MeterKind::Adl200Input)
            emit adl200InputMeterUpdated(Adl200Meter::decodeRealtimeHoldingRegisters(values));
        else if (m_currentRequest.meterKind == MeterKind::Amc16zFak24BranchPower)
            emit amc16zFak24BranchPowersUpdated(Amc16zFak24Meter::decodeActivePowerHoldingRegisters(values));
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
    pump();
}

void MeteringBusController::handleRequestFailure(const QString &error)
{
    const QString message = error.isEmpty() ? QStringLiteral("Metering bus request failed") : error;
    emit errorOccurred(message);
    updateBusMonitorFailure(message);
}

void MeteringBusController::updateBusMonitorSuccess()
{
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

int MeteringBusController::expectedResponseSize() const
{
    return ModbusRtuCodec::expectedReadResponseSize(m_currentRequest.count, false);
}

} // namespace DialogG2
