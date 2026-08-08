#include "ModbusBusMonitor.h"

#include <algorithm>

namespace DialogG2 {

ModbusBusMonitor::ModbusBusMonitor(int offlineFailureThreshold)
{
    setOfflineFailureThreshold(offlineFailureThreshold);
}

void ModbusBusMonitor::setOfflineFailureThreshold(int threshold)
{
    m_offlineFailureThreshold = std::max(1, threshold);
}

void ModbusBusMonitor::markSuccess()
{
    m_status.online = true;
    m_status.consecutiveFailures = 0;
    m_status.lastError.clear();
}

void ModbusBusMonitor::markFailure(const QString &error)
{
    ++m_status.consecutiveFailures;
    m_status.lastError = error;
    if (m_status.consecutiveFailures >= m_offlineFailureThreshold)
        m_status.online = false;
}

void ModbusBusMonitor::reset()
{
    m_status = {};
}

ModbusBusStatus ModbusBusMonitor::status() const
{
    return m_status;
}

} // namespace DialogG2
