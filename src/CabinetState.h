#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

class CabinetState : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY changed)
    Q_PROPERTY(Health health READ health WRITE setHealth NOTIFY changed)
    Q_PROPERTY(QString modeText READ modeText NOTIFY changed)
    Q_PROPERTY(QString healthText READ healthText NOTIFY changed)
    Q_PROPERTY(QString modeColor READ modeColor NOTIFY changed)
    Q_PROPERTY(QString healthColor READ healthColor NOTIFY changed)
    Q_PROPERTY(bool voltageControlOk READ voltageControlOk WRITE setVoltageControlOk NOTIFY changed)
    Q_PROPERTY(bool batteryOk READ batteryOk WRITE setBatteryOk NOTIFY changed)
    Q_PROPERTY(int batteryPercent READ batteryPercent WRITE setBatteryPercent NOTIFY changed)
    Q_PROPERTY(double inputVoltage READ inputVoltage WRITE setInputVoltage NOTIFY changed)
    Q_PROPERTY(double outputPower READ outputPower WRITE setOutputPower NOTIFY changed)
    Q_PROPERTY(double leakageCurrent READ leakageCurrent WRITE setLeakageCurrent NOTIFY changed)
    Q_PROPERTY(int temperature READ temperature WRITE setTemperature NOTIFY changed)

public:
    enum Mode {
        NormalMode,
        Emergency,
        Fire,
        ManualTest,
        ScheduledTest
    };
    Q_ENUM(Mode)

    enum Health {
        Normal,
        Fault
    };
    Q_ENUM(Health)

    explicit CabinetState(QObject *parent = nullptr);

    void setStateFilePath(const QString &path);
    void startPolling(int intervalMs = 500);

    Mode mode() const;
    void setMode(Mode mode);

    Health health() const;
    void setHealth(Health health);

    QString modeText() const;
    QString healthText() const;
    QString modeColor() const;
    QString healthColor() const;

    bool voltageControlOk() const;
    void setVoltageControlOk(bool ok);

    bool batteryOk() const;
    void setBatteryOk(bool ok);

    int batteryPercent() const;
    void setBatteryPercent(int percent);

    double inputVoltage() const;
    void setInputVoltage(double voltage);

    double outputPower() const;
    void setOutputPower(double power);

    double leakageCurrent() const;
    void setLeakageCurrent(double current);

    int temperature() const;
    void setTemperature(int temperature);

signals:
    void changed();

private:
    void reload();
    void applyJson(const QByteArray &data);

    Mode m_mode = NormalMode;
    Health m_health = Normal;
    bool m_voltageControlOk = true;
    bool m_batteryOk = true;
    int m_batteryPercent = -1;
    double m_inputVoltage = 0.0;
    double m_outputPower = 0.0;
    double m_leakageCurrent = 0.0;
    int m_temperature = 0;
    QString m_stateFilePath;
    QTimer m_pollTimer;
};
