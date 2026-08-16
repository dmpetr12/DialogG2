#pragma once

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include <limits>

namespace DialogG2 {

enum class CabinetMode
{
    Normal = 0,
    Emergency = 1,
    Fire = 2,
    ManualTest = 3,
    ScheduledTest = 4
};

enum class TestKind
{
    None = 0,
    Functional = 1,
    Duration = 2
};

enum class TestSource
{
    None = 0,
    Manual = 1,
    Scheduled = 2
};

enum class TestRunStatus
{
    None = 0,
    Running = 1,
    Passed = 2,
    Failed = 3,
    InterruptedByPriority = 4,
    StoppedByOperator = 5
};

enum class SystemHealth
{
    Normal = 0,
    Fault = 1
};

enum class BatteryState
{
    Normal = 0,
    Warning = 1,
    Fault = 2,
    Disconnected = 3
};

enum class LineState
{
    Normal = 0,
    Fault = 1,
    Disabled = 2,
    InsulationBreakdown = 3
};

enum class LineKind
{
    Constant = 0,
    NonConstant = 1
};

enum class LineOutputState
{
    Off = 0,
    On = 1
};

enum class LineOperationalState
{
    NotChecked = 0,
    WarmingUp = 1,
    Normal = 2,
    Fault = 3,
    NoMeasurement = 4
};

struct LineOperationalCheck
{
    LineOperationalState state = LineOperationalState::NotChecked;
    QDateTime startedAt;
    int warmupSeconds = 0;
    double measuredPower = std::numeric_limits<double>::quiet_NaN();
    double nominalPower = std::numeric_limits<double>::quiet_NaN();
    double tolerancePercent = std::numeric_limits<double>::quiet_NaN();
    QString details;
};

struct LineTestResult
{
    QDateTime completedAt;
    TestRunStatus status = TestRunStatus::None;
    double measuredPower = std::numeric_limits<double>::quiet_NaN();
    double nominalPower = std::numeric_limits<double>::quiet_NaN();
    double tolerancePercent = std::numeric_limits<double>::quiet_NaN();
    QString details;
};

struct LineSnapshot
{
    int index = 0;
    QString name;
    bool enabled = true;
    LineKind kind = LineKind::Constant;
    bool requestInputActive = false;
    LineOutputState outputState = LineOutputState::On;
    LineState state = LineState::Normal;
    double nominalPower = std::numeric_limits<double>::quiet_NaN();
    double powerTestTolerancePercent = std::numeric_limits<double>::quiet_NaN();
    double leakageCurrentLimit = 30.0;
    double outputVoltage = std::numeric_limits<double>::quiet_NaN();
    double outputCurrent = std::numeric_limits<double>::quiet_NaN();
    double outputPower = std::numeric_limits<double>::quiet_NaN();
    double leakageCurrent = std::numeric_limits<double>::quiet_NaN();
    LineOperationalCheck operationalCheck;
    LineTestResult lastFunctionalTest;
    LineTestResult lastDurationTest;
};

struct TestLineMeasurement
{
    int lineIndex = 0;
    QString lineName;
    double measuredPower = std::numeric_limits<double>::quiet_NaN();
    double nominalPower = std::numeric_limits<double>::quiet_NaN();
    double tolerancePercent = std::numeric_limits<double>::quiet_NaN();
    TestRunStatus status = TestRunStatus::None;
    QString details;
};

struct TestJournalEntry
{
    TestKind kind = TestKind::None;
    TestSource source = TestSource::None;
    QDateTime startedAt;
    QDateTime finishedAt;
    TestRunStatus status = TestRunStatus::None;
    QString reason;
    QVector<TestLineMeasurement> lines;
};

struct ActiveTestSnapshot
{
    bool active = false;
    TestKind kind = TestKind::None;
    TestSource source = TestSource::None;
    QDateTime startedAt;
    QDateTime dueAt;
    int warmupSeconds = 0;
    int durationSeconds = 0;
};

struct BatterySnapshot
{
    bool connected = true;
    bool communicationOk = true;
    BatteryState state = BatteryState::Normal;
    int socPercent = -1;
    double voltage = std::numeric_limits<double>::quiet_NaN();
    double current = std::numeric_limits<double>::quiet_NaN();
    double remainingCapacityAh = std::numeric_limits<double>::quiet_NaN();
    double nominalCapacityAh = std::numeric_limits<double>::quiet_NaN();
    double fullChargeCapacityAh = std::numeric_limits<double>::quiet_NaN();
    int cycleCount = -1;

    int cellCount = 0;
    QVector<double> cellVoltages;
    double minCellVoltage = std::numeric_limits<double>::quiet_NaN();
    double maxCellVoltage = std::numeric_limits<double>::quiet_NaN();
    double cellVoltageDelta = std::numeric_limits<double>::quiet_NaN();

    QVector<double> temperatures;
    double minTemperature = std::numeric_limits<double>::quiet_NaN();
    double maxTemperature = std::numeric_limits<double>::quiet_NaN();

    bool chargeAllowed = true;
    bool dischargeAllowed = true;
    bool balancingActive = false;
    bool heatingActive = false;

    quint16 protectionStatusRaw = 0;
    quint16 alarmStatusRaw = 0;
    QStringList faults;
    QStringList warnings;
};

struct MaintenanceLineStatus
{
    int lineIndex = 0;
    QString lineName;
    QDateTime lastTestAt;
    bool overdue = false;
};

struct MaintenanceSnapshot
{
    bool ok = true;
    int overdueLinesCount = 0;
    bool longTestOverdue = false;
    QDateTime lastLongTestAt;
    int lineLimitDays = 30;
    int longTestLimitDays = 365;
    QVector<MaintenanceLineStatus> lines;
    QString summary;
};

struct CabinetSnapshot
{
    int schemaVersion = 1;
    QDateTime timestamp = QDateTime::currentDateTimeUtc();

    CabinetMode mode = CabinetMode::Normal;
    SystemHealth health = SystemHealth::Normal;
    QString explanation = QStringLiteral("Штатный режим");

    bool voltageControlOk = true;
    bool fireInputActive = false;
    bool manualEmergencyActive = false;
    TestKind testKind = TestKind::None;
    TestSource testSource = TestSource::None;
    ActiveTestSnapshot activeTest;

    double inputVoltage = std::numeric_limits<double>::quiet_NaN();
    double inputCurrent = std::numeric_limits<double>::quiet_NaN();
    double inputPower = std::numeric_limits<double>::quiet_NaN();
    double inputFrequency = std::numeric_limits<double>::quiet_NaN();
    double temperature = std::numeric_limits<double>::quiet_NaN();

    BatterySnapshot battery;
    QVector<LineSnapshot> lines;
    QVector<TestJournalEntry> testJournal;
    MaintenanceSnapshot maintenance;
    QStringList activeFaults;
};

QString modeCode(CabinetMode mode);
QString modeText(CabinetMode mode);
QString testKindCode(TestKind kind);
QString testKindText(TestKind kind);
QString testSourceCode(TestSource source);
QString testSourceText(TestSource source);
QString testRunStatusCode(TestRunStatus status);
QString testRunStatusText(TestRunStatus status);
QString healthCode(SystemHealth health);
QString healthText(SystemHealth health);
QString batteryStateCode(BatteryState state);
QString batteryStateText(BatteryState state);
QString lineStateText(LineState state);
QString lineKindCode(LineKind kind);
QString lineKindText(LineKind kind);
QString lineOutputStateCode(LineOutputState state);
QString lineOutputStateText(LineOutputState state);
QString lineOperationalStateCode(LineOperationalState state);
QString lineOperationalStateText(LineOperationalState state);

QJsonObject toJson(const LineSnapshot &line);
QJsonObject toJson(const LineOperationalCheck &check);
QJsonObject toJson(const LineTestResult &result);
QJsonObject toJson(const TestLineMeasurement &measurement);
QJsonObject toJson(const TestJournalEntry &entry);
QJsonObject toJson(const ActiveTestSnapshot &test);
QJsonObject toJson(const BatterySnapshot &battery);
QJsonObject toJson(const MaintenanceLineStatus &line);
QJsonObject toJson(const MaintenanceSnapshot &maintenance);
QJsonObject toJson(const CabinetSnapshot &snapshot);

LineOperationalCheck lineOperationalCheckFromJson(const QJsonObject &obj);
LineTestResult lineTestResultFromJson(const QJsonObject &obj);
TestLineMeasurement testLineMeasurementFromJson(const QJsonObject &obj);
TestJournalEntry testJournalEntryFromJson(const QJsonObject &obj);
ActiveTestSnapshot activeTestFromJson(const QJsonObject &obj);
LineSnapshot lineFromJson(const QJsonObject &obj);
BatterySnapshot batteryFromJson(const QJsonObject &obj);
MaintenanceLineStatus maintenanceLineStatusFromJson(const QJsonObject &obj);
MaintenanceSnapshot maintenanceFromJson(const QJsonObject &obj);
CabinetSnapshot snapshotFromJson(const QJsonObject &obj);

} // namespace DialogG2
