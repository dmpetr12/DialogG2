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

class AppConfig
{
public:
    AppConfig();

    bool load(const QString &filePath, QString *error = nullptr);
    bool save(const QString &filePath, QString *error = nullptr) const;

    const ModbusRtuConfig &relayRtu() const;
    const ModbusRtuConfig &meteringRtu() const;

private:
    ModbusRtuConfig m_relayRtu;
    ModbusRtuConfig m_meteringRtu;
};

} // namespace DialogG2
