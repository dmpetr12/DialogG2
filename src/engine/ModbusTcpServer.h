#pragma once

#include "CabinetSnapshot.h"

#include <QObject>
#include <QTimer>
#include <QtSerialBus/QModbusDataUnit>
#include <QtSerialBus/QModbusDevice>

class QModbusTcpServer;

namespace DialogG2 {

class ModbusTcpServer : public QObject
{
    Q_OBJECT

public:
    explicit ModbusTcpServer(QObject *parent = nullptr);
    ~ModbusTcpServer() override;

    bool start(const QString &listenAddress, int port, int serverAddress);
    void stop();
    bool isRunning() const;

    void updateSnapshot(const CabinetSnapshot &snapshot);

signals:
    void logMessage(const QString &message);
    void manualEmergencyStartRequested();
    void manualEmergencyStopRequested();
    void stopTestRequested();
    void functionalTestRequested();
    void durationTestRequested();

private:
    bool setupServerMap();
    void refreshRegisters();
    void onStateChanged(int state);
    void onErrorOccurred(QModbusDevice::Error error);
    void onDataWritten(QModbusDataUnit::RegisterType table, int address, int size);
    void processWrittenCoil(int address, bool value);
    void resetCoil(int address);

    QModbusTcpServer *m_server = nullptr;
    QTimer m_refreshTimer;
    CabinetSnapshot m_snapshot;
};

} // namespace DialogG2
