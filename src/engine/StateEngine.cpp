#include "StateEngine.h"

namespace DialogG2 {

CabinetSnapshot StateEngine::evaluate(const EngineInputs &inputs) const
{
    CabinetSnapshot snapshot;
    snapshot.timestamp = QDateTime::currentDateTimeUtc();
    snapshot.mode = resolveMode(inputs);
    snapshot.activeFaults = collectFaults(inputs);
    snapshot.health = snapshot.activeFaults.isEmpty() ? SystemHealth::Normal : SystemHealth::Fault;
    snapshot.explanation = makeExplanation(snapshot.mode, snapshot.health, snapshot.activeFaults);

    snapshot.voltageControlOk = inputs.voltageControlOk;
    snapshot.fireInputActive = inputs.fireInputActive;
    snapshot.manualEmergencyActive = inputs.manualEmergencyActive;
    snapshot.testKind = inputs.testKind;
    snapshot.testSource = inputs.testSource;
    snapshot.activeTest = inputs.activeTest;
    snapshot.inputVoltage = inputs.inputVoltage;
    snapshot.inputCurrent = inputs.inputCurrent;
    snapshot.inputPower = inputs.inputPower;
    snapshot.inputFrequency = inputs.inputFrequency;
    snapshot.temperature = inputs.temperature;
    snapshot.battery = inputs.battery;
    snapshot.lines = inputs.lines;
    snapshot.testJournal = inputs.testJournal;
    snapshot.maintenance = inputs.maintenance;

    return snapshot;
}

CabinetMode StateEngine::resolveMode(const EngineInputs &inputs)
{
    if (!inputs.voltageControlOk)
        return CabinetMode::Emergency;

    if (inputs.fireInputActive || inputs.manualEmergencyActive)
        return CabinetMode::Fire;

    if (inputs.manualTestActive)
        return CabinetMode::ManualTest;

    if (inputs.scheduledTestActive)
        return CabinetMode::ScheduledTest;

    return CabinetMode::Normal;
}

QString StateEngine::makeExplanation(CabinetMode mode, SystemHealth health, const QStringList &faults)
{
    if (health == SystemHealth::Fault && !faults.isEmpty())
        return QStringLiteral("Есть неисправность: %1").arg(faults.join(QStringLiteral(", ")));

    switch (mode) {
    case CabinetMode::Normal:
        return QStringLiteral("Штатный режим");
    case CabinetMode::Emergency:
        return QStringLiteral("Авария по реле контроля напряжения");
    case CabinetMode::Fire:
        return QStringLiteral("Пожарный режим или ручная авария");
    case CabinetMode::ManualTest:
        return QStringLiteral("Идёт ручной тест шкафа");
    case CabinetMode::ScheduledTest:
        return QStringLiteral("Идёт тест шкафа по расписанию");
    }

    return QStringLiteral("Состояние неизвестно");
}

QStringList StateEngine::collectFaults(const EngineInputs &inputs)
{
    QStringList faults;

    if (inputs.relayFault)
        faults.append(QStringLiteral("реле"));
    if (inputs.modbusFault)
        faults.append(QStringLiteral("связь Modbus"));
    if (inputs.batteryFault
        || inputs.battery.state == BatteryState::Fault
        || inputs.battery.state == BatteryState::Disconnected
        || !inputs.battery.connected
        || !inputs.battery.communicationOk) {
        if (!inputs.battery.faults.isEmpty())
            faults.append(QStringLiteral("АКБ: %1").arg(inputs.battery.faults.join(QStringLiteral(", "))));
        else
            faults.append(QStringLiteral("АКБ"));
    }
    if (inputs.leakageFault)
        faults.append(QStringLiteral("ток утечки"));
    if (inputs.temperatureFault)
        faults.append(QStringLiteral("температура"));

    for (const LineSnapshot &line : inputs.lines) {
        if (!line.enabled)
            continue;

        if (line.state == LineState::Fault)
            faults.append(QStringLiteral("линия %1").arg(line.index));
        else if (line.state == LineState::InsulationBreakdown)
            faults.append(QStringLiteral("пробой изоляции линия %1").arg(line.index));
    }

    faults.removeDuplicates();
    return faults;
}

} // namespace DialogG2
