#pragma once

#include <QString>

namespace DialogG2 {

struct ModbusRtuConfig
{
    QString port = QStringLiteral("/dev/rs485_relay");
    int baudRate = 9600;
    QString parity = QStringLiteral("none");
    int dataBits = 8;
    int stopBits = 1;
    int timeoutMs = 250;
    int retries = 0;
    int busOfflineFailureThreshold = 3;
};

struct ModbusTcpConfig
{
    bool enabled = true;
    QString bind = QStringLiteral("0.0.0.0");
    int port = 502;
    int serverAddress = 1;
};

struct WebServerConfig
{
    bool enabled = true;
    QString bind = QStringLiteral("0.0.0.0");
    int port = 8080;
    QString root = QStringLiteral("web");
};

class AppConfig
{
public:
    AppConfig();

    bool load(const QString &filePath, QString *error = nullptr);
    bool save(const QString &filePath, QString *error = nullptr) const;

    const ModbusRtuConfig &relayRtu() const;
    const ModbusRtuConfig &meteringRtu() const;
    const ModbusTcpConfig &modbusTcp() const;
    const WebServerConfig &webServer() const;

private:
    ModbusRtuConfig m_relayRtu;
    ModbusRtuConfig m_meteringRtu;
    ModbusTcpConfig m_modbusTcp;
    WebServerConfig m_webServer;
};

} // namespace DialogG2
