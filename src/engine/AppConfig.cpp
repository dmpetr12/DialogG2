#include "AppConfig.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace DialogG2 {

static QJsonObject modbusRtuToJson(const ModbusRtuConfig &config)
{
    return {
        {QStringLiteral("port"), config.port},
        {QStringLiteral("baudRate"), config.baudRate},
        {QStringLiteral("parity"), config.parity},
        {QStringLiteral("dataBits"), config.dataBits},
        {QStringLiteral("stopBits"), config.stopBits},
        {QStringLiteral("timeoutMs"), config.timeoutMs},
        {QStringLiteral("retries"), config.retries},
        {QStringLiteral("busOfflineFailureThreshold"), config.busOfflineFailureThreshold}
    };
}

static void loadModbusRtu(const QJsonObject &obj, ModbusRtuConfig *config)
{
    if (!config)
        return;

    if (obj.value(QStringLiteral("port")).isString())
        config->port = obj.value(QStringLiteral("port")).toString(config->port);
    if (obj.value(QStringLiteral("baudRate")).isDouble())
        config->baudRate = obj.value(QStringLiteral("baudRate")).toInt(config->baudRate);
    if (obj.value(QStringLiteral("parity")).isString())
        config->parity = obj.value(QStringLiteral("parity")).toString(config->parity);
    if (obj.value(QStringLiteral("dataBits")).isDouble())
        config->dataBits = obj.value(QStringLiteral("dataBits")).toInt(config->dataBits);
    if (obj.value(QStringLiteral("stopBits")).isDouble())
        config->stopBits = obj.value(QStringLiteral("stopBits")).toInt(config->stopBits);
    if (obj.value(QStringLiteral("timeoutMs")).isDouble())
        config->timeoutMs = obj.value(QStringLiteral("timeoutMs")).toInt(config->timeoutMs);
    if (obj.value(QStringLiteral("retries")).isDouble())
        config->retries = obj.value(QStringLiteral("retries")).toInt(config->retries);
    if (obj.value(QStringLiteral("busOfflineFailureThreshold")).isDouble())
        config->busOfflineFailureThreshold = obj.value(QStringLiteral("busOfflineFailureThreshold"))
            .toInt(config->busOfflineFailureThreshold);
}

AppConfig::AppConfig()
{
    m_meteringRtu.port = QStringLiteral("/dev/rs485_metering");
}

bool AppConfig::load(const QString &filePath, QString *error)
{
    QFile file(filePath);
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

    const QJsonObject root = doc.object();
    const QJsonObject modbus = root.value(QStringLiteral("modbus")).toObject();
    const QJsonObject legacyRtu = modbus.value(QStringLiteral("rtu")).toObject();

    if (!legacyRtu.isEmpty()) {
        loadModbusRtu(legacyRtu, &m_relayRtu);
        m_meteringRtu = m_relayRtu;
        m_meteringRtu.port = QStringLiteral("/dev/rs485_metering");
    }

    loadModbusRtu(modbus.value(QStringLiteral("relay")).toObject(), &m_relayRtu);
    loadModbusRtu(modbus.value(QStringLiteral("metering")).toObject(), &m_meteringRtu);

    loadModbusRtu(modbus.value(QStringLiteral("internalIo")).toObject(), &m_relayRtu);
    loadModbusRtu(modbus.value(QStringLiteral("internalMetering")).toObject(), &m_meteringRtu);

    return true;
}

bool AppConfig::save(const QString &filePath, QString *error) const
{
    const QFileInfo info(filePath);
    if (!QDir().mkpath(info.absolutePath())) {
        if (error)
            *error = QStringLiteral("Cannot create config directory: %1").arg(info.absolutePath());
        return false;
    }

    const QJsonObject root = {
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("modbus"), QJsonObject{
            {QStringLiteral("relay"), modbusRtuToJson(m_relayRtu)},
            {QStringLiteral("metering"), modbusRtuToJson(m_meteringRtu)}
        }}
    };

    QSaveFile file(filePath);
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

const ModbusRtuConfig &AppConfig::relayRtu() const
{
    return m_relayRtu;
}

const ModbusRtuConfig &AppConfig::meteringRtu() const
{
    return m_meteringRtu;
}

} // namespace DialogG2
