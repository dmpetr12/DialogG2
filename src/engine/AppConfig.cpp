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

static QJsonObject modbusTcpToJson(const ModbusTcpConfig &config)
{
    return {
        {QStringLiteral("enabled"), config.enabled},
        {QStringLiteral("bind"), config.bind},
        {QStringLiteral("port"), config.port},
        {QStringLiteral("serverAddress"), config.serverAddress}
    };
}

static QJsonObject webServerToJson(const WebServerConfig &config)
{
    return {
        {QStringLiteral("enabled"), config.enabled},
        {QStringLiteral("bind"), config.bind},
        {QStringLiteral("port"), config.port},
        {QStringLiteral("root"), config.root}
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

static void loadModbusTcp(const QJsonObject &obj, ModbusTcpConfig *config)
{
    if (!config)
        return;

    if (obj.value(QStringLiteral("enabled")).isBool())
        config->enabled = obj.value(QStringLiteral("enabled")).toBool(config->enabled);
    if (obj.value(QStringLiteral("bind")).isString())
        config->bind = obj.value(QStringLiteral("bind")).toString(config->bind);
    if (obj.value(QStringLiteral("port")).isDouble())
        config->port = obj.value(QStringLiteral("port")).toInt(config->port);
    if (obj.value(QStringLiteral("serverAddress")).isDouble())
        config->serverAddress = obj.value(QStringLiteral("serverAddress")).toInt(config->serverAddress);
}

static void loadWebServer(const QJsonObject &obj, WebServerConfig *config)
{
    if (!config)
        return;

    if (obj.value(QStringLiteral("enabled")).isBool())
        config->enabled = obj.value(QStringLiteral("enabled")).toBool(config->enabled);
    if (obj.value(QStringLiteral("bind")).isString())
        config->bind = obj.value(QStringLiteral("bind")).toString(config->bind);
    if (obj.value(QStringLiteral("port")).isDouble())
        config->port = obj.value(QStringLiteral("port")).toInt(config->port);
    if (obj.value(QStringLiteral("root")).isString())
        config->root = obj.value(QStringLiteral("root")).toString(config->root);
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
    loadModbusTcp(modbus.value(QStringLiteral("tcp")).toObject(), &m_modbusTcp);
    loadWebServer(root.value(QStringLiteral("web")).toObject(), &m_webServer);
    loadWebServer(root.value(QStringLiteral("webServer")).toObject(), &m_webServer);

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
            {QStringLiteral("metering"), modbusRtuToJson(m_meteringRtu)},
            {QStringLiteral("tcp"), modbusTcpToJson(m_modbusTcp)}
        }},
        {QStringLiteral("web"), webServerToJson(m_webServer)}
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

const ModbusTcpConfig &AppConfig::modbusTcp() const
{
    return m_modbusTcp;
}

const WebServerConfig &AppConfig::webServer() const
{
    return m_webServer;
}

} // namespace DialogG2
