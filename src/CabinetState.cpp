#include "CabinetState.h"

#include <algorithm>

CabinetState::CabinetState(QObject *parent)
    : QObject(parent)
{
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
