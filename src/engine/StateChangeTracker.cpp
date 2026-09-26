#include "StateChangeTracker.h"

namespace DialogG2 {
namespace {

QString modeLogText(const CabinetSnapshot &snapshot)
{
    switch (snapshot.mode) {
    case CabinetMode::Normal:
        return QStringLiteral("РАБОЧИЙ");
    case CabinetMode::Emergency:
        return QStringLiteral("НЕИСПРАВНОСТЬ (реле контроля напряжения)");
    case CabinetMode::Fire:
        return QStringLiteral("ПОЖАР");
    case CabinetMode::ManualTest:
    case CabinetMode::ScheduledTest: {
        const QString source = snapshot.mode == CabinetMode::ManualTest
            ? QStringLiteral("ручной")
            : QStringLiteral("по расписанию");
        const QString kind = snapshot.testKind == TestKind::Functional
            ? QStringLiteral("исправность")
            : snapshot.testKind == TestKind::Duration
                ? QStringLiteral("длительность")
                : QString();
        return kind.isEmpty()
            ? QStringLiteral("ТЕСТ (%1)").arg(source)
            : QStringLiteral("ТЕСТ (%1, %2)").arg(source, kind);
    }
    }
    return QStringLiteral("НЕИЗВЕСТНО");
}

QString systemLogText(SystemHealth health)
{
    return health == SystemHealth::Normal ? QStringLiteral("НОРМА") : QStringLiteral("НЕИСПРАВНОСТЬ");
}

QString lineLogText(LineState state)
{
    switch (state) {
    case LineState::Normal: return QStringLiteral("НОРМА");
    case LineState::Fault: return QStringLiteral("НЕИСПРАВНОСТЬ");
    case LineState::Disabled: return QStringLiteral("ОТКЛЮЧЕНА");
    case LineState::InsulationBreakdown: return QStringLiteral("ПРОБОЙ");
    }
    return QStringLiteral("НЕИЗВЕСТНО");
}

QString outputLogText(LineOutputState state)
{
    return state == LineOutputState::On ? QStringLiteral("ВКЛ") : QStringLiteral("ВЫКЛ");
}

QString linePrefix(const LineSnapshot &line)
{
    return QStringLiteral("Линия %1 \"%2\"").arg(line.index).arg(line.name);
}

QString faultReason(const CabinetSnapshot &snapshot)
{
    return snapshot.activeFaults.isEmpty() ? snapshot.explanation : snapshot.activeFaults.join(QStringLiteral(", "));
}

bool isOperationalLineState(LineState state)
{
    return state != LineState::Disabled;
}

const LineSnapshot *findLine(const QVector<LineSnapshot> &lines, int index)
{
    for (const LineSnapshot &line : lines) {
        if (line.index == index)
            return &line;
    }
    return nullptr;
}

} // namespace

QVector<StateLogEvent> StateChangeTracker::update(const CabinetSnapshot &snapshot)
{
    QVector<StateLogEvent> events;

    if (!m_initialized) {
        events.append({QStringLiteral("Режим: %1").arg(modeLogText(snapshot)), false});
        QString systemMessage = QStringLiteral("Система: %1").arg(systemLogText(snapshot.health));
        if (snapshot.health == SystemHealth::Fault)
            systemMessage += QStringLiteral(". Причина: %1").arg(faultReason(snapshot));
        events.append({systemMessage, snapshot.health == SystemHealth::Fault});

        for (const LineSnapshot &line : snapshot.lines) {
            if (line.state == LineState::Fault || line.state == LineState::InsulationBreakdown) {
                events.append({QStringLiteral("%1: исходное состояние %2")
                                   .arg(linePrefix(line), lineLogText(line.state)),
                               true});
            }
        }

        m_previous = snapshot;
        m_initialized = true;
        return events;
    }

    if (snapshot.mode != m_previous.mode
        || ((snapshot.mode == CabinetMode::ManualTest || snapshot.mode == CabinetMode::ScheduledTest)
            && (snapshot.testKind != m_previous.testKind || snapshot.testSource != m_previous.testSource))) {
        events.append({QStringLiteral("Режим: %1 → %2")
                           .arg(modeLogText(m_previous), modeLogText(snapshot)),
                       false});
    }

    if (snapshot.health != m_previous.health) {
        QString message = QStringLiteral("Система: %1 → %2")
                              .arg(systemLogText(m_previous.health), systemLogText(snapshot.health));
        if (snapshot.health == SystemHealth::Fault)
            message += QStringLiteral(". Причина: %1").arg(faultReason(snapshot));
        events.append({message, snapshot.health == SystemHealth::Fault});
    } else if (snapshot.health == SystemHealth::Fault
               && faultReason(snapshot) != faultReason(m_previous)) {
        events.append({QStringLiteral("Система: НЕИСПРАВНОСТЬ. Причина изменена: %1")
                           .arg(faultReason(snapshot)),
                       true});
    }

    for (const LineSnapshot &line : snapshot.lines) {
        const LineSnapshot *previous = findLine(m_previous.lines, line.index);
        if (!previous)
            continue;

        if (line.state != previous->state
            && isOperationalLineState(line.state)
            && isOperationalLineState(previous->state)) {
            const bool warning = line.state == LineState::Fault
                || line.state == LineState::InsulationBreakdown;
            events.append({QStringLiteral("%1: %2 → %3")
                               .arg(linePrefix(line), lineLogText(previous->state), lineLogText(line.state)),
                           warning});
        }

        if (line.outputState != previous->outputState) {
            events.append({QStringLiteral("%1: выход %2 → %3")
                               .arg(linePrefix(line), outputLogText(previous->outputState), outputLogText(line.outputState)),
                           false});
        }
    }

    m_previous = snapshot;
    return events;
}

} // namespace DialogG2
