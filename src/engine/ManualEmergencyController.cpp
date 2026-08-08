#include "ManualEmergencyController.h"

namespace DialogG2 {

void ManualEmergencyController::reset()
{
    m_active = false;
}

bool ManualEmergencyController::evaluate(const ManualEmergencyInputs &inputs)
{
    if (inputs.startRequested)
        m_active = true;
    if (inputs.stopRequested)
        m_active = false;
    return m_active;
}

bool ManualEmergencyController::active() const
{
    return m_active;
}

} // namespace DialogG2
