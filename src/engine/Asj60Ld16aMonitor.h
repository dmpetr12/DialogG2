#pragma once

#include <QMetaType>
#include <QVector>

#include <limits>

namespace DialogG2 {

enum class Asj60ChannelStatus
{
    Normal = 0,
    Warning = 1,
    Alarm = 2,
    Unknown = 255
};

struct Asj60LeakageChannel
{
    int channel = 0;
    bool valid = false;
    Asj60ChannelStatus status = Asj60ChannelStatus::Unknown;
    double leakageCurrent = std::numeric_limits<double>::quiet_NaN();
};

class Asj60Ld16aMonitor
{
public:
    static constexpr int DefaultSlaveAddress = 4;
    static constexpr int ChannelDataHoldingStart = 0x0011;
    static constexpr int ChannelCount = 16;
    static constexpr int RegistersPerChannel = 2;
    static constexpr int ChannelDataHoldingCount = ChannelCount * RegistersPerChannel;

    static QVector<Asj60LeakageChannel> decodeChannelHoldingRegisters(const QVector<quint16> &registers);
};

} // namespace DialogG2

Q_DECLARE_METATYPE(DialogG2::Asj60LeakageChannel)
Q_DECLARE_METATYPE(QVector<DialogG2::Asj60LeakageChannel>)
