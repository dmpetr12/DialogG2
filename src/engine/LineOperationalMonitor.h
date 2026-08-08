#pragma once

#include "CabinetSnapshot.h"

#include <QHash>
#include <QVector>

namespace DialogG2 {

struct LineOperationalMonitorConfig
{
    int warmupSeconds = 120;
};

class LineOperationalMonitor
{
public:
    explicit LineOperationalMonitor(LineOperationalMonitorConfig config = {});

    void reset();
    QVector<LineSnapshot> evaluate(const QVector<LineSnapshot> &lines, const QDateTime &now);

private:
    static LineOperationalCheck checkLine(const LineSnapshot &line,
                                          const QDateTime &startedAt,
                                          const QDateTime &now,
                                          int warmupSeconds);

    LineOperationalMonitorConfig m_config;
    QHash<int, QDateTime> m_onSince;
};

} // namespace DialogG2
