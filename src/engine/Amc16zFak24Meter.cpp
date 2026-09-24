#include "Amc16zFak24Meter.h"

#include <cstring>
#include <cmath>

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

QVector<double> Amc16zFak24Meter::decodeRmsHoldingRegisters(const QVector<quint16> &registers)
{
    if (registers.size() < ActivePowerHoldingCount)
        return {};
    QVector<double> values;
    values.reserve(ActivePowerBranchCount);
    for (int i = 0; i < ActivePowerBranchCount; ++i) {
        const double value = floatFromRegisters(registers.at(i * 2), registers.at(i * 2 + 1));
        values.append(std::isfinite(value) && value >= 0 ? value : std::numeric_limits<double>::quiet_NaN());
    }
    return values;
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
        // Cabinet branches only supply loads; CT direction must not invert load checks.
        measurement.activePower = std::abs(double(floatFromRegisters(registers.at(i * 2), registers.at(i * 2 + 1)))) * 1000.0;
        measurement.valid = std::isfinite(measurement.activePower);
        measurements.append(measurement);
    }

    return measurements;
}

} // namespace DialogG2
