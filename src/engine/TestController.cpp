#include "TestController.h"

#include <algorithm>
#include <cmath>

namespace DialogG2 {

TestController::TestController(TestControllerConfig config)
    : m_config(config)
{
}

void TestController::reset()
{
    m_activeTest = {};
    m_journal.clear();
}

TestControllerResult TestController::evaluate(const TestControllerInputs &inputs)
{
    TestControllerResult result;
    result.lines = inputs.lines;

    const Candidate requested = highestRequestedTest(inputs, m_config);
    const Candidate active = activeCandidate();
    const QString priorityReason = hasActiveTest() ? interruptionReason(inputs, active) : QString();

    if (hasActiveTest() && !priorityReason.isEmpty()) {
        const TestJournalEntry entry = finishActiveTest(result.lines,
                                                        inputs.now,
                                                        TestRunStatus::InterruptedByPriority,
                                                        priorityReason);
        m_journal.append(entry);
        result.newJournalEntries.append(entry);
    }

    if (hasActiveTest() && requested.valid && requested.priority > activeCandidate().priority) {
        const TestJournalEntry entry = finishActiveTest(result.lines,
                                                        inputs.now,
                                                        TestRunStatus::InterruptedByPriority,
                                                        QStringLiteral("прерван по приоритету"));
        m_journal.append(entry);
        result.newJournalEntries.append(entry);
    }

    bool stopBlocksStart = inputs.stopRequested;
    if (hasActiveTest() && inputs.stopRequested) {
        const TestJournalEntry entry = finishActiveTest(result.lines,
                                                        inputs.now,
                                                        TestRunStatus::StoppedByOperator,
                                                        QStringLiteral("остановлен оператором"));
        m_journal.append(entry);
        result.newJournalEntries.append(entry);
    }

    if (!stopBlocksStart && !hasActiveTest() && requested.valid && inputs.voltageControlOk && !inputs.fireInputActive)
        startTest(requested, inputs.now);

    if (hasActiveTest() && activeTestDue(inputs.now)) {
        const TestKind finishedKind = m_activeTest.kind;
        TestJournalEntry entry = finishActiveTest(result.lines, inputs.now, TestRunStatus::Passed, QString());
        m_journal.append(entry);
        result.newJournalEntries.append(entry);

        for (LineSnapshot &line : result.lines) {
            for (const TestLineMeasurement &measurement : entry.lines) {
                if (measurement.lineIndex != line.index)
                    continue;

                if (finishedKind == TestKind::Functional)
                    line.lastFunctionalTest = lineResultFromMeasurement(measurement, entry.finishedAt);
                else if (finishedKind == TestKind::Duration)
                    line.lastDurationTest = lineResultFromMeasurement(measurement, entry.finishedAt);

                if (measurement.status == TestRunStatus::Failed)
                    line.state = LineState::Fault;
                break;
            }
        }
    }

    if (hasActiveTest()) {
        result.activeTest = m_activeTest;
        result.testKind = m_activeTest.kind;
        result.testSource = m_activeTest.source;
        result.modeRelayOn = true;
        result.manualTestActive = m_activeTest.source == TestSource::Manual;
        result.scheduledTestActive = m_activeTest.source == TestSource::Scheduled;
    }

    result.journalEntries = m_journal;
    return result;
}

bool TestController::hasActiveTest() const
{
    return m_activeTest.active;
}

TestController::Candidate TestController::activeCandidate() const
{
    Candidate candidate;
    if (!m_activeTest.active)
        return candidate;

    candidate.valid = true;
    candidate.kind = m_activeTest.kind;
    candidate.source = m_activeTest.source;
    candidate.durationSeconds = m_activeTest.durationSeconds;
    candidate.priority = priority(candidate.source, candidate.kind);
    return candidate;
}

void TestController::startTest(const Candidate &candidate, const QDateTime &now)
{
    m_activeTest = {};
    m_activeTest.active = true;
    m_activeTest.kind = candidate.kind;
    m_activeTest.source = candidate.source;
    m_activeTest.startedAt = now;
    m_activeTest.warmupSeconds = candidate.kind == TestKind::Functional
        ? std::max(0, m_config.functionalWarmupSeconds)
        : 0;
    m_activeTest.durationSeconds = candidate.kind == TestKind::Duration
        ? std::max(1, candidate.durationSeconds)
        : 0;

    const int waitSeconds = candidate.kind == TestKind::Duration
        ? m_activeTest.durationSeconds
        : m_activeTest.warmupSeconds;
    m_activeTest.dueAt = now.addSecs(waitSeconds);
}

TestJournalEntry TestController::finishActiveTest(const QVector<LineSnapshot> &lines,
                                                  const QDateTime &now,
                                                  TestRunStatus status,
                                                  const QString &reason)
{
    TestJournalEntry entry;
    entry.kind = m_activeTest.kind;
    entry.source = m_activeTest.source;
    entry.startedAt = m_activeTest.startedAt;
    entry.finishedAt = now;
    entry.status = status;
    entry.reason = reason;

    if (status == TestRunStatus::Passed || status == TestRunStatus::Failed) {
        bool hasFailures = false;
        for (const LineSnapshot &line : lines) {
            if (!line.enabled)
                continue;

            TestLineMeasurement measurement = measureLine(line, entry.kind, m_config.durationToleranceMultiplier);
            if (measurement.status == TestRunStatus::Failed)
                hasFailures = true;
            entry.lines.append(measurement);
        }
        entry.status = hasFailures ? TestRunStatus::Failed : TestRunStatus::Passed;
    }

    m_activeTest = {};
    return entry;
}

bool TestController::activeTestDue(const QDateTime &now) const
{
    return m_activeTest.active && m_activeTest.dueAt.isValid() && now >= m_activeTest.dueAt;
}

TestController::Candidate TestController::highestRequestedTest(const TestControllerInputs &inputs,
                                                               const TestControllerConfig &config)
{
    QVector<Candidate> candidates;
    candidates.append({inputs.manualDuration.active,
                       priority(TestSource::Manual, TestKind::Duration),
                       TestKind::Duration,
                       TestSource::Manual,
                       inputs.manualDuration.durationSeconds > 0
                           ? inputs.manualDuration.durationSeconds
                           : config.defaultDurationSeconds});
    candidates.append({inputs.manualFunctional.active,
                       priority(TestSource::Manual, TestKind::Functional),
                       TestKind::Functional,
                       TestSource::Manual,
                       0});
    candidates.append({inputs.scheduledDuration.active,
                       priority(TestSource::Scheduled, TestKind::Duration),
                       TestKind::Duration,
                       TestSource::Scheduled,
                       inputs.scheduledDuration.durationSeconds > 0
                           ? inputs.scheduledDuration.durationSeconds
                           : config.defaultDurationSeconds});
    candidates.append({inputs.scheduledFunctional.active,
                       priority(TestSource::Scheduled, TestKind::Functional),
                       TestKind::Functional,
                       TestSource::Scheduled,
                       0});

    Candidate best;
    for (const Candidate &candidate : candidates) {
        if (!candidate.valid)
            continue;
        if (!best.valid || candidate.priority > best.priority)
            best = candidate;
    }
    return best;
}

int TestController::priority(TestSource source, TestKind kind)
{
    if (source == TestSource::Manual && kind == TestKind::Duration)
        return 4;
    if (source == TestSource::Manual && kind == TestKind::Functional)
        return 3;
    if (source == TestSource::Scheduled && kind == TestKind::Duration)
        return 2;
    if (source == TestSource::Scheduled && kind == TestKind::Functional)
        return 1;
    return 0;
}

QString TestController::interruptionReason(const TestControllerInputs &inputs, const Candidate &)
{
    if (!inputs.voltageControlOk)
        return QStringLiteral("прерван по приоритету: пропало напряжение");
    if (inputs.fireInputActive)
        return QStringLiteral("прерван по приоритету: пожар");
    return {};
}

TestLineMeasurement TestController::measureLine(const LineSnapshot &line,
                                                TestKind kind,
                                                double durationToleranceMultiplier)
{
    TestLineMeasurement measurement;
    measurement.lineIndex = line.index;
    measurement.lineName = line.name;
    measurement.measuredPower = line.outputPower;
    measurement.nominalPower = line.nominalPower;

    const double baseTolerance = std::isfinite(line.powerTestTolerancePercent)
        ? line.powerTestTolerancePercent
        : 5.0;
    measurement.tolerancePercent = kind == TestKind::Duration
        ? baseTolerance * std::max(1.0, durationToleranceMultiplier)
        : baseTolerance;

    if (!std::isfinite(measurement.measuredPower) || !std::isfinite(measurement.nominalPower)
        || measurement.nominalPower <= 0.0) {
        measurement.status = TestRunStatus::Failed;
        measurement.details = QStringLiteral("нет корректного измерения мощности");
        return measurement;
    }

    const double deltaPercent = std::abs(measurement.measuredPower - measurement.nominalPower)
        * 100.0 / measurement.nominalPower;
    if (deltaPercent <= measurement.tolerancePercent) {
        measurement.status = TestRunStatus::Passed;
        measurement.details = QStringLiteral("мощность в допуске");
    } else {
        measurement.status = TestRunStatus::Failed;
        measurement.details = QStringLiteral("отклонение мощности %1%, допуск %2%")
            .arg(deltaPercent, 0, 'f', 1)
            .arg(measurement.tolerancePercent, 0, 'f', 1);
    }
    return measurement;
}

LineTestResult TestController::lineResultFromMeasurement(const TestLineMeasurement &measurement,
                                                         const QDateTime &completedAt)
{
    LineTestResult result;
    result.completedAt = completedAt;
    result.status = measurement.status;
    result.measuredPower = measurement.measuredPower;
    result.nominalPower = measurement.nominalPower;
    result.tolerancePercent = measurement.tolerancePercent;
    result.details = measurement.details;
    return result;
}

} // namespace DialogG2
