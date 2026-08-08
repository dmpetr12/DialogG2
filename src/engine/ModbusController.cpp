#include "ModbusController.h"

#include <QModbusDataUnit>
#include <QModbusDevice>
#include <QModbusReply>
#include <QModbusRtuSerialClient>
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

ModbusController::ModbusController(QObject *parent)
    : QObject(parent)
{
    recreateClient();
    connect(&m_scheduler, &QTimer::timeout, this, &ModbusController::pollTick);
    m_scheduler.setInterval(50);
}

ModbusController::~ModbusController()
{
    disconnectDevice();
}

void ModbusController::configure(const ModbusRtuConfig &config)
{
    const bool reconnect = isConnected();
    if (reconnect)
        disconnectDevice();

    m_config = config;
    m_busMonitor.setOfflineFailureThreshold(config.busOfflineFailureThreshold);
    setupDevice();

    if (reconnect)
        connectDevice();
}

void ModbusController::setWaveShareBaseAddress(int address)
{
    m_waveShareBaseAddress = std::max(1, address);
}

void ModbusController::setWaveShareModuleCount(int count)
{
    m_waveShareModuleCount = std::max(0, count);
}

bool ModbusController::isConnected() const
{
    return m_client && m_client->state() == QModbusDevice::ConnectedState;
}

ModbusBusStatus ModbusController::busStatus() const
{
    return m_busMonitor.status();
}

void ModbusController::connectDevice()
{
    if (!m_client)
        recreateClient();

    setupDevice();
    if (!m_client->connectDevice())
        updateBusMonitorFailure(m_client->errorString());
}

void ModbusController::disconnectDevice()
{
    stopPolling();
    clearQueues();
    m_busy = false;

    if (m_client)
        m_client->disconnectDevice();
}

void ModbusController::startPolling()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (PollTask &task : m_pollTasks)
        task.nextDueMsec = now;

    m_scheduler.start();
}

void ModbusController::stopPolling()
{
    m_scheduler.stop();
}

void ModbusController::clearPollTasks()
{
    m_pollTasks.clear();
}

void ModbusController::addWaveShareModulePolling(int module, int inputsIntervalMs, int relaysIntervalMs)
{
    if (module <= 0)
        return;

    Request inputs;
    inputs.type = RequestType::ReadInputs;
    inputs.module = module;
    inputs.slaveAddress = m_waveShareBaseAddress + module - 1;
    inputs.start = 0;
    inputs.count = 8;
    m_pollTasks.append({inputs, std::max(50, inputsIntervalMs), 0, RequestPriority::Normal});

    Request relays = inputs;
    relays.type = RequestType::ReadCoils;
    m_pollTasks.append({relays, std::max(50, relaysIntervalMs), 0, RequestPriority::Low});
}

void ModbusController::addAdl200InputMeterPolling(int intervalMs, int slaveAddress)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = std::max(1, slaveAddress);
    request.start = Adl200Meter::RealtimeHoldingStart;
    request.count = Adl200Meter::RealtimeHoldingCount;
    request.meterKind = MeterKind::Adl200Input;
    m_pollTasks.append({request, std::max(50, intervalMs), 0, RequestPriority::Normal});
}

void ModbusController::addAmc16zFak24BranchPowerPolling(int intervalMs, int slaveAddress)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = std::max(1, slaveAddress);
    request.start = Amc16zFak24Meter::ActivePowerHoldingStart;
    request.count = Amc16zFak24Meter::ActivePowerHoldingCount;
    request.meterKind = MeterKind::Amc16zFak24BranchPower;
    m_pollTasks.append({request, std::max(50, intervalMs), 0, RequestPriority::Normal});
}

void ModbusController::addAsj60Ld16aLeakagePolling(int intervalMs, int slaveAddress)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = std::max(1, slaveAddress);
    request.start = Asj60Ld16aMonitor::ChannelDataHoldingStart;
    request.count = Asj60Ld16aMonitor::ChannelDataHoldingCount;
    request.meterKind = MeterKind::Asj60Ld16aLeakage;
    m_pollTasks.append({request, std::max(50, intervalMs), 0, RequestPriority::Normal});
}

void ModbusController::addWhdTemperatureHumidityPolling(int intervalMs, int slaveAddress)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = std::max(1, slaveAddress);
    request.start = WhdTemperatureHumidityController::Channel1RealtimeRegisterStart;
    request.count = WhdTemperatureHumidityController::Channel1RealtimeRegisterCount;
    request.meterKind = MeterKind::WhdTemperatureHumidity;
    m_pollTasks.append({request, std::max(50, intervalMs), 0, RequestPriority::Normal});
}

void ModbusController::addHoldingRegistersPolling(int slaveAddress, int start, int count, int intervalMs)
{
    Request request;
    request.type = RequestType::ReadHolding;
    request.slaveAddress = slaveAddress;
    request.start = start;
    request.count = count;
    m_pollTasks.append({request, std::max(50, intervalMs), 0, RequestPriority::Normal});
}

void ModbusController::addInputRegistersPolling(int slaveAddress, int start, int count, int intervalMs)
{
    Request request;
    request.type = RequestType::ReadInputRegs;
    request.slaveAddress = slaveAddress;
    request.start = start;
    request.count = count;
    m_pollTasks.append({request, std::max(50, intervalMs), 0, RequestPriority::Normal});
}

void ModbusController::readWaveShareInputs(int module)
{
    if (module <= 0 || module > m_waveShareModuleCount)
        return;

    Request request;
    request.type = RequestType::ReadInputs;
    request.module = module;
    request.slaveAddress = m_waveShareBaseAddress + module - 1;
    request.start = 0;
    request.count = 8;
    enqueue(request, RequestPriority::Normal);
}

void ModbusController::readWaveShareRelays(int module)
{
    if (module <= 0 || module > m_waveShareModuleCount)
        return;

    Request request;
    request.type = RequestType::ReadCoils;
    request.module = module;
    request.slaveAddress = m_waveShareBaseAddress + module - 1;
    request.start = 0;
    request.count = 8;
    enqueue(request, RequestPriority::Low);
}

void ModbusController::writeWaveShareRelayByte(int module, quint8 bits)
{
    if (module <= 0 || module > m_waveShareModuleCount)
        return;

    Request request;
    request.type = RequestType::WriteCoils8;
    request.module = module;
    request.slaveAddress = m_waveShareBaseAddress + module - 1;
    request.start = 0;
    request.count = 8;
    request.coilsBits = bits;
    enqueue(request, RequestPriority::High);
}

void ModbusController::setupDevice()
{
    if (!m_client)
        return;

    m_client->setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_config.port);
    m_client->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_config.baudRate);
    m_client->setConnectionParameter(QModbusDevice::SerialParityParameter, parityFromConfig(m_config.parity));
    m_client->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBitsFromConfig(m_config.dataBits));
    m_client->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBitsFromConfig(m_config.stopBits));
    m_client->setTimeout(m_config.timeoutMs);
    m_client->setNumberOfRetries(m_config.retries);
}

void ModbusController::recreateClient()
{
    if (m_client) {
        m_client->deleteLater();
        m_client = nullptr;
    }

    m_client = new QModbusRtuSerialClient(this);
    setupDevice();

    connect(m_client, &QModbusClient::stateChanged, this, [this](QModbusDevice::State state) {
        const bool connected = state == QModbusDevice::ConnectedState;
        emit connectedChanged(connected);
        if (connected)
            updateBusMonitorSuccess();
    });

    connect(m_client, &QModbusClient::errorOccurred, this, [this](QModbusDevice::Error error) {
        if (error == QModbusDevice::NoError)
            return;

        const QString message = m_client ? m_client->errorString() : QStringLiteral("Modbus client error");
        emit errorOccurred(message);
    });
}

void ModbusController::pollTick()
{
    if (!isConnected())
        return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (PollTask &task : m_pollTasks) {
        if (task.nextDueMsec > now)
            continue;

        enqueue(task.request, task.priority);
        task.nextDueMsec = now + task.intervalMs;
    }
}

void ModbusController::enqueue(Request request, RequestPriority priority)
{
    auto enqueuePeriodic = [&request](std::deque<Request> *queue) {
        for (const Request &queued : *queue) {
            if (samePeriodicRequest(queued, request))
                return;
        }
        queue->push_back(request);
    };

    switch (priority) {
    case RequestPriority::High:
        for (auto it = m_highQueue.begin(); it != m_highQueue.end();) {
            if (sameWriteTarget(*it, request))
                it = m_highQueue.erase(it);
            else
                ++it;
        }
        m_highQueue.push_back(request);
        break;
    case RequestPriority::Normal:
        enqueuePeriodic(&m_normalQueue);
        break;
    case RequestPriority::Low:
        enqueuePeriodic(&m_lowQueue);
        break;
    }

    pump();
}

void ModbusController::pump()
{
    if (!isConnected() || m_busy)
        return;

    Request request;
    bool hasRequest = false;

    if (!m_highQueue.empty()) {
        request = m_highQueue.front();
        m_highQueue.pop_front();
        hasRequest = true;
    } else if (!m_normalQueue.empty()) {
        request = m_normalQueue.front();
        m_normalQueue.pop_front();
        hasRequest = true;
    } else if (!m_lowQueue.empty()) {
        request = m_lowQueue.front();
        m_lowQueue.pop_front();
        hasRequest = true;
    }

    if (!hasRequest)
        return;

    m_busy = true;
    sendRequest(request);
}

void ModbusController::sendRequest(const Request &request)
{
    if (!m_client) {
        m_busy = false;
        return;
    }

    QModbusReply *reply = nullptr;
    switch (request.type) {
    case RequestType::WriteCoils8: {
        QModbusDataUnit unit(QModbusDataUnit::Coils, request.start, request.count);
        for (int i = 0; i < request.count; ++i)
            unit.setValue(i, (request.coilsBits >> i) & 0x01);
        reply = m_client->sendWriteRequest(unit, request.slaveAddress);
        break;
    }
    case RequestType::ReadInputs: {
        const QModbusDataUnit unit(QModbusDataUnit::DiscreteInputs, request.start, request.count);
        reply = m_client->sendReadRequest(unit, request.slaveAddress);
        break;
    }
    case RequestType::ReadCoils: {
        const QModbusDataUnit unit(QModbusDataUnit::Coils, request.start, request.count);
        reply = m_client->sendReadRequest(unit, request.slaveAddress);
        break;
    }
    case RequestType::ReadHolding: {
        const QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, request.start, request.count);
        reply = m_client->sendReadRequest(unit, request.slaveAddress);
        break;
    }
    case RequestType::ReadInputRegs: {
        const QModbusDataUnit unit(QModbusDataUnit::InputRegisters, request.start, request.count);
        reply = m_client->sendReadRequest(unit, request.slaveAddress);
        break;
    }
    }

    if (!reply) {
        m_busy = false;
        handleRequestFailure(request, m_client->errorString());
        pump();
        return;
    }

    if (reply->isFinished()) {
        finishRequest(request, reply);
        return;
    }

    connect(reply, &QModbusReply::finished, this, [this, request, reply]() {
        finishRequest(request, reply);
    });
}

void ModbusController::finishRequest(const Request &request, QModbusReply *reply)
{
    if (!reply) {
        m_busy = false;
        pump();
        return;
    }

    if (reply->error() == QModbusDevice::NoError)
        handleRequestSuccess(request, reply);
    else
        handleRequestFailure(request, reply->errorString());

    reply->deleteLater();
    m_busy = false;
    pump();
}

void ModbusController::handleRequestSuccess(const Request &request, QModbusReply *reply)
{
    updateBusMonitorSuccess();

    switch (request.type) {
    case RequestType::WriteCoils8:
        emit waveShareRelaysUpdated(request.module, request.coilsBits);
        break;
    case RequestType::ReadInputs:
        emit waveShareInputsUpdated(request.module, bitsFromReply(reply));
        break;
    case RequestType::ReadCoils:
        emit waveShareRelaysUpdated(request.module, bitsFromReply(reply));
        break;
    case RequestType::ReadHolding:
    {
        const QVector<quint16> values = valuesFromReply(reply);
        emit holdingRegistersUpdated(request.slaveAddress, request.start, values);
        if (request.meterKind == MeterKind::Adl200Input)
            emit adl200InputMeterUpdated(Adl200Meter::decodeRealtimeHoldingRegisters(values));
        else if (request.meterKind == MeterKind::Amc16zFak24BranchPower)
            emit amc16zFak24BranchPowersUpdated(Amc16zFak24Meter::decodeActivePowerHoldingRegisters(values));
        else if (request.meterKind == MeterKind::Asj60Ld16aLeakage)
            emit asj60Ld16aLeakageUpdated(Asj60Ld16aMonitor::decodeChannelHoldingRegisters(values));
        else if (request.meterKind == MeterKind::WhdTemperatureHumidity)
            emit whdTemperatureHumidityUpdated(WhdTemperatureHumidityController::decodeChannel1RealtimeRegisters(values));
        break;
    }
    case RequestType::ReadInputRegs:
        emit inputRegistersUpdated(request.slaveAddress, request.start, valuesFromReply(reply));
        break;
    }
}

void ModbusController::handleRequestFailure(const Request &, const QString &error)
{
    const QString message = error.isEmpty() ? QStringLiteral("Modbus request failed") : error;
    emit errorOccurred(message);
    updateBusMonitorFailure(message);
}

void ModbusController::updateBusMonitorSuccess()
{
    const bool wasOnline = m_busMonitor.status().online;
    m_busMonitor.markSuccess();
    emit busStatusChanged(m_busMonitor.status());
    if (!wasOnline)
        emit busOnline();
}

void ModbusController::updateBusMonitorFailure(const QString &error)
{
    const bool wasOnline = m_busMonitor.status().online;
    m_busMonitor.markFailure(error);
    emit busStatusChanged(m_busMonitor.status());
    if (wasOnline && !m_busMonitor.status().online)
        emit busOffline(error);
}

void ModbusController::clearQueues()
{
    m_highQueue.clear();
    m_normalQueue.clear();
    m_lowQueue.clear();
}

bool ModbusController::samePeriodicRequest(const Request &a, const Request &b)
{
    if (a.type == RequestType::WriteCoils8 || b.type == RequestType::WriteCoils8)
        return false;

    return a.type == b.type
        && a.slaveAddress == b.slaveAddress
        && a.start == b.start
        && a.count == b.count
        && a.module == b.module
        && a.meterKind == b.meterKind;
}

bool ModbusController::sameWriteTarget(const Request &a, const Request &b)
{
    return a.type == RequestType::WriteCoils8
        && b.type == RequestType::WriteCoils8
        && a.slaveAddress == b.slaveAddress
        && a.start == b.start
        && a.count == b.count;
}

QVector<quint16> ModbusController::valuesFromReply(QModbusReply *reply)
{
    QVector<quint16> values;
    if (!reply)
        return values;

    const QModbusDataUnit result = reply->result();
    values.reserve(static_cast<int>(result.valueCount()));
    for (uint i = 0; i < result.valueCount(); ++i)
        values.append(result.value(i));
    return values;
}

quint8 ModbusController::bitsFromReply(QModbusReply *reply)
{
    quint8 bits = 0;
    const QVector<quint16> values = valuesFromReply(reply);
    for (int i = 0; i < values.size() && i < 8; ++i) {
        if (values.at(i) != 0)
            bits = static_cast<quint8>(bits | (1u << i));
    }
    return bits;
}

} // namespace DialogG2
