#pragma once

#include "CabinetSnapshot.h"

namespace DialogG2 {

struct MaintenanceCheckerConfig
{
    int lineLimitDays = 30;
    int longTestLimitDays = 365;
};

class MaintenanceChecker
{
public:
    explicit MaintenanceChecker(MaintenanceCheckerConfig config = {});

    MaintenanceSnapshot evaluate(const QVector<LineSnapshot> &lines,
                                 const QVector<TestJournalEntry> &journal,
                                 const QDateTime &now) const;

private:
    static QDateTime latestCompletedDurationTest(const QVector<TestJournalEntry> &journal);
    static bool isCompletedTestStatus(TestRunStatus status);
    static QString lineDisplayName(const LineSnapshot &line);

    MaintenanceCheckerConfig m_config;
};

} // namespace DialogG2
