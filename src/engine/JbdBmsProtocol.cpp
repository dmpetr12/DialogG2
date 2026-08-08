#include "JbdBmsProtocol.h"

#include <QtGlobal>

#include <algorithm>
#include <limits>

namespace DialogG2 {

static quint8 byteAt(const QByteArray &data, int index)
{
    return static_cast<quint8>(data.at(index));
}

static quint16 u16At(const QByteArray &data, int index)
{
    return static_cast<quint16>((byteAt(data, index) << 8) | byteAt(data, index + 1));
}

static qint16 i16At(const QByteArray &data, int index)
{
    return static_cast<qint16>(u16At(data, index));
}

static void setError(QString *error, const QString &message)
{
    if (error)
        *error = message;
}

static QStringList labelsForBits(quint16 bits, const QStringList &labels)
{
    QStringList result;
    for (int i = 0; i < labels.size(); ++i) {
        if (bits & (1u << i))
            result.append(labels.at(i));
    }
    return result;
}

quint16 JbdBmsProtocol::checksum(const QByteArray &bytes)
{
    quint32 sum = 0;
    for (char ch : bytes)
        sum += static_cast<quint8>(ch);
    return static_cast<quint16>((~sum + 1u) & 0xFFFFu);
}

QByteArray JbdBmsProtocol::buildReadCommand(quint8 command, const QByteArray &callbackId)
{
    QByteArray frame;
    frame.append(char(0xDD));
    frame.append(char(0xA5));
    frame.append(char(command));
    frame.append(char(0x00));
    frame.append(callbackId.left(4));

    QByteArray checked;
    checked.append(char(command));
    checked.append(char(0x00));
    const quint16 check = checksum(checked);
    frame.append(char((check >> 8) & 0xFF));
    frame.append(char(check & 0xFF));
    frame.append(char(0x77));
    return frame;
}

bool JbdBmsProtocol::parseResponse(const QByteArray &frame, JbdBmsResponse *response, QString *error)
{
    if (frame.size() < 7) {
        setError(error, QStringLiteral("BMS frame is too short"));
        return false;
    }
    if (byteAt(frame, 0) != 0xDD || byteAt(frame, frame.size() - 1) != 0x77) {
        setError(error, QStringLiteral("BMS frame has invalid start/stop byte"));
        return false;
    }

    const quint8 length = byteAt(frame, 3);
    const int dataStart = 4;
    const int dataEnd = dataStart + length;
    if (dataEnd + 3 > frame.size()) {
        setError(error, QStringLiteral("BMS frame length exceeds packet size"));
        return false;
    }

    QByteArray checked;
    checked.append(frame.mid(2, 2));
    checked.append(frame.mid(dataStart, length));
    const quint16 expected = checksum(checked);

    const int checkIndex = frame.size() - 3;
    const quint16 packetChecksum = u16At(frame, checkIndex);
    if (packetChecksum != expected) {
        setError(error, QStringLiteral("BMS frame checksum mismatch"));
        return false;
    }

    if (response) {
        response->command = byteAt(frame, 1);
        response->status = byteAt(frame, 2);
        response->data = frame.mid(dataStart, length);
        response->callbackId = frame.mid(dataEnd, checkIndex - dataEnd);
    }
    return true;
}

BatterySnapshot JbdBmsProtocol::decodeBasicInfo(const QByteArray &data)
{
    BatterySnapshot battery;
    battery.connected = true;
    battery.communicationOk = true;

    if (data.size() < 23) {
        battery.connected = false;
        battery.communicationOk = false;
        battery.state = BatteryState::Disconnected;
        battery.faults.append(QStringLiteral("нет связи с BMS"));
        return battery;
    }

    const quint8 fet = byteAt(data, 20);
    const bool wideUnits = (fet & 0x80) != 0;
    const double currentUnit = wideUnits ? 0.1 : 0.01;
    const double capacityUnit = wideUnits ? 0.1 : 0.01;

    battery.voltage = u16At(data, 0) * 0.01;
    battery.current = i16At(data, 2) * currentUnit;
    battery.remainingCapacityAh = u16At(data, 4) * capacityUnit;
    battery.nominalCapacityAh = u16At(data, 6) * capacityUnit;
    battery.cycleCount = u16At(data, 8);
    battery.protectionStatusRaw = u16At(data, 16);
    battery.socPercent = byteAt(data, 19);
    battery.chargeAllowed = (fet & 0x01) != 0;
    battery.dischargeAllowed = (fet & 0x02) != 0;
    battery.heatingActive = (fet & 0x08) != 0;
    battery.cellCount = byteAt(data, 21);

    const quint8 ntcCount = byteAt(data, 22);
    int offset = 23;
    battery.temperatures.clear();
    for (int i = 0; i < ntcCount && offset + 1 < data.size(); ++i, offset += 2)
        battery.temperatures.append((u16At(data, offset) - 2731) / 10.0);

    if (!battery.temperatures.isEmpty()) {
        auto minmax = std::minmax_element(battery.temperatures.cbegin(), battery.temperatures.cend());
        battery.minTemperature = *minmax.first;
        battery.maxTemperature = *minmax.second;
    }

    if (offset < data.size())
        ++offset;
    if (offset + 1 < data.size()) {
        battery.alarmStatusRaw = u16At(data, offset);
        offset += 2;
    }
    if (offset + 1 < data.size())
        battery.fullChargeCapacityAh = u16At(data, offset) * capacityUnit;

    battery.balancingActive = (u16At(data, 12) | u16At(data, 14)) != 0;
    battery.faults = labelsForBits(battery.protectionStatusRaw, {
        QStringLiteral("перенапряжение ячейки"),
        QStringLiteral("пониженное напряжение ячейки"),
        QStringLiteral("перенапряжение батареи"),
        QStringLiteral("пониженное напряжение батареи"),
        QStringLiteral("перегрев при заряде"),
        QStringLiteral("низкая температура при заряде"),
        QStringLiteral("перегрев при разряде"),
        QStringLiteral("низкая температура при разряде"),
        QStringLiteral("сверхток заряда"),
        QStringLiteral("сверхток разряда"),
        QStringLiteral("короткое замыкание"),
        QStringLiteral("ошибка front-end IC"),
        QStringLiteral("software lock MOS"),
        QStringLiteral("пробой charge MOS"),
        QStringLiteral("пробой discharge MOS"),
        QStringLiteral("резерв защиты")
    });
    battery.warnings = labelsForBits(battery.alarmStatusRaw, {
        QStringLiteral("высокое напряжение ячейки"),
        QStringLiteral("низкое напряжение ячейки"),
        QStringLiteral("высокое напряжение батареи"),
        QStringLiteral("низкое напряжение батареи"),
        QStringLiteral("высокая температура заряда"),
        QStringLiteral("низкая температура заряда"),
        QStringLiteral("высокая температура разряда"),
        QStringLiteral("низкая температура разряда"),
        QStringLiteral("высокий ток заряда"),
        QStringLiteral("высокий ток разряда"),
        QStringLiteral("большой разброс ячеек"),
        QStringLiteral("низкая ёмкость")
    });

    if (!battery.faults.isEmpty())
        battery.state = BatteryState::Fault;
    else if (!battery.warnings.isEmpty())
        battery.state = BatteryState::Warning;
    else
        battery.state = BatteryState::Normal;

    return battery;
}

void JbdBmsProtocol::applyCellVoltages(BatterySnapshot *battery, const QByteArray &data)
{
    if (!battery)
        return;

    battery->cellVoltages.clear();
    for (int offset = 0; offset + 1 < data.size(); offset += 2)
        battery->cellVoltages.append(u16At(data, offset) / 1000.0);

    battery->cellCount = battery->cellVoltages.size();
    if (battery->cellVoltages.isEmpty()) {
        battery->minCellVoltage = std::numeric_limits<double>::quiet_NaN();
        battery->maxCellVoltage = std::numeric_limits<double>::quiet_NaN();
        battery->cellVoltageDelta = std::numeric_limits<double>::quiet_NaN();
        return;
    }

    auto minmax = std::minmax_element(battery->cellVoltages.cbegin(), battery->cellVoltages.cend());
    battery->minCellVoltage = *minmax.first;
    battery->maxCellVoltage = *minmax.second;
    battery->cellVoltageDelta = battery->maxCellVoltage - battery->minCellVoltage;
}

} // namespace DialogG2
