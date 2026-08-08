#include "TestJournalStore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <utility>

namespace DialogG2 {

TestJournalStore::TestJournalStore(QString filePath)
    : m_filePath(std::move(filePath))
{
}

const QString &TestJournalStore::filePath() const
{
    return m_filePath;
}

bool TestJournalStore::read(QVector<TestJournalEntry> *entries, QString *error) const
{
    if (entries)
        entries->clear();

    if (!QFile::exists(m_filePath))
        return true;

    QFile file(m_filePath);
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

    if (!entries)
        return true;

    const QJsonArray array = doc.object().value(QStringLiteral("entries")).toArray();
    entries->reserve(array.size());
    for (const QJsonValue &value : array)
        entries->append(testJournalEntryFromJson(value.toObject()));

    return true;
}

bool TestJournalStore::write(const QVector<TestJournalEntry> &entries, QString *error) const
{
    const QFileInfo info(m_filePath);
    if (!QDir().mkpath(info.absolutePath())) {
        if (error)
            *error = QStringLiteral("Cannot create test journal directory: %1").arg(info.absolutePath());
        return false;
    }

    QJsonArray array;
    for (const TestJournalEntry &entry : entries)
        array.append(toJson(entry));

    const QJsonObject root = {
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("entries"), array}
    };

    QSaveFile file(m_filePath);
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

bool TestJournalStore::append(const QVector<TestJournalEntry> &entries, QString *error) const
{
    if (entries.isEmpty())
        return true;

    QVector<TestJournalEntry> existing;
    if (!read(&existing, error))
        return false;

    existing += entries;
    return write(existing, error);
}

} // namespace DialogG2
