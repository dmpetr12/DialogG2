#include "Amc16zFak24Meter.h"

#include <cstring>

namespace DialogG2 {

static float floatFromRegisters(quint16 highWord, quint16 lowWord)
{
    const quint32 raw = (static_cast<quint32>(highWord) << 16)
        | static_cast<quint32>(lowWord);

    float value = 0.0f;
    static_assert(sizeof(value) == sizeof(raw), "float must be 32-bit");
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}

QVector<Amc16zBranchMeasurement> Amc16zFak24Meter::decodeActivePowerHoldingRegisters(const QVector<quint16> &registers)
{
    QVector<Amc16zBranchMeasurement> measurements;
    if (registers.size() < ActivePowerHoldingCount)
        return measurements;

    measurements.reserve(ActivePowerBranchCount);
    for (int i = 0; i < ActivePowerBranchCount; ++i) {
        Amc16zBranchMeasurement measurement;
        measurement.channel = i + 1;
        measurement.valid = true;
        measurement.activePower = floatFromRegisters(registers.at(i * 2), registers.at(i * 2 + 1)) * 1000.0;
        measurements.append(measurement);
    }

    return measurements;
}

} // namespace DialogG2
