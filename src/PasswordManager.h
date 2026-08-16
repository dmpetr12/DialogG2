#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class PasswordManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)

public:
    explicit PasswordManager(QObject *parent = nullptr)
        : QObject(parent)
        , m_settings(QStringLiteral("LightTech"), QStringLiteral("DialogG2Panel"))
    {
    }

    QString password() const
    {
        return m_settings.value(QStringLiteral("password"), QStringLiteral("1234")).toString();
    }

    void setPassword(const QString &password)
    {
        if (password == this->password())
            return;

        m_settings.setValue(QStringLiteral("password"), password);
        emit passwordChanged();
    }

signals:
    void passwordChanged();

private:
    QSettings m_settings;
};
