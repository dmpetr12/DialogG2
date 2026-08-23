#pragma once

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

class PanelFacade : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY changed)
    Q_PROPERTY(bool testRunning READ testRunning NOTIFY changed)
    Q_PROPERTY(int testPlannedSec READ testPlannedSec NOTIFY changed)
    Q_PROPERTY(int testRemainingSec READ testRemainingSec NOTIFY changed)
    Q_PROPERTY(int lineCount READ lineCount NOTIFY changed)
    Q_PROPERTY(QVariantList lines READ lines NOTIFY changed)
    Q_PROPERTY(QString modeText READ modeText NOTIFY changed)
    Q_PROPERTY(QString healthText READ healthText NOTIFY changed)
    Q_PROPERTY(QString modeColor READ modeColor NOTIFY changed)
    Q_PROPERTY(bool manualEmergencyActive READ manualEmergencyActive NOTIFY changed)
    Q_PROPERTY(bool systemOk READ systemOk NOTIFY changed)
    Q_PROPERTY(bool linesOk READ linesOk NOTIFY changed)
    Q_PROPERTY(bool batteryOk READ batteryOk NOTIFY changed)
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY changed)
    Q_PROPERTY(QVariantMap battery READ battery NOTIFY changed)
    Q_PROPERTY(QVariantMap maintenance READ maintenance NOTIFY changed)
    Q_PROPERTY(double inputVoltage READ inputVoltage NOTIFY changed)
    Q_PROPERTY(double inputCurrent READ inputCurrent NOTIFY changed)
    Q_PROPERTY(double inputFrequency READ inputFrequency NOTIFY changed)
    Q_PROPERTY(double outputPower READ outputPower NOTIFY changed)
    Q_PROPERTY(double temperature READ temperature NOTIFY changed)
    Q_PROPERTY(QString logLevel READ logLevel NOTIFY changed)

public:
    explicit PanelFacade(QObject *parent = nullptr);

    bool connected() const;
    bool testRunning() const;
    int testPlannedSec() const;
    int testRemainingSec() const;
    int lineCount() const;
    QVariantList lines() const;
    QString modeText() const;
    QString healthText() const;
    QString modeColor() const;
    bool manualEmergencyActive() const;
    bool systemOk() const;
    bool linesOk() const;
    bool batteryOk() const;
    int batteryPercent() const;
    QVariantMap battery() const;
    QVariantMap maintenance() const;
    double inputVoltage() const;
    double inputCurrent() const;
    double inputFrequency() const;
    double outputPower() const;
    double temperature() const;
    QString logLevel() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantMap lineAt(int index) const;
    Q_INVOKABLE bool addLine();
    Q_INVOKABLE bool removeLine(int index);
    Q_INVOKABLE bool updateLine(int index, const QVariantMap &lineData);
    Q_INVOKABLE bool saveLines();
    Q_INVOKABLE bool applyLineModes();
    Q_INVOKABLE bool stopCurrentTest();
    Q_INVOKABLE bool startFunctionalTest(int warmupSec);
    Q_INVOKABLE bool startDurationTest(int durationSec);
    Q_INVOKABLE bool setLineSetupActive(int index, bool active);
    Q_INVOKABLE bool startManualEmergency();
    Q_INVOKABLE bool stopManualEmergency();
    Q_INVOKABLE QVariantList journal();
    Q_INVOKABLE QVariantList readLogs(int offset = -200, int limit = 200);
    Q_INVOKABLE QString exportSystemLogToUsb();
    Q_INVOKABLE QString exportTestJournalToUsb();
    Q_INVOKABLE QVariantList getAllTests();
    Q_INVOKABLE bool addTest(const QVariantMap &data);
    Q_INVOKABLE bool removeTest(int index);
    Q_INVOKABLE bool updateTestProperty(int index, const QString &key, const QVariant &value);
    Q_INVOKABLE bool updateWeekDays(int index, const QStringList &days);
    Q_INVOKABLE bool checkPassword(const QString &password);
    Q_INVOKABLE bool changePassword(const QString &newPassword);
    Q_INVOKABLE bool setSystemTime(qint64 msec);

signals:
    void changed();

private:
    bool sendCommand(const QJsonObject &request, QJsonObject *response = nullptr) const;
    QJsonObject state() const;
    void pollState();

    QString m_serverName = QStringLiteral("emergency_panel_backend");
    QTimer m_pollTimer;
    mutable bool m_connected = false;
    QJsonObject m_state;
};
