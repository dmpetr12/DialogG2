#include "PanelFacade.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLocalSocket>

PanelFacade::PanelFacade(QObject *parent)
    : QObject(parent)
{
    connect(&m_pollTimer, &QTimer::timeout, this, &PanelFacade::pollState);
    m_pollTimer.start(1000);
    pollState();
}

bool PanelFacade::connected() const
{
    return m_connected;
}

bool PanelFacade::testRunning() const
{
    return state().value(QStringLiteral("testRunning")).toBool(false);
}

int PanelFacade::testPlannedSec() const
{
    return state().value(QStringLiteral("testPlannedSec")).toInt(0);
}

int PanelFacade::testRemainingSec() const
{
    return state().value(QStringLiteral("testRemainingSec")).toInt(0);
}

int PanelFacade::lineCount() const
{
    return state().value(QStringLiteral("lineCount")).toInt(lines().size());
}

QVariantList PanelFacade::lines() const
{
    return state().value(QStringLiteral("lines")).toArray().toVariantList();
}

QString PanelFacade::modeText() const
{
    return state().value(QStringLiteral("modeText")).toString(QStringLiteral("Норма"));
}

QString PanelFacade::healthText() const
{
    return state().value(QStringLiteral("healthText")).toString(QStringLiteral("Норма"));
}

QString PanelFacade::modeColor() const
{
    return state().value(QStringLiteral("modeColor")).toString(QStringLiteral("#d84236"));
}

bool PanelFacade::manualEmergencyActive() const
{
    return state().value(QStringLiteral("manualEmergencyActive")).toBool(false);
}

bool PanelFacade::systemOk() const
{
    return state().value(QStringLiteral("systemOk")).toBool(true);
}

bool PanelFacade::linesOk() const
{
    return state().value(QStringLiteral("linesOk")).toBool(true);
}

bool PanelFacade::batteryOk() const
{
    return state().value(QStringLiteral("batteryOk")).toBool(true);
}

int PanelFacade::batteryPercent() const
{
    return state().value(QStringLiteral("batteryPercent")).toInt(-1);
}

QVariantMap PanelFacade::battery() const
{
    return state().value(QStringLiteral("battery")).toObject().toVariantMap();
}

double PanelFacade::inputVoltage() const
{
    return state().value(QStringLiteral("inletU")).toDouble(0.0);
}

double PanelFacade::inputCurrent() const
{
    return state().value(QStringLiteral("inletI")).toDouble(0.0);
}

double PanelFacade::inputFrequency() const
{
    return state().value(QStringLiteral("inletF")).toDouble(0.0);
}

double PanelFacade::outputPower() const
{
    return state().value(QStringLiteral("inletP")).toDouble(0.0);
}

double PanelFacade::temperature() const
{
    return state().value(QStringLiteral("temperature")).toDouble(0.0);
}

void PanelFacade::refresh()
{
    pollState();
}

QVariantMap PanelFacade::lineAt(int index) const
{
    QJsonObject response;
    const bool ok = sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("lineAt")},
        {QStringLiteral("index"), index}
    }, &response);

    if (!ok)
        return {};

    return response.value(QStringLiteral("line")).toObject().toVariantMap();
}

bool PanelFacade::addLine()
{
    const bool ok = sendCommand({{QStringLiteral("cmd"), QStringLiteral("addLine")}});
    pollState();
    return ok;
}

bool PanelFacade::updateLine(int index, const QVariantMap &lineData)
{
    return sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("updateLine")},
        {QStringLiteral("index"), index},
        {QStringLiteral("lineData"), QJsonObject::fromVariantMap(lineData)}
    });
}

bool PanelFacade::saveLines()
{
    return sendCommand({{QStringLiteral("cmd"), QStringLiteral("saveLines")}});
}

bool PanelFacade::applyLineModes()
{
    return sendCommand({{QStringLiteral("cmd"), QStringLiteral("applyLineModes")}});
}

bool PanelFacade::stopCurrentTest()
{
    const bool ok = sendCommand({{QStringLiteral("cmd"), QStringLiteral("stopCurrentTest")}});
    pollState();
    return ok;
}

bool PanelFacade::startFunctionalTest(int warmupSec)
{
    const bool ok = sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("startFunctionalTest")},
        {QStringLiteral("warmupSec"), warmupSec}
    });
    pollState();
    return ok;
}

bool PanelFacade::startDurationTest(int durationSec)
{
    const bool ok = sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("startDurationTest")},
        {QStringLiteral("durationSec"), durationSec}
    });
    pollState();
    return ok;
}

bool PanelFacade::setLineSetupActive(int index, bool active)
{
    return sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("setLineSetupActive")},
        {QStringLiteral("index"), index},
        {QStringLiteral("active"), active}
    });
}

bool PanelFacade::startManualEmergency()
{
    const bool ok = sendCommand({{QStringLiteral("cmd"), QStringLiteral("startManualEmergency")}});
    pollState();
    return ok;
}

bool PanelFacade::stopManualEmergency()
{
    const bool ok = sendCommand({{QStringLiteral("cmd"), QStringLiteral("stopManualEmergency")}});
    pollState();
    return ok;
}

QVariantList PanelFacade::journal()
{
    QJsonObject response;
    if (!sendCommand({{QStringLiteral("cmd"), QStringLiteral("journal")}}, &response))
        return {};
    return response.value(QStringLiteral("entries")).toArray().toVariantList();
}

QVariantList PanelFacade::readLogs(int offset, int limit)
{
    QJsonObject response;
    if (!sendCommand({
            {QStringLiteral("cmd"), QStringLiteral("readLogs")},
            {QStringLiteral("offset"), offset},
            {QStringLiteral("limit"), limit}
        }, &response)) {
        return {};
    }
    return response.value(QStringLiteral("lines")).toArray().toVariantList();
}

QVariantList PanelFacade::getAllTests()
{
    QJsonObject response;
    if (!sendCommand({{QStringLiteral("cmd"), QStringLiteral("getAllTests")}}, &response))
        return {};
    return response.value(QStringLiteral("entries")).toArray().toVariantList();
}

bool PanelFacade::addTest(const QVariantMap &data)
{
    return sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("addTest")},
        {QStringLiteral("data"), QJsonObject::fromVariantMap(data)}
    });
}

bool PanelFacade::removeTest(int index)
{
    return sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("removeTest")},
        {QStringLiteral("index"), index}
    });
}

bool PanelFacade::updateTestProperty(int index, const QString &key, const QVariant &value)
{
    return sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("updateTestProperty")},
        {QStringLiteral("index"), index},
        {QStringLiteral("key"), key},
        {QStringLiteral("value"), QJsonValue::fromVariant(value)}
    });
}

bool PanelFacade::updateWeekDays(int index, const QStringList &days)
{
    QJsonArray array;
    for (const QString &day : days)
        array.append(day);

    return sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("updateWeekDays")},
        {QStringLiteral("index"), index},
        {QStringLiteral("days"), array}
    });
}

bool PanelFacade::checkPassword(const QString &password)
{
    QJsonObject response;
    const bool ok = sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("checkPassword")},
        {QStringLiteral("password"), password}
    }, &response);
    return ok && response.value(QStringLiteral("match")).toBool(false);
}

bool PanelFacade::changePassword(const QString &newPassword)
{
    return sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("changePassword")},
        {QStringLiteral("password"), newPassword}
    });
}

bool PanelFacade::setSystemTime(qint64 msec)
{
    return sendCommand({
        {QStringLiteral("cmd"), QStringLiteral("setSystemTime")},
        {QStringLiteral("time"), QString::number(msec)}
    });
}

bool PanelFacade::sendCommand(const QJsonObject &request, QJsonObject *response) const
{
    QLocalSocket socket;
    socket.connectToServer(m_serverName);
    if (!socket.waitForConnected(300)) {
        m_connected = false;
        return false;
    }

    socket.write(QJsonDocument(request).toJson(QJsonDocument::Compact));
    if (!socket.waitForBytesWritten(300)) {
        m_connected = false;
        return false;
    }

    if (!socket.waitForReadyRead(500)) {
        m_connected = false;
        return false;
    }

    const QByteArray raw = socket.readAll();
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        m_connected = false;
        return false;
    }

    const QJsonObject object = doc.object();
    if (response)
        *response = object;

    m_connected = object.value(QStringLiteral("ok")).toBool(false);
    return m_connected;
}

QJsonObject PanelFacade::state() const
{
    return m_state;
}

void PanelFacade::pollState()
{
    const bool wasConnected = m_connected;
    QJsonObject response;
    const bool ok = sendCommand({{QStringLiteral("cmd"), QStringLiteral("getState")}}, &response);
    if (!ok) {
        if (wasConnected)
            emit changed();
        return;
    }

    m_state = response.value(QStringLiteral("state")).toObject();
    emit changed();
}
