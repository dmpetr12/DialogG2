#include "WhdTemperatureHumidityController.h"

#include <QVector>

namespace DialogG2 {

static double signedTenths(quint16 value)
{
    return static_cast<qint16>(value) / 10.0;
}

WhdMeasurement WhdTemperatureHumidityController::decodeChannel1RealtimeRegisters(const QVector<quint16> &registers)
{
    WhdMeasurement measurement;
    if (registers.size() < Channel1RealtimeRegisterCount)
        return measurement;

    measurement.valid = true;
    measurement.temperature = signedTenths(registers.at(0));
    measurement.humidity = signedTenths(registers.at(1));
    return measurement;
}

} // namespace DialogG2
