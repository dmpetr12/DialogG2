#pragma once

#include "TestController.h"

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QVector>

namespace DialogG2 {

struct TestScheduleEntry
{
    bool enabled = true;
    QString period = QStringLiteral("один раз");
    QDate startDate = QDate::currentDate();
    QTime startTime = QTime(10, 0);
    QString testType = QStringLiteral("functional");
    QStringList weekDays;
};

struct TestScheduleRequest
{
    TestRequest functional;
    TestRequest duration;
    int entryIndex = -1;
    QString testType;
    QString period;
    QDateTime plannedAt;
};

class TestScheduleManager
{
public:
    bool load(const QString &filePath, QString *error = nullptr);
    bool save(const QString &filePath, QString *error = nullptr) const;
    bool saveDefault(const QString &filePath, QString *error = nullptr) const;

    const QVector<TestScheduleEntry> &entries() const;
    bool addEntry(const TestScheduleEntry &entry, QString *error = nullptr);
    bool updateEntry(int index, const TestScheduleEntry &entry, QString *error = nullptr);
    bool updateEntryProperty(int index, const QString &key, const QVariant &value, QString *error = nullptr);
    bool removeEntry(int index, QString *error = nullptr);

    TestScheduleRequest evaluate(const QDateTime &now);

    static QVector<TestScheduleEntry> defaultEntries();
    static TestKind kindFromTestType(const QString &testType);

private:
    static bool validateEntry(const TestScheduleEntry &entry, QString *error);
    static int weekdayKeyToQt(const QString &key);
    static int monthsForPeriod(const QString &period);
    static QDateTime buildCandidatePlanned(const TestScheduleEntry &entry, const QDateTime &now);
    static QString triggerKey(int index, const QDateTime &planned);

    QVector<TestScheduleEntry> m_entries = defaultEntries();
    QVector<QDateTime> m_lastPlannedTriggered;
};

} // namespace DialogG2
