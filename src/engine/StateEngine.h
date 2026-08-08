#pragma once

#include "CabinetSnapshot.h"

#include <limits>

namespace DialogG2 {

struct EngineInputs
{
    bool voltageControlOk = true;
    bool fireInputActive = false;
    bool manualEmergencyActive = false;
    bool manualTestActive = false;
    bool scheduledTestActive = false;
    TestKind testKind = TestKind::None;
    TestSource testSource = TestSource::None;
    ActiveTestSnapshot activeTest;
    QVector<TestJournalEntry> testJournal;

    bool relayFault = false;
    bool modbusFault = false;
    bool batteryFault = false;
    bool leakageFault = false;
    bool temperatureFault = false;

    double inputVoltage = std::numeric_limits<double>::quiet_NaN();
    double inputCurrent = std::numeric_limits<double>::quiet_NaN();
    double inputPower = std::numeric_limits<double>::quiet_NaN();
    double inputFrequency = std::numeric_limits<double>::quiet_NaN();
    double temperature = std::numeric_limits<double>::quiet_NaN();
    BatterySnapshot battery;
    QVector<LineSnapshot> lines;
};

class StateEngine
{
public:
    CabinetSnapshot evaluate(const EngineInputs &inputs) const;

private:
    static CabinetMode resolveMode(const EngineInputs &inputs);
    static QString makeExplanation(CabinetMode mode, SystemHealth health, const QStringList &faults);
    static QStringList collectFaults(const EngineInputs &inputs);
};

} // namespace DialogG2
