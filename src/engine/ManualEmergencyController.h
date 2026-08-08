#pragma once

namespace DialogG2 {

struct ManualEmergencyInputs
{
    bool startRequested = false;
    bool stopRequested = false;
};

class ManualEmergencyController
{
public:
    void reset();
    bool evaluate(const ManualEmergencyInputs &inputs);
    bool active() const;

private:
    bool m_active = false;
};

} // namespace DialogG2
