#include "CabinetSnapshot.h"

#include <QJsonDocument>

#include <cmath>
#include <limits>

namespace DialogG2 {

QString modeCode(CabinetMode mode)
{
    switch (mode) {
    case CabinetMode::Normal: return QStringLiteral("normal");
    case CabinetMode::Emergency: return QStringLiteral("emergency");
    case CabinetMode::Fire: return QStringLiteral("fire");
    case CabinetMode::ManualTest: return QStringLiteral("manual_test");
    case CabinetMode::ScheduledTest: return QStringLiteral("scheduled_test");
    }
    return QStringLiteral("unknown");
}

QString modeText(CabinetMode mode)
{
    switch (mode) {
    case CabinetMode::Normal: return QStringLiteral("Норма");
    case CabinetMode::Emergency: return QStringLiteral("Авария");
    case CabinetMode::Fire: return QStringLiteral("Пожар");
    case CabinetMode::ManualTest: return QStringLiteral("Тест ручной");
    case CabinetMode::ScheduledTest: return QStringLiteral("Тест по расписанию");
    }
    return QStringLiteral("Неизвестно");
}

QString testKindCode(TestKind kind)
{
    switch (kind) {
    case TestKind::None: return QStringLiteral("none");
    case TestKind::Functional: return QStringLiteral("functional");
    case TestKind::Duration: return QStringLiteral("duration");
    }
    return QStringLiteral("unknown");
}

QString testKindText(TestKind kind)
{
    switch (kind) {
    case TestKind::None: return QStringLiteral("");
    case TestKind::Functional: return QStringLiteral("Исправность");
    case TestKind::Duration: return QStringLiteral("Длительность работы");
    }
    return QStringLiteral("Неизвестно");
}

QString testSourceCode(TestSource source)
{
    switch (source) {
    case TestSource::None: return QStringLiteral("none");
    case TestSource::Manual: return QStringLiteral("manual");
    case TestSource::Scheduled: return QStringLiteral("scheduled");
    }
    return QStringLiteral("unknown");
}

QString testSourceText(TestSource source)
{
    switch (source) {
    case TestSource::None: return QStringLiteral("");
    case TestSource::Manual: return QStringLiteral("Ручной");
    case TestSource::Scheduled: return QStringLiteral("По расписанию");
    }
    return QStringLiteral("Неизвестно");
}

QString testRunStatusCode(TestRunStatus status)
{
    switch (status) {
    case TestRunStatus::None: return QStringLiteral("none");
    case TestRunStatus::Running: return QStringLiteral("running");
    case TestRunStatus::Passed: return QStringLiteral("passed");
    case TestRunStatus::Failed: return QStringLiteral("failed");
    case TestRunStatus::InterruptedByPriority: return QStringLiteral("interrupted_by_priority");
    case TestRunStatus::StoppedByOperator: return QStringLiteral("stopped_by_operator");
    }
    return QStringLiteral("unknown");
}

QString testRunStatusText(TestRunStatus status)
{
    switch (status) {
    case TestRunStatus::None: return QStringLiteral("");
    case TestRunStatus::Running: return QStringLiteral("Выполняется");
    case TestRunStatus::Passed: return QStringLiteral("Исправно");
    case TestRunStatus::Failed: return QStringLiteral("Неисправно");
    case TestRunStatus::InterruptedByPriority: return QStringLiteral("Прерван по приоритету");
    case TestRunStatus::StoppedByOperator: return QStringLiteral("Остановлен оператором");
    }
    return QStringLiteral("Неизвестно");
}

QString healthCode(SystemHealth health)
{
    return health == SystemHealth::Normal ? QStringLiteral("normal") : QStringLiteral("fault");
}

QString healthText(SystemHealth health)
{
    return health == SystemHealth::Normal ? QStringLiteral("Норма") : QStringLiteral("Неисправность");
}

QString batteryStateCode(BatteryState state)
{
    switch (state) {
    case BatteryState::Normal: return QStringLiteral("normal");
    case BatteryState::Warning: return QStringLiteral("warning");
    case BatteryState::Fault: return QStringLiteral("fault");
    case BatteryState::Disconnected: return QStringLiteral("disconnected");
    }
    return QStringLiteral("unknown");
}

QString batteryStateText(BatteryState state)
{
    switch (state) {
    case BatteryState::Normal: return QStringLiteral("Норма");
    case BatteryState::Warning: return QStringLiteral("Предупреждение");
    case BatteryState::Fault: return QStringLiteral("Неисправность");
    case BatteryState::Disconnected: return QStringLiteral("Нет связи");
    }
    return QStringLiteral("Неизвестно");
}

QString lineStateText(LineState state)
{
    switch (state) {
    case LineState::Normal: return QStringLiteral("Норма");
    case LineState::Fault: return QStringLiteral("Неисправность");
    case LineState::Disabled: return QStringLiteral("Отключена");
    case LineState::InsulationBreakdown: return QStringLiteral("Пробой изоляции");
    }
    return QStringLiteral("Неизвестно");
}

QString lineKindCode(LineKind kind)
{
    switch (kind) {
    case LineKind::Constant: return QStringLiteral("constant");
    case LineKind::NonConstant: return QStringLiteral("non_constant");
    }
    return QStringLiteral("unknown");
}

QString lineKindText(LineKind kind)
{
    switch (kind) {
    case LineKind::Constant: return QStringLiteral("Постоянная");
    case LineKind::NonConstant: return QStringLiteral("Непостоянная");
    }
    return QStringLiteral("Неизвестно");
}

QString lineOutputStateCode(LineOutputState state)
{
    switch (state) {
    case LineOutputState::Off: return QStringLiteral("off");
    case LineOutputState::On: return QStringLiteral("on");
    }
    return QStringLiteral("off");
}

QString lineOutputStateText(LineOutputState state)
{
    switch (state) {
    case LineOutputState::Off: return QStringLiteral("Выключена");
    case LineOutputState::On: return QStringLiteral("Включена");
    }
    return QStringLiteral("Выключена");
}

QString lineOperationalStateCode(LineOperationalState state)
{
    switch (state) {
    case LineOperationalState::NotChecked: return QStringLiteral("not_checked");
    case LineOperationalState::WarmingUp: return QStringLiteral("warming_up");
    case LineOperationalState::Normal: return QStringLiteral("normal");
    case LineOperationalState::Fault: return QStringLiteral("fault");
    case LineOperationalState::NoMeasurement: return QStringLiteral("no_measurement");
    }
    return QStringLiteral("unknown");
}

QString lineOperationalStateText(LineOperationalState state)
{
    switch (state) {
    case LineOperationalState::NotChecked: return QStringLiteral("Не контролируется");
    case LineOperationalState::WarmingUp: return QStringLiteral("Прогрев");
    case LineOperationalState::Normal: return QStringLiteral("Норма");
    case LineOperationalState::Fault: return QStringLiteral("Неисправность");
    case LineOperationalState::NoMeasurement: return QStringLiteral("Нет измерения");
    }
    return QStringLiteral("Неизвестно");
}

static QJsonValue numberOrNull(double value)
{
    return std::isfinite(value) ? QJsonValue(value) : QJsonValue(QJsonValue::Null);
}

static QJsonValue intOrNull(int value)
{
    return value >= 0 ? QJsonValue(value) : QJsonValue(QJsonValue::Null);
}

static double doubleOrNaN(const QJsonObject &obj, const QString &key)
{
    const QJsonValue value = obj.value(key);
    return value.isDouble() ? value.toDouble() : std::numeric_limits<double>::quiet_NaN();
}

static int intOrUnknown(const QJsonObject &obj, const QString &key)
{
    const QJsonValue value = obj.value(key);
    return value.isDouble() ? value.toInt() : -1;
}

static QJsonArray numbersToJson(const QVector<double> &values)
{
    QJsonArray array;
    for (double value : values)
        array.append(numberOrNull(value));
    return array;
}

static QVector<double> numbersFromJson(const QJsonArray &array)
{
    QVector<double> values;
    values.reserve(array.size());
    for (const QJsonValue &value : array)
        values.append(value.isDouble() ? value.toDouble() : std::numeric_limits<double>::quiet_NaN());
    return values;
}

static QJsonArray stringsToJson(const QStringList &values)
{
    QJsonArray array;
    for (const QString &value : values)
        array.append(value);
    return array;
}

static QStringList stringsFromJson(const QJsonArray &array)
{
    QStringList values;
    for (const QJsonValue &value : array)
        values.append(value.toString());
    return values;
}

static QString dateTimeToJson(const QDateTime &value)
{
    return value.isValid() ? value.toUTC().toString(Qt::ISODateWithMs) : QString();
}

static QJsonValue dateTimeOrNull(const QDateTime &value)
{
    return value.isValid() ? QJsonValue(dateTimeToJson(value)) : QJsonValue(QJsonValue::Null);
}

static QDateTime dateTimeFromJson(const QJsonObject &obj, const QString &key)
{
    const QJsonValue value = obj.value(key);
    return value.isString() ? QDateTime::fromString(value.toString(), Qt::ISODateWithMs) : QDateTime();
}

QJsonObject toJson(const LineTestResult &result)
{
    return {
        {QStringLiteral("completedAt"), dateTimeOrNull(result.completedAt)},
        {QStringLiteral("status"), static_cast<int>(result.status)},
        {QStringLiteral("statusCode"), testRunStatusCode(result.status)},
        {QStringLiteral("statusText"), testRunStatusText(result.status)},
        {QStringLiteral("measuredPower"), numberOrNull(result.measuredPower)},
        {QStringLiteral("nominalPower"), numberOrNull(result.nominalPower)},
        {QStringLiteral("tolerancePercent"), numberOrNull(result.tolerancePercent)},
        {QStringLiteral("details"), result.details}
    };
}

QJsonObject toJson(const LineOperationalCheck &check)
{
    return {
        {QStringLiteral("state"), static_cast<int>(check.state)},
        {QStringLiteral("stateCode"), lineOperationalStateCode(check.state)},
        {QStringLiteral("stateText"), lineOperationalStateText(check.state)},
        {QStringLiteral("startedAt"), dateTimeOrNull(check.startedAt)},
        {QStringLiteral("warmupSeconds"), check.warmupSeconds},
        {QStringLiteral("measuredPower"), numberOrNull(check.measuredPower)},
        {QStringLiteral("nominalPower"), numberOrNull(check.nominalPower)},
        {QStringLiteral("tolerancePercent"), numberOrNull(check.tolerancePercent)},
        {QStringLiteral("details"), check.details}
    };
}

QJsonObject toJson(const TestLineMeasurement &measurement)
{
    return {
        {QStringLiteral("lineIndex"), measurement.lineIndex},
        {QStringLiteral("lineName"), measurement.lineName},
        {QStringLiteral("measuredPower"), numberOrNull(measurement.measuredPower)},
        {QStringLiteral("nominalPower"), numberOrNull(measurement.nominalPower)},
        {QStringLiteral("tolerancePercent"), numberOrNull(measurement.tolerancePercent)},
        {QStringLiteral("status"), static_cast<int>(measurement.status)},
        {QStringLiteral("statusCode"), testRunStatusCode(measurement.status)},
        {QStringLiteral("statusText"), testRunStatusText(measurement.status)},
        {QStringLiteral("details"), measurement.details}
    };
}

QJsonObject toJson(const TestJournalEntry &entry)
{
    QJsonArray lines;
    for (const TestLineMeasurement &line : entry.lines)
        lines.append(toJson(line));

    return {
        {QStringLiteral("kind"), static_cast<int>(entry.kind)},
        {QStringLiteral("kindCode"), testKindCode(entry.kind)},
        {QStringLiteral("kindText"), testKindText(entry.kind)},
        {QStringLiteral("source"), static_cast<int>(entry.source)},
        {QStringLiteral("sourceCode"), testSourceCode(entry.source)},
        {QStringLiteral("sourceText"), testSourceText(entry.source)},
        {QStringLiteral("startedAt"), dateTimeOrNull(entry.startedAt)},
        {QStringLiteral("finishedAt"), dateTimeOrNull(entry.finishedAt)},
        {QStringLiteral("status"), static_cast<int>(entry.status)},
        {QStringLiteral("statusCode"), testRunStatusCode(entry.status)},
        {QStringLiteral("statusText"), testRunStatusText(entry.status)},
        {QStringLiteral("reason"), entry.reason},
        {QStringLiteral("lines"), lines}
    };
}

QJsonObject toJson(const ActiveTestSnapshot &test)
{
    return {
        {QStringLiteral("active"), test.active},
        {QStringLiteral("kind"), static_cast<int>(test.kind)},
        {QStringLiteral("kindCode"), testKindCode(test.kind)},
        {QStringLiteral("kindText"), testKindText(test.kind)},
        {QStringLiteral("source"), static_cast<int>(test.source)},
        {QStringLiteral("sourceCode"), testSourceCode(test.source)},
        {QStringLiteral("sourceText"), testSourceText(test.source)},
        {QStringLiteral("startedAt"), dateTimeOrNull(test.startedAt)},
        {QStringLiteral("dueAt"), dateTimeOrNull(test.dueAt)},
        {QStringLiteral("warmupSeconds"), test.warmupSeconds},
        {QStringLiteral("durationSeconds"), test.durationSeconds}
    };
}

QJsonObject toJson(const LineSnapshot &line)
{
    return {
        {QStringLiteral("index"), line.index},
        {QStringLiteral("name"), line.name.isEmpty()
             ? QStringLiteral("Линия %1").arg(line.index)
             : line.name},
        {QStringLiteral("enabled"), line.enabled},
        {QStringLiteral("kind"), static_cast<int>(line.kind)},
        {QStringLiteral("kindCode"), lineKindCode(line.kind)},
        {QStringLiteral("kindText"), lineKindText(line.kind)},
        {QStringLiteral("requestInputActive"), line.requestInputActive},
        {QStringLiteral("outputState"), static_cast<int>(line.outputState)},
        {QStringLiteral("outputStateCode"), lineOutputStateCode(line.outputState)},
        {QStringLiteral("outputStateText"), lineOutputStateText(line.outputState)},
        {QStringLiteral("state"), static_cast<int>(line.state)},
        {QStringLiteral("stateText"), lineStateText(line.state)},
        {QStringLiteral("nominalPower"), numberOrNull(line.nominalPower)},
        {QStringLiteral("powerTestTolerancePercent"), numberOrNull(line.powerTestTolerancePercent)},
        {QStringLiteral("leakageCurrentLimit"), numberOrNull(line.leakageCurrentLimit)},
        {QStringLiteral("outputVoltage"), numberOrNull(line.outputVoltage)},
        {QStringLiteral("outputCurrent"), numberOrNull(line.outputCurrent)},
        {QStringLiteral("outputPower"), numberOrNull(line.outputPower)},
        {QStringLiteral("leakageCurrent"), numberOrNull(line.leakageCurrent)},
        {QStringLiteral("operationalCheck"), toJson(line.operationalCheck)},
        {QStringLiteral("lastFunctionalTest"), toJson(line.lastFunctionalTest)},
        {QStringLiteral("lastDurationTest"), toJson(line.lastDurationTest)}
    };
}

QJsonObject toJson(const BatterySnapshot &battery)
{
    return {
        {QStringLiteral("connected"), battery.connected},
        {QStringLiteral("communicationOk"), battery.communicationOk},
        {QStringLiteral("state"), static_cast<int>(battery.state)},
        {QStringLiteral("stateCode"), batteryStateCode(battery.state)},
        {QStringLiteral("stateText"), batteryStateText(battery.state)},
        {QStringLiteral("socPercent"), intOrNull(battery.socPercent)},
        {QStringLiteral("voltage"), numberOrNull(battery.voltage)},
        {QStringLiteral("current"), numberOrNull(battery.current)},
        {QStringLiteral("remainingCapacityAh"), numberOrNull(battery.remainingCapacityAh)},
        {QStringLiteral("nominalCapacityAh"), numberOrNull(battery.nominalCapacityAh)},
        {QStringLiteral("fullChargeCapacityAh"), numberOrNull(battery.fullChargeCapacityAh)},
        {QStringLiteral("cycleCount"), intOrNull(battery.cycleCount)},
        {QStringLiteral("cellCount"), battery.cellCount},
        {QStringLiteral("cellVoltages"), numbersToJson(battery.cellVoltages)},
        {QStringLiteral("minCellVoltage"), numberOrNull(battery.minCellVoltage)},
        {QStringLiteral("maxCellVoltage"), numberOrNull(battery.maxCellVoltage)},
        {QStringLiteral("cellVoltageDelta"), numberOrNull(battery.cellVoltageDelta)},
        {QStringLiteral("temperatures"), numbersToJson(battery.temperatures)},
        {QStringLiteral("minTemperature"), numberOrNull(battery.minTemperature)},
        {QStringLiteral("maxTemperature"), numberOrNull(battery.maxTemperature)},
        {QStringLiteral("chargeAllowed"), battery.chargeAllowed},
        {QStringLiteral("dischargeAllowed"), battery.dischargeAllowed},
        {QStringLiteral("balancingActive"), battery.balancingActive},
        {QStringLiteral("heatingActive"), battery.heatingActive},
        {QStringLiteral("protectionStatusRaw"), static_cast<int>(battery.protectionStatusRaw)},
        {QStringLiteral("alarmStatusRaw"), static_cast<int>(battery.alarmStatusRaw)},
        {QStringLiteral("faults"), stringsToJson(battery.faults)},
        {QStringLiteral("warnings"), stringsToJson(battery.warnings)}
    };
}

QJsonObject toJson(const CabinetSnapshot &snapshot)
{
    QJsonArray lines;
    for (const LineSnapshot &line : snapshot.lines)
        lines.append(toJson(line));

    QJsonArray faults;
    for (const QString &fault : snapshot.activeFaults)
        faults.append(fault);

    QJsonArray testJournal;
    for (const TestJournalEntry &entry : snapshot.testJournal)
        testJournal.append(toJson(entry));

    return {
        {QStringLiteral("schemaVersion"), snapshot.schemaVersion},
        {QStringLiteral("timestamp"), snapshot.timestamp.toString(Qt::ISODateWithMs)},
        {QStringLiteral("mode"), static_cast<int>(snapshot.mode)},
        {QStringLiteral("modeCode"), modeCode(snapshot.mode)},
        {QStringLiteral("modeText"), modeText(snapshot.mode)},
        {QStringLiteral("health"), static_cast<int>(snapshot.health)},
        {QStringLiteral("healthCode"), healthCode(snapshot.health)},
        {QStringLiteral("healthText"), healthText(snapshot.health)},
        {QStringLiteral("explanation"), snapshot.explanation},
        {QStringLiteral("voltageControlOk"), snapshot.voltageControlOk},
        {QStringLiteral("fireInputActive"), snapshot.fireInputActive},
        {QStringLiteral("manualEmergencyActive"), snapshot.manualEmergencyActive},
        {QStringLiteral("testActive"), snapshot.testKind != TestKind::None},
        {QStringLiteral("testKind"), static_cast<int>(snapshot.testKind)},
        {QStringLiteral("testKindCode"), testKindCode(snapshot.testKind)},
        {QStringLiteral("testKindText"), testKindText(snapshot.testKind)},
        {QStringLiteral("testSource"), static_cast<int>(snapshot.testSource)},
        {QStringLiteral("testSourceCode"), testSourceCode(snapshot.testSource)},
        {QStringLiteral("testSourceText"), testSourceText(snapshot.testSource)},
        {QStringLiteral("activeTest"), toJson(snapshot.activeTest)},
        {QStringLiteral("inputVoltage"), numberOrNull(snapshot.inputVoltage)},
        {QStringLiteral("inputCurrent"), numberOrNull(snapshot.inputCurrent)},
        {QStringLiteral("inputPower"), numberOrNull(snapshot.inputPower)},
        {QStringLiteral("inputFrequency"), numberOrNull(snapshot.inputFrequency)},
        {QStringLiteral("temperature"), numberOrNull(snapshot.temperature)},
        {QStringLiteral("battery"), toJson(snapshot.battery)},
        {QStringLiteral("lines"), lines},
        {QStringLiteral("testJournal"), testJournal},
        {QStringLiteral("activeFaults"), faults}
    };
}

LineTestResult lineTestResultFromJson(const QJsonObject &obj)
{
    LineTestResult result;
    result.completedAt = dateTimeFromJson(obj, QStringLiteral("completedAt"));
    result.status = static_cast<TestRunStatus>(obj.value(QStringLiteral("status")).toInt());
    result.measuredPower = doubleOrNaN(obj, QStringLiteral("measuredPower"));
    result.nominalPower = doubleOrNaN(obj, QStringLiteral("nominalPower"));
    result.tolerancePercent = doubleOrNaN(obj, QStringLiteral("tolerancePercent"));
    result.details = obj.value(QStringLiteral("details")).toString();
    return result;
}

LineOperationalCheck lineOperationalCheckFromJson(const QJsonObject &obj)
{
    LineOperationalCheck check;
    check.state = static_cast<LineOperationalState>(obj.value(QStringLiteral("state")).toInt());
    check.startedAt = dateTimeFromJson(obj, QStringLiteral("startedAt"));
    check.warmupSeconds = obj.value(QStringLiteral("warmupSeconds")).toInt();
    check.measuredPower = doubleOrNaN(obj, QStringLiteral("measuredPower"));
    check.nominalPower = doubleOrNaN(obj, QStringLiteral("nominalPower"));
    check.tolerancePercent = doubleOrNaN(obj, QStringLiteral("tolerancePercent"));
    check.details = obj.value(QStringLiteral("details")).toString();
    return check;
}

TestLineMeasurement testLineMeasurementFromJson(const QJsonObject &obj)
{
    TestLineMeasurement measurement;
    measurement.lineIndex = obj.value(QStringLiteral("lineIndex")).toInt();
    measurement.lineName = obj.value(QStringLiteral("lineName")).toString();
    measurement.measuredPower = doubleOrNaN(obj, QStringLiteral("measuredPower"));
    measurement.nominalPower = doubleOrNaN(obj, QStringLiteral("nominalPower"));
    measurement.tolerancePercent = doubleOrNaN(obj, QStringLiteral("tolerancePercent"));
    measurement.status = static_cast<TestRunStatus>(obj.value(QStringLiteral("status")).toInt());
    measurement.details = obj.value(QStringLiteral("details")).toString();
    return measurement;
}

TestJournalEntry testJournalEntryFromJson(const QJsonObject &obj)
{
    TestJournalEntry entry;
    entry.kind = static_cast<TestKind>(obj.value(QStringLiteral("kind")).toInt());
    entry.source = static_cast<TestSource>(obj.value(QStringLiteral("source")).toInt());
    entry.startedAt = dateTimeFromJson(obj, QStringLiteral("startedAt"));
    entry.finishedAt = dateTimeFromJson(obj, QStringLiteral("finishedAt"));
    entry.status = static_cast<TestRunStatus>(obj.value(QStringLiteral("status")).toInt());
    entry.reason = obj.value(QStringLiteral("reason")).toString();

    const QJsonArray lines = obj.value(QStringLiteral("lines")).toArray();
    for (const QJsonValue &value : lines)
        entry.lines.append(testLineMeasurementFromJson(value.toObject()));

    return entry;
}

ActiveTestSnapshot activeTestFromJson(const QJsonObject &obj)
{
    ActiveTestSnapshot test;
    test.active = obj.value(QStringLiteral("active")).toBool(false);
    test.kind = static_cast<TestKind>(obj.value(QStringLiteral("kind")).toInt());
    test.source = static_cast<TestSource>(obj.value(QStringLiteral("source")).toInt());
    test.startedAt = dateTimeFromJson(obj, QStringLiteral("startedAt"));
    test.dueAt = dateTimeFromJson(obj, QStringLiteral("dueAt"));
    test.warmupSeconds = obj.value(QStringLiteral("warmupSeconds")).toInt();
    test.durationSeconds = obj.value(QStringLiteral("durationSeconds")).toInt();
    return test;
}

LineSnapshot lineFromJson(const QJsonObject &obj)
{
    LineSnapshot line;
    line.index = obj.value(QStringLiteral("index")).toInt();
    line.name = obj.value(QStringLiteral("name")).toString(QStringLiteral("Линия %1").arg(line.index));
    line.enabled = obj.value(QStringLiteral("enabled")).toBool(true);
    line.kind = static_cast<LineKind>(obj.value(QStringLiteral("kind")).toInt());
    line.requestInputActive = obj.value(QStringLiteral("requestInputActive")).toBool(false);
    line.outputState = static_cast<LineOutputState>(obj.value(QStringLiteral("outputState")).toInt());
    line.state = static_cast<LineState>(obj.value(QStringLiteral("state")).toInt());
    line.nominalPower = doubleOrNaN(obj, QStringLiteral("nominalPower"));
    line.powerTestTolerancePercent = doubleOrNaN(obj, QStringLiteral("powerTestTolerancePercent"));
    line.leakageCurrentLimit = obj.value(QStringLiteral("leakageCurrentLimit")).isDouble()
        ? obj.value(QStringLiteral("leakageCurrentLimit")).toDouble()
        : 30.0;
    line.outputVoltage = doubleOrNaN(obj, QStringLiteral("outputVoltage"));
    line.outputCurrent = doubleOrNaN(obj, QStringLiteral("outputCurrent"));
    line.outputPower = doubleOrNaN(obj, QStringLiteral("outputPower"));
    line.leakageCurrent = doubleOrNaN(obj, QStringLiteral("leakageCurrent"));
    line.operationalCheck = lineOperationalCheckFromJson(obj.value(QStringLiteral("operationalCheck")).toObject());
    line.lastFunctionalTest = lineTestResultFromJson(obj.value(QStringLiteral("lastFunctionalTest")).toObject());
    line.lastDurationTest = lineTestResultFromJson(obj.value(QStringLiteral("lastDurationTest")).toObject());
    return line;
}

BatterySnapshot batteryFromJson(const QJsonObject &obj)
{
    BatterySnapshot battery;
    battery.connected = obj.value(QStringLiteral("connected")).toBool(true);
    battery.communicationOk = obj.value(QStringLiteral("communicationOk")).toBool(true);
    battery.state = static_cast<BatteryState>(obj.value(QStringLiteral("state")).toInt());
    battery.socPercent = intOrUnknown(obj, QStringLiteral("socPercent"));
    battery.voltage = doubleOrNaN(obj, QStringLiteral("voltage"));
    battery.current = doubleOrNaN(obj, QStringLiteral("current"));
    battery.remainingCapacityAh = doubleOrNaN(obj, QStringLiteral("remainingCapacityAh"));
    battery.nominalCapacityAh = doubleOrNaN(obj, QStringLiteral("nominalCapacityAh"));
    battery.fullChargeCapacityAh = doubleOrNaN(obj, QStringLiteral("fullChargeCapacityAh"));
    battery.cycleCount = intOrUnknown(obj, QStringLiteral("cycleCount"));
    battery.cellCount = obj.value(QStringLiteral("cellCount")).toInt();
    battery.cellVoltages = numbersFromJson(obj.value(QStringLiteral("cellVoltages")).toArray());
    battery.minCellVoltage = doubleOrNaN(obj, QStringLiteral("minCellVoltage"));
    battery.maxCellVoltage = doubleOrNaN(obj, QStringLiteral("maxCellVoltage"));
    battery.cellVoltageDelta = doubleOrNaN(obj, QStringLiteral("cellVoltageDelta"));
    battery.temperatures = numbersFromJson(obj.value(QStringLiteral("temperatures")).toArray());
    battery.minTemperature = doubleOrNaN(obj, QStringLiteral("minTemperature"));
    battery.maxTemperature = doubleOrNaN(obj, QStringLiteral("maxTemperature"));
    battery.chargeAllowed = obj.value(QStringLiteral("chargeAllowed")).toBool(true);
    battery.dischargeAllowed = obj.value(QStringLiteral("dischargeAllowed")).toBool(true);
    battery.balancingActive = obj.value(QStringLiteral("balancingActive")).toBool(false);
    battery.heatingActive = obj.value(QStringLiteral("heatingActive")).toBool(false);
    battery.protectionStatusRaw = static_cast<quint16>(obj.value(QStringLiteral("protectionStatusRaw")).toInt());
    battery.alarmStatusRaw = static_cast<quint16>(obj.value(QStringLiteral("alarmStatusRaw")).toInt());
    battery.faults = stringsFromJson(obj.value(QStringLiteral("faults")).toArray());
    battery.warnings = stringsFromJson(obj.value(QStringLiteral("warnings")).toArray());
    return battery;
}

CabinetSnapshot snapshotFromJson(const QJsonObject &obj)
{
    CabinetSnapshot snapshot;
    snapshot.schemaVersion = obj.value(QStringLiteral("schemaVersion")).toInt(1);
    snapshot.timestamp = QDateTime::fromString(obj.value(QStringLiteral("timestamp")).toString(), Qt::ISODateWithMs);
    snapshot.mode = static_cast<CabinetMode>(obj.value(QStringLiteral("mode")).toInt());
    snapshot.health = static_cast<SystemHealth>(obj.value(QStringLiteral("health")).toInt());
    snapshot.explanation = obj.value(QStringLiteral("explanation")).toString();
    snapshot.voltageControlOk = obj.value(QStringLiteral("voltageControlOk")).toBool(true);
    snapshot.fireInputActive = obj.value(QStringLiteral("fireInputActive")).toBool(false);
    snapshot.manualEmergencyActive = obj.value(QStringLiteral("manualEmergencyActive")).toBool(false);
    snapshot.testKind = static_cast<TestKind>(obj.value(QStringLiteral("testKind")).toInt());
    snapshot.testSource = static_cast<TestSource>(obj.value(QStringLiteral("testSource")).toInt());
    snapshot.activeTest = activeTestFromJson(obj.value(QStringLiteral("activeTest")).toObject());
    snapshot.inputVoltage = doubleOrNaN(obj, QStringLiteral("inputVoltage"));
    snapshot.inputCurrent = doubleOrNaN(obj, QStringLiteral("inputCurrent"));
    snapshot.inputPower = doubleOrNaN(obj, QStringLiteral("inputPower"));
    snapshot.inputFrequency = doubleOrNaN(obj, QStringLiteral("inputFrequency"));
    snapshot.temperature = doubleOrNaN(obj, QStringLiteral("temperature"));
    snapshot.battery = batteryFromJson(obj.value(QStringLiteral("battery")).toObject());

    const QJsonArray lineArray = obj.value(QStringLiteral("lines")).toArray();
    for (const QJsonValue &value : lineArray)
        snapshot.lines.append(lineFromJson(value.toObject()));

    const QJsonArray testJournal = obj.value(QStringLiteral("testJournal")).toArray();
    for (const QJsonValue &value : testJournal)
        snapshot.testJournal.append(testJournalEntryFromJson(value.toObject()));

    const QJsonArray faults = obj.value(QStringLiteral("activeFaults")).toArray();
    for (const QJsonValue &value : faults)
        snapshot.activeFaults.append(value.toString());

    return snapshot;
}

} // namespace DialogG2
