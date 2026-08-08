#pragma once

#include <QMetaType>
#include <QVector>

#include <limits>

namespace DialogG2 {

struct WhdMeasurement
{
    bool valid = false;
    double temperature = std::numeric_limits<double>::quiet_NaN();
    double humidity = std::numeric_limits<double>::quiet_NaN();
};

class WhdTemperatureHumidityController
{
public:
    static constexpr int DefaultSlaveAddress = 5;
    static constexpr int Channel1RealtimeRegisterStart = 0x0001;
    static constexpr int Channel1RealtimeRegisterCount = 2;

    static WhdMeasurement decodeChannel1RealtimeRegisters(const QVector<quint16> &registers);
};

} // namespace DialogG2

Q_DECLARE_METATYPE(DialogG2::WhdMeasurement)
