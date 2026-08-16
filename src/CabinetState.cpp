#include "CabinetState.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>

CabinetState::CabinetState(QObject *parent)
    : QObject(parent)
{
    connect(&m_pollTimer, &QTimer::timeout, this, &CabinetState::reload);
}

CabinetState::Mode CabinetState::mode() const { return m_mode; }

void CabinetState::setMode(Mode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    emit changed();
}

CabinetState::Health CabinetState::health() const { return m_health; }

void CabinetState::setHealth(Health health)
{
    if (m_health == health)
        return;
    m_health = health;
    emit changed();
}

QString CabinetState::modeText() const
{
    switch (m_mode) {
    case NormalMode: return QStringLiteral("Норма");
    case Emergency: return QStringLiteral("Авария");
    case Fire: return QStringLiteral("Пожар");
    case ManualTest: return QStringLiteral("Тест ручной");
    case ScheduledTest: return QStringLiteral("Тест по расписанию");
    }
    return QStringLiteral("Неизвестно");
}

QString CabinetState::healthText() const
{
    return m_health == Normal ? QStringLiteral("Норма") : QStringLiteral("Неисправность");
}

QString CabinetState::modeColor() const
{
    switch (m_mode) {
    case NormalMode: return QStringLiteral("#0bbf63");
    case Emergency: return QStringLiteral("#d93636");
    case Fire: return QStringLiteral("#e06b21");
    case ManualTest:
    case ScheduledTest:
        return QStringLiteral("#d99a00");
    }
    return QStringLiteral("#808080");
}

QString CabinetState::healthColor() const
{
    return m_health == Normal ? QStringLiteral("#0bbf63") : QStringLiteral("#d93636");
}

bool CabinetState::voltageControlOk() const { return m_voltageControlOk; }

void CabinetState::setVoltageControlOk(bool ok)
{
    if (m_voltageControlOk == ok)
        return;
    m_voltageControlOk = ok;
    emit changed();
}

bool CabinetState::batteryOk() const { return m_batteryOk; }

void CabinetState::setBatteryOk(bool ok)
{
    if (m_batteryOk == ok)
        return;
    m_batteryOk = ok;
    emit changed();
}

int CabinetState::batteryPercent() const { return m_batteryPercent; }

void CabinetState::setBatteryPercent(int percent)
{
    if (percent >= 0)
        percent = std::clamp(percent, 0, 100);
    if (m_batteryPercent == percent)
        return;
    m_batteryPercent = percent;
    emit changed();
}

double CabinetState::inputVoltage() const { return m_inputVoltage; }

void CabinetState::setInputVoltage(double voltage)
{
    if (m_inputVoltage == voltage)
        return;
    m_inputVoltage = voltage;
    emit changed();
}

double CabinetState::outputPower() const { return m_outputPower; }

void CabinetState::setOutputPower(double power)
{
    if (m_outputPower == power)
        return;
    m_outputPower = power;
    emit changed();
}

double CabinetState::leakageCurrent() const { return m_leakageCurrent; }

void CabinetState::setLeakageCurrent(double current)
{
    if (m_leakageCurrent == current)
        return;
    m_leakageCurrent = current;
    emit changed();
}

int CabinetState::temperature() const { return m_temperature; }

void CabinetState::setTemperature(int temperature)
{
    if (m_temperature == temperature)
        return;
    m_temperature = temperature;
    emit changed();
}

void CabinetState::setStateFilePath(const QString &path)
{
    m_stateFilePath = path;
    reload();
}

void CabinetState::startPolling(int intervalMs)
{
    m_pollTimer.start(std::max(100, intervalMs));
}

void CabinetState::reload()
{
    if (m_stateFilePath.isEmpty())
        return;

    QFile file(m_stateFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    applyJson(file.readAll());
}

void CabinetState::applyJson(const QByteArray &data)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
        return;

    const QJsonObject root = doc.object();
    setMode(static_cast<Mode>(root.value(QStringLiteral("mode")).toInt()));
    setHealth(static_cast<Health>(root.value(QStringLiteral("health")).toInt()));
    setVoltageControlOk(root.value(QStringLiteral("voltageControlOk")).toBool(true));
    setInputVoltage(root.value(QStringLiteral("inputVoltage")).toDouble(0.0));
    setTemperature(static_cast<int>(std::round(root.value(QStringLiteral("temperature")).toDouble(0.0))));

    const QJsonObject battery = root.value(QStringLiteral("battery")).toObject();
    const int batteryState = battery.value(QStringLiteral("state")).toInt();
    setBatteryOk(battery.value(QStringLiteral("connected")).toBool(true)
                 && battery.value(QStringLiteral("communicationOk")).toBool(true)
                 && batteryState < 2);
    setBatteryPercent(battery.value(QStringLiteral("socPercent")).toInt(-1));

    double totalPower = 0.0;
    double maxLeakage = 0.0;
    const QJsonArray lines = root.value(QStringLiteral("lines")).toArray();
    for (const QJsonValue &value : lines) {
        const QJsonObject line = value.toObject();
        const double power = line.value(QStringLiteral("outputPower")).toDouble(0.0);
        const double leakage = line.value(QStringLiteral("leakageCurrent")).toDouble(0.0);
        if (std::isfinite(power))
            totalPower += power;
        if (std::isfinite(leakage))
            maxLeakage = std::max(maxLeakage, leakage);
    }
    setOutputPower(totalPower);
    setLeakageCurrent(maxLeakage);
}
