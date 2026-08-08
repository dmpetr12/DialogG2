#pragma once

#include <QMetaType>
#include <QVector>

#include <limits>

namespace DialogG2 {

struct Adl200Measurement
{
    bool valid = false;
    double voltage = std::numeric_limits<double>::quiet_NaN();
    double current = std::numeric_limits<double>::quiet_NaN();
    double activePower = std::numeric_limits<double>::quiet_NaN();
    double reactivePower = std::numeric_limits<double>::quiet_NaN();
    double apparentPower = std::numeric_limits<double>::quiet_NaN();
    double powerFactor = std::numeric_limits<double>::quiet_NaN();
    double frequency = std::numeric_limits<double>::quiet_NaN();
};

class Adl200Meter
{
public:
    static constexpr int DefaultSlaveAddress = 1;
    static constexpr int RealtimeHoldingStart = 0x000B;
    static constexpr int RealtimeHoldingCount = 7;

    static Adl200Measurement decodeRealtimeHoldingRegisters(const QVector<quint16> &registers);
};

} // namespace DialogG2

Q_DECLARE_METATYPE(DialogG2::Adl200Measurement)
