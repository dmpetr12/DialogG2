#pragma once

#include <QString>

namespace DialogG2 {

struct ModbusBusStatus
{
    bool online = false;
    int consecutiveFailures = 0;
    QString lastError;
};

class ModbusBusMonitor
{
public:
    explicit ModbusBusMonitor(int offlineFailureThreshold = 3);

    void setOfflineFailureThreshold(int threshold);
    void markSuccess();
    void markFailure(const QString &error);
    void reset();

    ModbusBusStatus status() const;

private:
    int m_offlineFailureThreshold = 3;
    ModbusBusStatus m_status;
};

} // namespace DialogG2
