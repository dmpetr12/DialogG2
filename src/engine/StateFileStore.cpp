#include "StateFileStore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>

#include <utility>

namespace DialogG2 {

StateFileStore::StateFileStore(QString filePath)
    : m_filePath(std::move(filePath))
{
}

const QString &StateFileStore::filePath() const
{
    return m_filePath;
}

bool StateFileStore::write(const CabinetSnapshot &snapshot, QString *error) const
{
    const QFileInfo info(m_filePath);
    if (!QDir().mkpath(info.absolutePath())) {
        if (error)
            *error = QStringLiteral("Cannot create state directory: %1").arg(info.absolutePath());
        return false;
    }

    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    const QJsonDocument doc(toJson(snapshot));
    file.write(doc.toJson(QJsonDocument::Indented));

    if (!file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }

    return true;
}

bool StateFileStore::read(CabinetSnapshot *snapshot, QString *error) const
{
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

    if (snapshot)
        *snapshot = snapshotFromJson(doc.object());

    return true;
}

} // namespace DialogG2
