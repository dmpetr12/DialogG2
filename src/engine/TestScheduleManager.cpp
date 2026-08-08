#include "TestScheduleManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>
#include <QVariant>

namespace DialogG2 {

static QJsonObject entryToJson(const TestScheduleEntry &entry)
{
    QJsonArray weekDays;
    for (const QString &day : entry.weekDays)
        weekDays.append(day);

    return {
        {QStringLiteral("enabled"), entry.enabled},
        {QStringLiteral("period"), entry.period},
        {QStringLiteral("startDate"), entry.startDate.toString(QStringLiteral("yyyy-MM-dd"))},
        {QStringLiteral("startTime"), entry.startTime.toString(QStringLiteral("HH:mm"))},
        {QStringLiteral("testType"), entry.testType},
        {QStringLiteral("weekDays"), weekDays}
    };
}

static TestScheduleEntry entryFromJson(const QJsonObject &obj)
{
    TestScheduleEntry entry;
    entry.enabled = obj.value(QStringLiteral("enabled")).toBool(true);
    entry.period = obj.value(QStringLiteral("period")).toString(QStringLiteral("один раз"));
    entry.startDate = QDate::fromString(obj.value(QStringLiteral("startDate")).toString(), QStringLiteral("yyyy-MM-dd"));
    entry.startTime = QTime::fromString(obj.value(QStringLiteral("startTime")).toString(), QStringLiteral("HH:mm"));
    entry.testType = obj.value(QStringLiteral("testType")).toString(QStringLiteral("functional"));

    const QJsonArray days = obj.value(QStringLiteral("weekDays")).toArray();
    for (const QJsonValue &day : days)
        entry.weekDays.append(day.toString());

    return entry;
}

bool TestScheduleManager::load(const QString &filePath, QString *error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error)
            *error = parseError.errorString();
        return false;
    }

    QJsonArray array;
    if (doc.isArray()) {
        array = doc.array();
    } else if (doc.isObject()) {
        array = doc.object().value(QStringLiteral("entries")).toArray();
    } else {
        if (error)
            *error = QStringLiteral("Schedule JSON must be array or object with entries");
        return false;
    }

    QVector<TestScheduleEntry> loaded;
    for (const QJsonValue &value : array) {
        const TestScheduleEntry entry = entryFromJson(value.toObject());
        QString validationError;
        if (validateEntry(entry, &validationError)) {
            loaded.append(entry);
        } else {
            if (error)
                *error = validationError;
        }
    }

    m_entries = loaded;
    m_lastPlannedTriggered = QVector<QDateTime>(m_entries.size());
    return true;
}

bool TestScheduleManager::save(const QString &filePath, QString *error) const
{
    const QFileInfo info(filePath);
    if (!QDir().mkpath(info.absolutePath())) {
        if (error)
            *error = QStringLiteral("Cannot create config directory: %1").arg(info.absolutePath());
        return false;
    }

    QJsonArray array;
    for (const TestScheduleEntry &entry : m_entries)
        array.append(entryToJson(entry));

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }

    return true;
}

bool TestScheduleManager::saveDefault(const QString &filePath, QString *error) const
{
    TestScheduleManager defaults;
    return defaults.save(filePath, error);
}

const QVector<TestScheduleEntry> &TestScheduleManager::entries() const
{
    return m_entries;
}

bool TestScheduleManager::addEntry(const TestScheduleEntry &entry, QString *error)
{
    if (!validateEntry(entry, error))
        return false;

    m_entries.append(entry);
    m_lastPlannedTriggered = QVector<QDateTime>(m_entries.size());
    return true;
}

bool TestScheduleManager::updateEntry(int index, const TestScheduleEntry &entry, QString *error)
{
    if (index < 0 || index >= m_entries.size()) {
        if (error)
            *error = QStringLiteral("Schedule entry index out of range");
        return false;
    }
    if (!validateEntry(entry, error))
        return false;

    m_entries[index] = entry;
    if (m_lastPlannedTriggered.size() != m_entries.size())
        m_lastPlannedTriggered = QVector<QDateTime>(m_entries.size());
    else
        m_lastPlannedTriggered[index] = {};
    return true;
}

bool TestScheduleManager::updateEntryProperty(int index, const QString &key, const QVariant &value, QString *error)
{
    if (index < 0 || index >= m_entries.size()) {
        if (error)
            *error = QStringLiteral("Schedule entry index out of range");
        return false;
    }

    TestScheduleEntry entry = m_entries.at(index);
    if (key == QStringLiteral("enabled"))
        entry.enabled = value.toBool();
    else if (key == QStringLiteral("period"))
        entry.period = value.toString();
    else if (key == QStringLiteral("startDate"))
        entry.startDate = QDate::fromString(value.toString(), QStringLiteral("yyyy-MM-dd"));
    else if (key == QStringLiteral("startTime"))
        entry.startTime = QTime::fromString(value.toString(), QStringLiteral("HH:mm"));
    else if (key == QStringLiteral("testType"))
        entry.testType = value.toString();
    else if (key == QStringLiteral("weekDays"))
        entry.weekDays = value.toStringList();
    else {
        if (error)
            *error = QStringLiteral("Unknown schedule property: %1").arg(key);
        return false;
    }

    return updateEntry(index, entry, error);
}

bool TestScheduleManager::removeEntry(int index, QString *error)
{
    if (index < 0 || index >= m_entries.size()) {
        if (error)
            *error = QStringLiteral("Schedule entry index out of range");
        return false;
    }

    m_entries.removeAt(index);
    m_lastPlannedTriggered = QVector<QDateTime>(m_entries.size());
    return true;
}

TestScheduleRequest TestScheduleManager::evaluate(const QDateTime &now)
{
    TestScheduleRequest request;
    if (!now.isValid())
        return request;

    constexpr qint64 windowSec = 90;
    constexpr qint64 dedupeSec = 180;
    if (m_lastPlannedTriggered.size() != m_entries.size())
        m_lastPlannedTriggered = QVector<QDateTime>(m_entries.size());

    const QDateTime localNow = now.toLocalTime();
    for (int i = 0; i < m_entries.size(); ++i) {
        const TestScheduleEntry &entry = m_entries.at(i);
        if (!entry.enabled)
            continue;

        const QDateTime planned = buildCandidatePlanned(entry, localNow);
        if (!planned.isValid())
            continue;

        const qint64 secsAfter = planned.secsTo(localNow);
        if (secsAfter < 0 || secsAfter > windowSec)
            continue;

        if (m_lastPlannedTriggered.at(i).isValid()) {
            if (m_lastPlannedTriggered.at(i) == planned)
                continue;
            if (m_lastPlannedTriggered.at(i).secsTo(localNow) < dedupeSec)
                continue;
        }

        m_lastPlannedTriggered[i] = planned;
        request.entryIndex = i;
        request.testType = entry.testType;
        request.period = entry.period;
        request.plannedAt = planned;

        const TestKind kind = kindFromTestType(entry.testType);
        if (kind == TestKind::Duration) {
            request.duration.active = true;
            request.duration.durationSeconds = 0;
        } else if (kind == TestKind::Functional) {
            request.functional.active = true;
        }
        return request;
    }

    return request;
}

QVector<TestScheduleEntry> TestScheduleManager::defaultEntries()
{
    return {};
}

TestKind TestScheduleManager::kindFromTestType(const QString &testType)
{
    const QString normalized = testType.trimmed().toLower();
    if (normalized == QStringLiteral("тест на время")
        || normalized == QStringLiteral("duration")
        || normalized == QStringLiteral("duration_test")
        || normalized == QStringLiteral("long")) {
        return TestKind::Duration;
    }
    if (normalized == QStringLiteral("функциональный тест")
        || normalized == QStringLiteral("functional")
        || normalized == QStringLiteral("functional_test")
        || normalized == QStringLiteral("short")) {
        return TestKind::Functional;
    }
    return TestKind::None;
}

bool TestScheduleManager::validateEntry(const TestScheduleEntry &entry, QString *error)
{
    if (kindFromTestType(entry.testType) == TestKind::None) {
        if (error)
            *error = QStringLiteral("Unknown schedule testType: %1").arg(entry.testType);
        return false;
    }
    if (!entry.startDate.isValid()) {
        if (error)
            *error = QStringLiteral("Schedule startDate is invalid");
        return false;
    }
    if (!entry.startTime.isValid()) {
        if (error)
            *error = QStringLiteral("Schedule startTime is invalid");
        return false;
    }
    if (entry.period.trimmed() == QStringLiteral("дни недели")) {
        for (const QString &day : entry.weekDays) {
            if (weekdayKeyToQt(day) == 0) {
                if (error)
                    *error = QStringLiteral("Unknown weekday key: %1").arg(day);
                return false;
            }
        }
    }
    return true;
}

int TestScheduleManager::weekdayKeyToQt(const QString &key)
{
    if (key == QStringLiteral("Mon")) return 1;
    if (key == QStringLiteral("Tue")) return 2;
    if (key == QStringLiteral("Wed")) return 3;
    if (key == QStringLiteral("Thu")) return 4;
    if (key == QStringLiteral("Fri")) return 5;
    if (key == QStringLiteral("Sat")) return 6;
    if (key == QStringLiteral("Sun")) return 7;
    return 0;
}

int TestScheduleManager::monthsForPeriod(const QString &period)
{
    if (period == QStringLiteral("раз в месяц")) return 1;
    if (period == QStringLiteral("раз в 3 месяца")) return 3;
    if (period == QStringLiteral("раз в полгода")) return 6;
    if (period == QStringLiteral("раз в год")) return 12;
    return 0;
}

QDateTime TestScheduleManager::buildCandidatePlanned(const TestScheduleEntry &entry, const QDateTime &now)
{
    if (!entry.startDate.isValid() || !entry.startTime.isValid())
        return {};

    const QString period = entry.period.trimmed().isEmpty()
        ? QStringLiteral("один раз")
        : entry.period.trimmed();

    if (period == QStringLiteral("один раз"))
        return QDateTime(entry.startDate, entry.startTime);

    if (period == QStringLiteral("ежедневно")) {
        QDate day = now.date();
        if (day < entry.startDate)
            day = entry.startDate;

        QDateTime planned(day, entry.startTime);
        if (planned > now) {
            planned = planned.addDays(-1);
            if (planned.date() < entry.startDate)
                return {};
        }
        return planned;
    }

    if (period == QStringLiteral("дни недели")) {
        QSet<int> allowed;
        for (const QString &key : entry.weekDays) {
            const int day = weekdayKeyToQt(key);
            if (day != 0)
                allowed.insert(day);
        }
        if (allowed.isEmpty())
            return {};

        QDate day = now.date();
        if (day < entry.startDate)
            day = entry.startDate;

        QDateTime planned(day, entry.startTime);
        if (planned > now)
            day = day.addDays(-1);

        for (int back = 0; back < 8; ++back) {
            if (day < entry.startDate)
                return {};

            const QDateTime candidate(day, entry.startTime);
            if (candidate <= now && allowed.contains(day.dayOfWeek()))
                return candidate;
            day = day.addDays(-1);
        }
        return {};
    }

    const int stepMonths = monthsForPeriod(period);
    if (stepMonths > 0) {
        const QDateTime base(entry.startDate, entry.startTime);
        if (!base.isValid() || now < base)
            return {};

        const int monthsDiff = (now.date().year() - base.date().year()) * 12
            + (now.date().month() - base.date().month());
        const int monthsToAdd = (monthsDiff / stepMonths) * stepMonths;

        QDateTime planned = base.addMonths(monthsToAdd);
        if (planned > now)
            planned = planned.addMonths(-stepMonths);
        if (planned < base)
            planned = base;
        return planned <= now ? planned : QDateTime();
    }

    return QDateTime(entry.startDate, entry.startTime);
}

QString TestScheduleManager::triggerKey(int index, const QDateTime &planned)
{
    return QStringLiteral("%1:%2").arg(index).arg(planned.toString(QStringLiteral("yyyyMMddHHmmss")));
}

} // namespace DialogG2
