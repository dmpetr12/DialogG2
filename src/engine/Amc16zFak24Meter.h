#pragma once

#include <QMetaType>
#include <QVector>

#include <limits>

namespace DialogG2 {

struct Amc16zBranchMeasurement
{
    int channel = 0;
    bool valid = false;
    double activePower = std::numeric_limits<double>::quiet_NaN();
};

class Amc16zFak24Meter
{
public:
    static constexpr int DefaultSlaveAddress = 2;
    static constexpr int OccupiedAddressCount = 2;
    static constexpr int ActivePowerHoldingStart = 0x00C0;
    static constexpr int ActivePowerBranchCount = 24;
    static constexpr int ActivePowerHoldingCount = ActivePowerBranchCount * 2;

    static QVector<Amc16zBranchMeasurement> decodeActivePowerHoldingRegisters(const QVector<quint16> &registers);
};

} // namespace DialogG2

Q_DECLARE_METATYPE(DialogG2::Amc16zBranchMeasurement)
Q_DECLARE_METATYPE(QVector<DialogG2::Amc16zBranchMeasurement>)
