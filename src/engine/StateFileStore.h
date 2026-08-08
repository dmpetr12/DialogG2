#pragma once

#include "CabinetSnapshot.h"

#include <QString>

namespace DialogG2 {

class StateFileStore
{
public:
    explicit StateFileStore(QString filePath);

    const QString &filePath() const;
    bool write(const CabinetSnapshot &snapshot, QString *error = nullptr) const;
    bool read(CabinetSnapshot *snapshot, QString *error = nullptr) const;

private:
    QString m_filePath;
};

} // namespace DialogG2
