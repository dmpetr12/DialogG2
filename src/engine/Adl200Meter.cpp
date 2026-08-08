#include "Adl200Meter.h"

namespace DialogG2 {

static qint16 signedRegister(quint16 value)
{
    return static_cast<qint16>(value);
}

Adl200Measurement Adl200Meter::decodeRealtimeHoldingRegisters(const QVector<quint16> &registers)
{
    Adl200Measurement measurement;
    if (registers.size() < RealtimeHoldingCount)
        return measurement;

    measurement.valid = true;
    measurement.voltage = registers.at(0) * 0.1;
    measurement.current = registers.at(1) * 0.01;
    measurement.activePower = signedRegister(registers.at(2)) * 1.0;
    measurement.reactivePower = signedRegister(registers.at(3)) * 1.0;
    measurement.apparentPower = registers.at(4) * 1.0;
    measurement.powerFactor = signedRegister(registers.at(5)) * 0.001;
    measurement.frequency = registers.at(6) * 0.01;
    return measurement;
}

} // namespace DialogG2
