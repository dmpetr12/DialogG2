#pragma once

#include "CabinetSnapshot.h"

#include <QDateTime>
#include <QVector>

namespace DialogG2 {

struct TestRequest
{
    bool active = false;
    int durationSeconds = 0;
};

struct TestControllerConfig
{
    int functionalWarmupSeconds = 120;
    int defaultDurationSeconds = 3600;
    double durationToleranceMultiplier = 2.0;
};

struct TestControllerInputs
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    bool voltageControlOk = true;
    bool fireInputActive = false;
    bool stopRequested = false;
    QVector<LineSnapshot> lines;

    TestRequest manualFunctional;
    TestRequest manualDuration;
    TestRequest scheduledFunctional;
    TestRequest scheduledDuration;
};

struct TestControllerResult
{
    bool manualTestActive = false;
    bool scheduledTestActive = false;
    bool modeRelayOn = false;
    TestKind testKind = TestKind::None;
    TestSource testSource = TestSource::None;
    ActiveTestSnapshot activeTest;
    QVector<LineSnapshot> lines;
    QVector<TestJournalEntry> journalEntries;
    QVector<TestJournalEntry> newJournalEntries;
};

class TestController
{
public:
    explicit TestController(TestControllerConfig config = {});

    void reset();
    TestControllerResult evaluate(const TestControllerInputs &inputs);

private:
    struct Candidate
    {
        bool valid = false;
        int priority = 0;
        TestKind kind = TestKind::None;
        TestSource source = TestSource::None;
        int durationSeconds = 0;
    };

    bool hasActiveTest() const;
    Candidate activeCandidate() const;
    void startTest(const Candidate &candidate, const QDateTime &now);
    TestJournalEntry finishActiveTest(const QVector<LineSnapshot> &lines,
                                      const QDateTime &now,
                                      TestRunStatus status,
                                      const QString &reason);
    bool activeTestDue(const QDateTime &now) const;

    static Candidate highestRequestedTest(const TestControllerInputs &inputs, const TestControllerConfig &config);
    static int priority(TestSource source, TestKind kind);
    static QString interruptionReason(const TestControllerInputs &inputs, const Candidate &candidate);
    static TestLineMeasurement measureLine(const LineSnapshot &line,
                                           TestKind kind,
                                           double durationToleranceMultiplier);
    static LineTestResult lineResultFromMeasurement(const TestLineMeasurement &measurement,
                                                    const QDateTime &completedAt);

    TestControllerConfig m_config;
    ActiveTestSnapshot m_activeTest;
    QVector<TestJournalEntry> m_journal;
};

} // namespace DialogG2
