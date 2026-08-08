#pragma once

#include "CabinetSnapshot.h"

#include <QByteArray>
#include <QString>

namespace DialogG2 {

struct JbdBmsResponse
{
    quint8 command = 0;
    quint8 status = 0;
    QByteArray data;
    QByteArray callbackId;

    bool ok() const { return status == 0; }
};

class JbdBmsProtocol
{
public:
    static constexpr quint8 CommandBasicInfo = 0x03;
    static constexpr quint8 CommandCellVoltages = 0x04;
    static constexpr quint8 CommandHardwareVersion = 0x05;
    static constexpr quint8 CommandProtectionCounters = 0xAA;

    static QByteArray buildReadCommand(quint8 command, const QByteArray &callbackId = {});
    static bool parseResponse(const QByteArray &frame, JbdBmsResponse *response, QString *error = nullptr);

    static BatterySnapshot decodeBasicInfo(const QByteArray &data);
    static void applyCellVoltages(BatterySnapshot *battery, const QByteArray &data);

private:
    static quint16 checksum(const QByteArray &bytes);
};

} // namespace DialogG2
