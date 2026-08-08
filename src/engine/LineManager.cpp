#include "LineManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <cmath>

namespace DialogG2 {

static constexpr double DefaultPowerTestTolerancePercent = 5.0;

static QString lineKindToConfig(LineKind kind)
{
    return kind == LineKind::NonConstant ? QStringLiteral("non_constant") : QStringLiteral("constant");
}

static LineKind lineKindFromConfig(const QString &value)
{
    return value == QStringLiteral("non_constant") ? LineKind::NonConstant : LineKind::Constant;
}

static QJsonObject pointToJson(const WaveSharePoint &point)
{
    if (!point.isValid())
        return {};

    return {
        {QStringLiteral("module"), point.module},
        {QStringLiteral("channel"), point.channel}
    };
}

static WaveSharePoint pointFromJson(const QJsonObject &obj)
{
    return {
        obj.value(QStringLiteral("module")).toInt(),
        obj.value(QStringLiteral("channel")).toInt()
    };
}

static QJsonObject lineToJson(const LineConfig &line)
{
    return {
        {QStringLiteral("index"), line.index},
        {QStringLiteral("name"), line.name},
        {QStringLiteral("enabled"), line.enabled},
        {QStringLiteral("kind"), lineKindToConfig(line.kind)},
        {QStringLiteral("nominalPower"), std::isfinite(line.nominalPower) ? QJsonValue(line.nominalPower) : QJsonValue(QJsonValue::Null)},
        {QStringLiteral("powerTestTolerancePercent"), std::isfinite(line.powerTestTolerancePercent) ? QJsonValue(line.powerTestTolerancePercent) : QJsonValue(QJsonValue::Null)},
        {QStringLiteral("leakageCurrentLimit"), line.leakageCurrentLimit},
        {QStringLiteral("requestInput"), pointToJson(line.requestInput)},
        {QStringLiteral("outputRelay"), pointToJson(line.outputRelay)},
        {QStringLiteral("lastFunctionalTest"), toJson(line.lastFunctionalTest)},
        {QStringLiteral("lastDurationTest"), toJson(line.lastDurationTest)}
    };
}

static LineConfig lineConfigFromJson(const QJsonObject &obj)
{
    LineConfig line;
    line.index = obj.value(QStringLiteral("index")).toInt();
    line.name = obj.value(QStringLiteral("name")).toString(QStringLiteral("Линия %1").arg(line.index));
    line.enabled = obj.value(QStringLiteral("enabled")).toBool(true);
    line.kind = lineKindFromConfig(obj.value(QStringLiteral("kind")).toString());
    line.nominalPower = obj.value(QStringLiteral("nominalPower")).isDouble()
        ? obj.value(QStringLiteral("nominalPower")).toDouble()
        : std::numeric_limits<double>::quiet_NaN();
    line.powerTestTolerancePercent = obj.value(QStringLiteral("powerTestTolerancePercent")).isDouble()
        ? obj.value(QStringLiteral("powerTestTolerancePercent")).toDouble()
        : DefaultPowerTestTolerancePercent;
    line.leakageCurrentLimit = obj.value(QStringLiteral("leakageCurrentLimit")).toDouble(30.0);
    line.requestInput = pointFromJson(obj.value(QStringLiteral("requestInput")).toObject());
    line.outputRelay = pointFromJson(obj.value(QStringLiteral("outputRelay")).toObject());
    line.lastFunctionalTest = lineTestResultFromJson(obj.value(QStringLiteral("lastFunctionalTest")).toObject());
    line.lastDurationTest = lineTestResultFromJson(obj.value(QStringLiteral("lastDurationTest")).toObject());
    return line;
}

static QJsonObject ioMapToJson(const CabinetIoMap &map)
{
    return {
        {QStringLiteral("fireInput"), pointToJson(map.fireInput)},
        {QStringLiteral("manualFireButton"), pointToJson(map.manualFireButton)},
        {QStringLiteral("manualStopButton"), pointToJson(map.manualStopButton)},
        {QStringLiteral("voltageControlInput"), pointToJson(map.voltageControlInput)},
        {QStringLiteral("modeRelay"), pointToJson(map.modeRelay)},
        {QStringLiteral("faultLampRelay"), pointToJson(map.faultLampRelay)},
        {QStringLiteral("testLampRelay"), pointToJson(map.testLampRelay)},
        {QStringLiteral("reserveRelay"), pointToJson(map.reserveRelay)}
    };
}

static CabinetIoMap ioMapFromJson(const QJsonObject &obj)
{
    CabinetIoMap map;
    map.fireInput = pointFromJson(obj.value(QStringLiteral("fireInput")).toObject());
    map.manualFireButton = pointFromJson(obj.value(QStringLiteral("manualFireButton")).toObject());
    if (!map.manualFireButton.isValid())
        map.manualFireButton = pointFromJson(obj.value(QStringLiteral("reserveInput1")).toObject());
    map.manualStopButton = pointFromJson(obj.value(QStringLiteral("manualStopButton")).toObject());
    if (!map.manualStopButton.isValid())
        map.manualStopButton = pointFromJson(obj.value(QStringLiteral("reserveInput2")).toObject());
    map.voltageControlInput = pointFromJson(obj.value(QStringLiteral("voltageControlInput")).toObject());
    map.modeRelay = pointFromJson(obj.value(QStringLiteral("modeRelay")).toObject());
    if (!map.modeRelay.isValid())
        map.modeRelay = pointFromJson(obj.value(QStringLiteral("fireModeRelay")).toObject());
    if (!map.modeRelay.isValid())
        map.modeRelay = pointFromJson(obj.value(QStringLiteral("emergencyModeRelay")).toObject());
    map.faultLampRelay = pointFromJson(obj.value(QStringLiteral("faultLampRelay")).toObject());
    map.testLampRelay = pointFromJson(obj.value(QStringLiteral("testLampRelay")).toObject());
    map.reserveRelay = pointFromJson(obj.value(QStringLiteral("reserveRelay")).toObject());
    return map;
}

bool LineManager::loadConfig(const QString &filePath, QString *error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error)
            *error = parseError.errorString();
        return false;
    }

    const QJsonObject root = doc.object();
    m_ioMap = ioMapFromJson(root.value(QStringLiteral("ioMap")).toObject());

    QVector<LineConfig> loadedLines;
    const QJsonArray array = root.value(QStringLiteral("lines")).toArray();
    loadedLines.reserve(array.size());
    for (const QJsonValue &value : array) {
        const LineConfig line = lineConfigFromJson(value.toObject());
        if (line.index > 0)
            loadedLines.append(line);
    }

    if (!loadedLines.isEmpty())
        m_lines = loadedLines;

    return true;
}

bool LineManager::saveConfig(const QString &filePath, QString *error) const
{
    const QFileInfo info(filePath);
    if (!QDir().mkpath(info.absolutePath())) {
        if (error)
            *error = QStringLiteral("Cannot create config directory: %1").arg(info.absolutePath());
        return false;
    }

    QJsonArray lines;
    for (const LineConfig &line : m_lines)
        lines.append(lineToJson(line));

    const QJsonObject root = {
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("ioMap"), ioMapToJson(m_ioMap)},
        {QStringLiteral("lines"), lines}
    };

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }

    return true;
}

bool LineManager::saveDefaultConfig(const QString &filePath, QString *error) const
{
    LineManager defaults;
    return defaults.saveConfig(filePath, error);
}

const QVector<LineConfig> &LineManager::lines() const
{
    return m_lines;
}

const CabinetIoMap &LineManager::ioMap() const
{
    return m_ioMap;
}

const LineConfig *LineManager::line(int index) const
{
    const int pos = findLineIndex(index);
    return pos >= 0 ? &m_lines[pos] : nullptr;
}

bool LineManager::addLine(LineConfig line, QString *error)
{
    if (m_lines.size() >= 25) {
        if (error)
            *error = QStringLiteral("Maximum line count is 25");
        return false;
    }

    if (line.index <= 0)
        line.index = nextLineIndex();

    if (findLineIndex(line.index) >= 0) {
        if (error)
            *error = QStringLiteral("Line %1 already exists").arg(line.index);
        return false;
    }

    if (line.name.isEmpty())
        line.name = QStringLiteral("Линия %1").arg(line.index);

    if (!line.requestInput.isValid())
        line.requestInput = defaultLinePoint(line.index);

    if (!line.outputRelay.isValid())
        line.outputRelay = line.requestInput;

    if (pointIsUsed(line.requestInput) || pointIsUsed(line.outputRelay)) {
        if (error)
            *error = QStringLiteral("WaveShare point is already used");
        return false;
    }

    m_lines.append(line);
    std::sort(m_lines.begin(), m_lines.end(), [](const LineConfig &a, const LineConfig &b) {
        return a.index < b.index;
    });
    return true;
}

bool LineManager::removeLine(int index, QString *error)
{
    const int pos = findLineIndex(index);
    if (pos < 0) {
        if (error)
            *error = QStringLiteral("Line %1 not found").arg(index);
        return false;
    }

    m_lines.removeAt(pos);
    return true;
}

bool LineManager::updateLine(const LineConfig &line, QString *error)
{
    if (line.index <= 0) {
        if (error)
            *error = QStringLiteral("Line index is invalid");
        return false;
    }

    const int pos = findLineIndex(line.index);
    if (pos < 0) {
        if (error)
            *error = QStringLiteral("Line %1 not found").arg(line.index);
        return false;
    }

    LineConfig updated = line;
    updated.name = updated.name.trimmed();
    if (updated.name.isEmpty())
        updated.name = QStringLiteral("Линия %1").arg(updated.index);

    if (!updated.requestInput.isValid())
        updated.requestInput = defaultLinePoint(updated.index);

    if (!updated.outputRelay.isValid())
        updated.outputRelay = updated.requestInput;

    if ((std::isfinite(updated.nominalPower) && updated.nominalPower < 0.0)
        || (std::isfinite(updated.powerTestTolerancePercent) && updated.powerTestTolerancePercent < 0.0)
        || !std::isfinite(updated.leakageCurrentLimit)
        || updated.leakageCurrentLimit <= 0.0) {
        if (error)
            *error = QStringLiteral("Line limits are invalid");
        return false;
    }

    for (int i = 0; i < m_lines.size(); ++i) {
        if (i == pos)
            continue;

        const LineConfig &other = m_lines.at(i);
        if (other.requestInput.module == updated.requestInput.module
            && other.requestInput.channel == updated.requestInput.channel) {
            if (error)
                *error = QStringLiteral("WaveShare input point is already used");
            return false;
        }
        if (other.outputRelay.module == updated.outputRelay.module
            && other.outputRelay.channel == updated.outputRelay.channel) {
            if (error)
                *error = QStringLiteral("WaveShare relay point is already used");
            return false;
        }
    }

    m_lines[pos] = updated;
    return true;
}

void LineManager::applyTestResults(const QVector<TestJournalEntry> &entries)
{
    for (const TestJournalEntry &entry : entries) {
        if (entry.status != TestRunStatus::Passed && entry.status != TestRunStatus::Failed)
            continue;

        for (const TestLineMeasurement &measurement : entry.lines) {
            const int pos = findLineIndex(measurement.lineIndex);
            if (pos < 0)
                continue;

            LineTestResult result;
            result.completedAt = entry.finishedAt;
            result.status = measurement.status;
            result.measuredPower = measurement.measuredPower;
            result.nominalPower = measurement.nominalPower;
            result.tolerancePercent = measurement.tolerancePercent;
            result.details = measurement.details;

            if (entry.kind == TestKind::Functional)
                m_lines[pos].lastFunctionalTest = result;
            else if (entry.kind == TestKind::Duration)
                m_lines[pos].lastDurationTest = result;
        }
    }
}

LineConfig LineManager::makeNextLine(LineKind kind) const
{
    LineConfig line;
    line.index = nextLineIndex();
    line.name = QStringLiteral("Линия %1").arg(line.index);
    line.kind = kind;
    line.nominalPower = 100.0;
    line.powerTestTolerancePercent = DefaultPowerTestTolerancePercent;
    line.leakageCurrentLimit = 30.0;
    line.requestInput = nextDefaultLinePoint();
    line.outputRelay = line.requestInput;
    return line;
}

WaveSharePoint LineManager::nextDefaultLinePoint() const
{
    for (int index = 1; index <= 25; ++index) {
        const WaveSharePoint point = defaultLinePoint(index);
        if (!pointIsUsed(point))
            return point;
    }
    return {};
}

int LineManager::nextLineIndex() const
{
    for (int index = 1; index <= 25; ++index) {
        if (findLineIndex(index) < 0)
            return index;
    }
    return 0;
}

LineManagerResult LineManager::evaluate(const LineManagerInputs &inputs) const
{
    LineManagerResult result;
    result.fireInputActive = inputActive(inputs.modules, m_ioMap.fireInput);
    result.voltageControlOk = inputActive(inputs.modules, m_ioMap.voltageControlInput);
    result.manualFireButtonActive = inputActive(inputs.modules, m_ioMap.manualFireButton);
    result.manualStopButtonActive = inputActive(inputs.modules, m_ioMap.manualStopButton);

    setRelayBit(&result.relayOutputBytes, m_ioMap.modeRelay, inputs.modeRelayOn);
    setRelayBit(&result.relayOutputBytes, m_ioMap.faultLampRelay, !inputs.faultLampOn);
    setRelayBit(&result.relayOutputBytes, m_ioMap.testLampRelay, inputs.testLampOn);
    setRelayBit(&result.relayOutputBytes, m_ioMap.reserveRelay, false);

    for (const WaveShareModuleState &module : inputs.modules) {
        if (!module.online)
            result.faults.append(QStringLiteral("нет связи WaveShare %1").arg(module.module));
    }

    for (const LineConfig &config : m_lines) {
        LineSnapshot line;
        line.index = config.index;
        line.name = config.name;
        line.enabled = config.enabled;
        line.kind = config.kind;
        line.nominalPower = config.nominalPower;
        line.powerTestTolerancePercent = config.powerTestTolerancePercent;
        line.leakageCurrentLimit = config.leakageCurrentLimit;
        line.lastFunctionalTest = config.lastFunctionalTest;
        line.lastDurationTest = config.lastDurationTest;
        line.state = config.enabled ? LineState::Normal : LineState::Disabled;
        line.requestInputActive = config.kind == LineKind::NonConstant
            && inputActive(inputs.modules, config.requestInput);

        bool shouldBeOn = false;
        if (config.enabled && config.kind == LineKind::Constant)
            shouldBeOn = true;
        else if (config.enabled && config.kind == LineKind::NonConstant)
            shouldBeOn = line.requestInputActive;

        line.outputState = shouldBeOn ? LineOutputState::On : LineOutputState::Off;
        setRelayBit(&result.relayOutputBytes, config.outputRelay, shouldBeOn);
        result.lines.append(line);
    }

    result.faults.removeDuplicates();
    return result;
}

QVector<LineConfig> LineManager::defaultLines()
{
    QVector<LineConfig> lines;
    lines.reserve(3);

    for (int i = 1; i <= 3; ++i) {
        LineConfig line;
        line.index = i;
        line.name = QStringLiteral("Линия %1").arg(i);
        line.kind = LineKind::Constant;
        line.nominalPower = 100.0;
        line.powerTestTolerancePercent = DefaultPowerTestTolerancePercent;
        line.leakageCurrentLimit = 30.0;
        line.requestInput = defaultLinePoint(i);
        line.outputRelay = defaultLinePoint(i);

        lines.append(line);
    }

    return lines;
}

WaveSharePoint LineManager::defaultLinePoint(int lineIndex)
{
    if (lineIndex < 1 || lineIndex > 25)
        return {};

    if (lineIndex <= 4)
        return {1, 4 + lineIndex};

    const int offset = lineIndex - 5;
    return {2 + offset / 8, 1 + offset % 8};
}

CabinetIoMap LineManager::defaultIoMap()
{
    CabinetIoMap map;
    map.fireInput = {1, 1};
    map.manualFireButton = {1, 2};
    map.manualStopButton = {1, 3};
    map.voltageControlInput = {1, 4};

    map.modeRelay = {1, 1};
    map.faultLampRelay = {1, 2};
    map.testLampRelay = {1, 3};
    map.reserveRelay = {1, 4};
    return map;
}

bool LineManager::inputActive(const QHash<int, WaveShareModuleState> &modules, const WaveSharePoint &point)
{
    if (!point.isValid())
        return false;

    const auto it = modules.constFind(point.module);
    if (it == modules.cend() || !it->online)
        return false;

    const quint8 mask = static_cast<quint8>(1u << (point.channel - 1));
    return (it->inputs & mask) != 0;
}

void LineManager::setRelayBit(QHash<int, quint8> *outputBytes, const WaveSharePoint &point, bool on)
{
    if (!outputBytes || !point.isValid())
        return;

    const quint8 mask = static_cast<quint8>(1u << (point.channel - 1));
    quint8 value = outputBytes->value(point.module, 0);

    if (on)
        value = static_cast<quint8>(value | mask);
    else
        value = static_cast<quint8>(value & ~mask);

    outputBytes->insert(point.module, value);
}

int LineManager::findLineIndex(int index) const
{
    for (int i = 0; i < m_lines.size(); ++i) {
        if (m_lines.at(i).index == index)
            return i;
    }
    return -1;
}

bool LineManager::pointIsUsed(const WaveSharePoint &point) const
{
    if (!point.isValid())
        return false;

    for (const LineConfig &line : m_lines) {
        if ((line.requestInput.module == point.module && line.requestInput.channel == point.channel)
            || (line.outputRelay.module == point.module && line.outputRelay.channel == point.channel)) {
            return true;
        }
    }

    return false;
}

} // namespace DialogG2
