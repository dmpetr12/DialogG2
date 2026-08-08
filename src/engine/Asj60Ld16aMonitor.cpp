#include "Asj60Ld16aMonitor.h"

namespace DialogG2 {

static Asj60ChannelStatus statusFromRegister(quint16 value)
{
    switch (value) {
    case 0: return Asj60ChannelStatus::Normal;
    case 1: return Asj60ChannelStatus::Warning;
    case 2: return Asj60ChannelStatus::Alarm;
    default: return Asj60ChannelStatus::Unknown;
    }
}

QVector<Asj60LeakageChannel> Asj60Ld16aMonitor::decodeChannelHoldingRegisters(const QVector<quint16> &registers)
{
    QVector<Asj60LeakageChannel> channels;
    if (registers.size() < ChannelDataHoldingCount)
        return channels;

    channels.reserve(ChannelCount);
    for (int i = 0; i < ChannelCount; ++i) {
        Asj60LeakageChannel channel;
        channel.channel = i + 1;
        channel.valid = true;
        channel.status = statusFromRegister(registers.at(i * RegistersPerChannel));
        channel.leakageCurrent = registers.at(i * RegistersPerChannel + 1);
        channels.append(channel);
    }

    return channels;
}

} // namespace DialogG2
