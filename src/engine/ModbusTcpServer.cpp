#include "ModbusTcpServer.h"

#include <QDateTime>
#include <QtSerialBus/QModbusTcpServer>

#include <algorithm>
#include <cmath>

namespace DialogG2 {
namespace {

enum CoilAddress : int
{
    CoilFireOn = 0,
    CoilFireOff = 1,
    CoilStopOn = 2,
    CoilStopOff = 3,
    CoilStopTest = 4,
    CoilStartFunctionalTest = 5,
    CoilStartDurationTest = 6,

    CoilCount = 16
};

enum InputRegAddress : int
{
    InRegCabinetEnabled = 0,
    InRegCabinetState = 1,
    InRegLinesCount = 2,
    InRegBatteryState = 3,
    InRegEmergencyState = 4,
    InRegDoorState = 5,
    InRegSystemState = 6,
    InRegReserved08 = 7,

    InRegInputVoltage = 8,
    InRegInputPower = 9,
    InRegInputCurrent = 10,
    InRegInputFrequency = 11,
    InRegTemperature = 12,
    InRegReserved14 = 13,
    InRegReserved15 = 14,
    InRegReserved16 = 15,

    InRegSystemTimeLow = 16,
    InRegSystemTimeHigh = 17,
    InRegLastDurationTestLow = 18,
    InRegLastDurationTestHigh = 19,

    InRegLinesBase = 100,
    LineBlockSize = 6,

    InputRegisterCount = 400
};

enum LineBlockOffset : int
{
    LineType = 0,
    LineState = 1,
    LineOutputState = 2,
    LineLastTestLow = 3,
    LineLastTestHigh = 4,
    LineReserved1 = 5
};

static constexpr quint16 InvalidRegValue = 0xFFFF;

static quint16 lowWord(quint32 value)
{
    return static_cast<quint16>(value & 0xFFFF);
}

static quint16 highWord(quint32 value)
{
    return static_cast<quint16>((value >> 16) & 0xFFFF);
}

static quint32 dateTimeToU32(const QDateTime &dateTime)
{
    if (!dateTime.isValid())
        return 0;

    const qint64 seconds = dateTime.toUTC().toSecsSinceEpoch();
    if (seconds <= 0)
        return 0;

    return static_cast<quint32>(seconds);
}

static quint16 scaled(double value, double multiplier)
{
    if (!std::isfinite(value))
        return InvalidRegValue;
    if (value < 0.0)
        value = 0.0;

    const double rounded = std::round(value * multiplier);
    return static_cast<quint16>(std::clamp(rounded, 0.0, 65534.0));
}

static QDateTime latestDateTime(const QDateTime &left, const QDateTime &right)
{
    if (!left.isValid())
        return right;
    if (!right.isValid())
        return left;
    return left > right ? left : right;
}

static quint16 cabinetState(const CabinetSnapshot &snapshot)
{
    if (snapshot.fireInputActive || snapshot.manualEmergencyActive || snapshot.mode == CabinetMode::Fire)
        return 1;
    if (snapshot.activeTest.active || snapshot.testKind != TestKind::None)
        return 2;
    if (snapshot.battery.state == BatteryState::Fault || snapshot.battery.state == BatteryState::Disconnected)
        return 4;
    if (snapshot.battery.state == BatteryState::Warning)
        return 5;
    if (snapshot.health == SystemHealth::Fault)
        return 3;
    return 0;
}

static quint16 batteryState(const BatterySnapshot &battery)
{
    switch (battery.state) {
    case BatteryState::Normal: return 0;
    case BatteryState::Warning: return 2;
    case BatteryState::Fault:
    case BatteryState::Disconnected:
        return 1;
    }
    return 1;
}

static quint16 emergencyState(const CabinetSnapshot &snapshot)
{
    return (snapshot.fireInputActive || snapshot.manualEmergencyActive) ? 1 : 0;
}

static quint16 lineType(const LineSnapshot &line)
{
    if (!line.enabled)
        return 0;
    return line.kind == LineKind::Constant ? 1 : 2;
}

static quint16 lineState(const LineSnapshot &line, const CabinetSnapshot &snapshot)
{
    if (!line.enabled)
        return 3;
    if (snapshot.activeTest.active)
        return 2;
    return line.state == LineState::Normal ? 0 : 1;
}

static quint16 lineOutputState(const LineSnapshot &line)
{
    if (!line.enabled)
        return 2;
    return line.outputState == LineOutputState::On ? 1 : 0;
}

static quint16 readInputRegister(const CabinetSnapshot &snapshot, int address)
{
    switch (address) {
    case InRegCabinetEnabled: return 1;
    case InRegCabinetState: return cabinetState(snapshot);
    case InRegLinesCount: return static_cast<quint16>(std::clamp<int>(snapshot.lines.size(), 0, 65535));
    case InRegBatteryState: return batteryState(snapshot.battery);
    case InRegEmergencyState: return emergencyState(snapshot);
    case InRegDoorState: return 0;
    case InRegSystemState: return snapshot.health == SystemHealth::Normal ? 0 : 1;
    case InRegInputVoltage: return scaled(snapshot.inputVoltage, 10.0);
    case InRegInputPower: return scaled(snapshot.inputPower, 10.0);
    case InRegInputCurrent: return scaled(snapshot.inputCurrent, 100.0);
    case InRegInputFrequency: return scaled(snapshot.inputFrequency, 100.0);
    case InRegTemperature: return scaled(snapshot.temperature, 10.0);
    case InRegSystemTimeLow:
    case InRegSystemTimeHigh: {
        const quint32 timestamp = static_cast<quint32>(QDateTime::currentSecsSinceEpoch());
        return address == InRegSystemTimeLow ? lowWord(timestamp) : highWord(timestamp);
    }
    case InRegLastDurationTestLow:
    case InRegLastDurationTestHigh: {
        const quint32 timestamp = dateTimeToU32(snapshot.maintenance.lastLongTestAt);
        return address == InRegLastDurationTestLow ? lowWord(timestamp) : highWord(timestamp);
    }
    default:
        break;
    }

    if (address < InRegLinesBase)
        return 0;

    const int relative = address - InRegLinesBase;
    const int lineIndex = relative / LineBlockSize;
    const int offset = relative % LineBlockSize;
    if (lineIndex < 0 || lineIndex >= snapshot.lines.size())
        return 0;

    const LineSnapshot &line = snapshot.lines.at(lineIndex);
    switch (offset) {
    case LineType:
        return lineType(line);
    case LineState:
        return lineState(line, snapshot);
    case LineOutputState:
        return lineOutputState(line);
    case LineLastTestLow:
    case LineLastTestHigh: {
        const QDateTime lastTest = latestDateTime(line.lastFunctionalTest.completedAt,
                                                  line.lastDurationTest.completedAt);
        const quint32 timestamp = dateTimeToU32(lastTest);
        return offset == LineLastTestLow ? lowWord(timestamp) : highWord(timestamp);
    }
    case LineReserved1:
    default:
        return 0;
    }
}

} // namespace

ModbusTcpServer::ModbusTcpServer(QObject *parent)
    : QObject(parent)
    , m_server(new QModbusTcpServer(this))
{
    connect(m_server, &QModbusServer::stateChanged, this, &ModbusTcpServer::onStateChanged);
    connect(m_server, &QModbusServer::errorOccurred, this, &ModbusTcpServer::onErrorOccurred);
    connect(m_server, &QModbusServer::dataWritten, this, &ModbusTcpServer::onDataWritten);

    m_refreshTimer.setInterval(500);
    m_refreshTimer.setSingleShot(false);
    connect(&m_refreshTimer, &QTimer::timeout, this, &ModbusTcpServer::refreshRegisters);
}

ModbusTcpServer::~ModbusTcpServer()
{
    stop();
}

bool ModbusTcpServer::setupServerMap()
{
    if (!m_server)
        return false;

    QModbusDataUnitMap map;
    map.insert(QModbusDataUnit::Coils, QModbusDataUnit(QModbusDataUnit::Coils, 0, CoilCount));
    map.insert(QModbusDataUnit::DiscreteInputs, QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 16));
    map.insert(QModbusDataUnit::InputRegisters,
               QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, InputRegisterCount));
    map.insert(QModbusDataUnit::HoldingRegisters,
               QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 32));

    return m_server->setMap(map);
}

bool ModbusTcpServer::start(const QString &listenAddress, int port, int serverAddress)
{
    if (!m_server)
        return false;

    if (m_server->state() != QModbusDevice::UnconnectedState)
        stop();

    if (!setupServerMap()) {
        emit logMessage(QStringLiteral("Modbus TCP: не удалось создать карту регистров"));
        return false;
    }

    m_server->setConnectionParameter(QModbusDevice::NetworkAddressParameter, listenAddress);
    m_server->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    m_server->setServerAddress(serverAddress);

    if (!m_server->connectDevice()) {
        emit logMessage(QStringLiteral("Modbus TCP start error: %1").arg(m_server->errorString()));
        return false;
    }

    refreshRegisters();
    m_refreshTimer.start();

    emit logMessage(QStringLiteral("Modbus TCP сервер запущен: %1:%2, slave id=%3")
                        .arg(listenAddress)
                        .arg(port)
                        .arg(serverAddress));
    return true;
}

void ModbusTcpServer::stop()
{
    m_refreshTimer.stop();
    if (m_server)
        m_server->disconnectDevice();
}

bool ModbusTcpServer::isRunning() const
{
    return m_server && m_server->state() == QModbusDevice::ConnectedState;
}

void ModbusTcpServer::updateSnapshot(const CabinetSnapshot &snapshot)
{
    m_snapshot = snapshot;
    refreshRegisters();
}

void ModbusTcpServer::refreshRegisters()
{
    if (!m_server)
        return;

    for (int address = 0; address < InputRegisterCount; ++address)
        m_server->setData(QModbusDataUnit::InputRegisters, address, readInputRegister(m_snapshot, address));
}

void ModbusTcpServer::onStateChanged(int state)
{
    QString text;
    switch (state) {
    case QModbusDevice::UnconnectedState: text = QStringLiteral("Unconnected"); break;
    case QModbusDevice::ConnectingState: text = QStringLiteral("Connecting"); break;
    case QModbusDevice::ConnectedState: text = QStringLiteral("Connected"); break;
    case QModbusDevice::ClosingState: text = QStringLiteral("Closing"); break;
    default: text = QStringLiteral("Unknown"); break;
    }

    emit logMessage(QStringLiteral("Modbus TCP state: %1").arg(text));
}

void ModbusTcpServer::onErrorOccurred(QModbusDevice::Error error)
{
    if (error == QModbusDevice::NoError || !m_server)
        return;

    emit logMessage(QStringLiteral("Modbus TCP error: %1").arg(m_server->errorString()));
}

void ModbusTcpServer::onDataWritten(QModbusDataUnit::RegisterType table, int address, int size)
{
    if (!m_server || table != QModbusDataUnit::Coils)
        return;

    for (int offset = 0; offset < size; ++offset) {
        quint16 value = 0;
        if (!m_server->data(QModbusDataUnit::Coils, address + offset, &value))
            continue;
        processWrittenCoil(address + offset, value != 0);
    }
}

void ModbusTcpServer::processWrittenCoil(int address, bool value)
{
    if (!value)
        return;

    bool accepted = true;
    switch (address) {
    case CoilFireOn:
        emit manualEmergencyStartRequested();
        break;
    case CoilFireOff:
        emit manualEmergencyStopRequested();
        break;
    case CoilStopOn:
    case CoilStopOff:
        accepted = false;
        break;
    case CoilStopTest:
        emit stopTestRequested();
        break;
    case CoilStartFunctionalTest:
        emit functionalTestRequested();
        break;
    case CoilStartDurationTest:
        emit durationTestRequested();
        break;
    default:
        accepted = false;
        break;
    }

    emit logMessage(QStringLiteral("Modbus TCP coil %1 = 1, result=%2")
                        .arg(address + 1)
                        .arg(accepted ? QStringLiteral("OK") : QStringLiteral("FAIL")));
    resetCoil(address);
}

void ModbusTcpServer::resetCoil(int address)
{
    if (m_server)
        m_server->setData(QModbusDataUnit::Coils, address, 0);
}

} // namespace DialogG2
