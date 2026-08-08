#include "LineOperationalMonitor.h"

#include <cmath>

namespace DialogG2 {

LineOperationalMonitor::LineOperationalMonitor(LineOperationalMonitorConfig config)
    : m_config(config)
{
}

void LineOperationalMonitor::reset()
{
    m_onSince.clear();
}

QVector<LineSnapshot> LineOperationalMonitor::evaluate(const QVector<LineSnapshot> &lines,
                                                       const QDateTime &now)
{
    QVector<LineSnapshot> result = lines;

    QHash<int, bool> seen;
    for (LineSnapshot &line : result) {
        seen.insert(line.index, true);

        const bool shouldCheck = line.enabled && line.outputState == LineOutputState::On;
        if (!shouldCheck) {
            m_onSince.remove(line.index);
            line.operationalCheck.state = LineOperationalState::NotChecked;
            line.operationalCheck.startedAt = {};
            line.operationalCheck.warmupSeconds = m_config.warmupSeconds;
            line.operationalCheck.measuredPower = line.outputPower;
            line.operationalCheck.nominalPower = line.nominalPower;
            line.operationalCheck.tolerancePercent = line.powerTestTolerancePercent;
            line.operationalCheck.details = QStringLiteral("линия выключена");
            continue;
        }

        if (!m_onSince.contains(line.index))
            m_onSince.insert(line.index, now);

        line.operationalCheck = checkLine(line, m_onSince.value(line.index), now, m_config.warmupSeconds);
        if (line.operationalCheck.state == LineOperationalState::Fault
            || line.operationalCheck.state == LineOperationalState::NoMeasurement) {
            line.state = LineState::Fault;
        }
    }

    for (auto it = m_onSince.begin(); it != m_onSince.end();) {
        if (!seen.contains(it.key()))
            it = m_onSince.erase(it);
        else
            ++it;
    }

    return result;
}

LineOperationalCheck LineOperationalMonitor::checkLine(const LineSnapshot &line,
                                                       const QDateTime &startedAt,
                                                       const QDateTime &now,
                                                       int warmupSeconds)
{
    LineOperationalCheck check;
    check.startedAt = startedAt;
    check.warmupSeconds = warmupSeconds;
    check.measuredPower = line.outputPower;
    check.nominalPower = line.nominalPower;
    check.tolerancePercent = std::isfinite(line.powerTestTolerancePercent)
        ? line.powerTestTolerancePercent
        : 5.0;

    if (startedAt.isValid() && startedAt.secsTo(now) < warmupSeconds) {
        check.state = LineOperationalState::WarmingUp;
        check.details = QStringLiteral("ожидание прогрева линии");
        return check;
    }

    if (!std::isfinite(check.measuredPower) || !std::isfinite(check.nominalPower)
        || check.nominalPower <= 0.0) {
        check.state = LineOperationalState::NoMeasurement;
        check.details = QStringLiteral("нет корректного измерения мощности");
        return check;
    }

    const double deltaPercent = std::abs(check.measuredPower - check.nominalPower)
        * 100.0 / check.nominalPower;
    if (deltaPercent <= check.tolerancePercent) {
        check.state = LineOperationalState::Normal;
        check.details = QStringLiteral("мощность в допуске");
    } else {
        check.state = LineOperationalState::Fault;
        check.details = QStringLiteral("отклонение мощности %1%, допуск %2%")
            .arg(deltaPercent, 0, 'f', 1)
            .arg(check.tolerancePercent, 0, 'f', 1);
    }
    return check;
}

} // namespace DialogG2
