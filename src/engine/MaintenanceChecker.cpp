#include "MaintenanceChecker.h"

#include <QStringList>

namespace DialogG2 {

MaintenanceChecker::MaintenanceChecker(MaintenanceCheckerConfig config)
    : m_config(config)
{
}

MaintenanceSnapshot MaintenanceChecker::evaluate(const QVector<LineSnapshot> &lines,
                                                  const QVector<TestJournalEntry> &journal,
                                                  const QDateTime &now) const
{
    MaintenanceSnapshot snapshot;
    snapshot.lineLimitDays = m_config.lineLimitDays;
    snapshot.longTestLimitDays = m_config.longTestLimitDays;
    snapshot.lastLongTestAt = latestCompletedDurationTest(journal);

    snapshot.longTestOverdue = !snapshot.lastLongTestAt.isValid()
        || snapshot.lastLongTestAt.daysTo(now) > m_config.longTestLimitDays;

    QStringList overdueLineNames;
    for (const LineSnapshot &line : lines) {
        if (!line.enabled)
            continue;

        MaintenanceLineStatus status;
        status.lineIndex = line.index;
        status.lineName = lineDisplayName(line);
        status.lastTestAt = line.lastFunctionalTest.completedAt;
        status.overdue = !status.lastTestAt.isValid()
            || status.lastTestAt.daysTo(now) > m_config.lineLimitDays;

        if (status.overdue) {
            ++snapshot.overdueLinesCount;
            overdueLineNames.append(status.lineName);
        }

        snapshot.lines.append(status);
    }

    snapshot.ok = snapshot.overdueLinesCount == 0 && !snapshot.longTestOverdue;

    QStringList parts;
    if (snapshot.longTestOverdue) {
        parts.append(snapshot.lastLongTestAt.isValid()
            ? QStringLiteral("Тест длительности просрочен, последний: %1")
                  .arg(snapshot.lastLongTestAt.toString(QStringLiteral("dd.MM.yyyy")))
            : QStringLiteral("Тест длительности еще не проводился"));
    }
    if (snapshot.overdueLinesCount > 0) {
        parts.append(QStringLiteral("Просрочены проверки линий: %1")
                         .arg(overdueLineNames.join(QStringLiteral(", "))));
    }

    snapshot.summary = parts.isEmpty()
        ? QStringLiteral("Обслуживание по тестам в срок")
        : parts.join(QStringLiteral("; "));
    return snapshot;
}

QDateTime MaintenanceChecker::latestCompletedDurationTest(const QVector<TestJournalEntry> &journal)
{
    QDateTime latest;
    for (const TestJournalEntry &entry : journal) {
        if (entry.kind != TestKind::Duration || !isCompletedTestStatus(entry.status)
            || !entry.finishedAt.isValid()) {
            continue;
        }

        if (!latest.isValid() || latest < entry.finishedAt)
            latest = entry.finishedAt;
    }
    return latest;
}

bool MaintenanceChecker::isCompletedTestStatus(TestRunStatus status)
{
    return status == TestRunStatus::Passed || status == TestRunStatus::Failed;
}

QString MaintenanceChecker::lineDisplayName(const LineSnapshot &line)
{
    return line.name.isEmpty() ? QStringLiteral("Линия %1").arg(line.index) : line.name;
}

} // namespace DialogG2
