#pragma once

#include "CabinetSnapshot.h"

#include <QString>

namespace DialogG2 {

class TestJournalStore
{
public:
    explicit TestJournalStore(QString filePath);

    const QString &filePath() const;
    bool read(QVector<TestJournalEntry> *entries, QString *error = nullptr) const;
    bool write(const QVector<TestJournalEntry> &entries, QString *error = nullptr) const;
    bool append(const QVector<TestJournalEntry> &entries, QString *error = nullptr) const;

private:
    QString m_filePath;
};

} // namespace DialogG2
