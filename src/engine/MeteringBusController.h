#pragma once

#include "Adl200Meter.h"
#include "Amc16zFak24Meter.h"
#include "AppConfig.h"
#include "Asj60Ld16aMonitor.h"
#include "CabinetSnapshot.h"
#include "JbdBmsProtocol.h"
#include "ModbusBusMonitor.h"
#include "WhdTemperatureHumidityController.h"

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <QVector>

#include <deque>

class QSerialPort;

namespace DialogG2 {

class MeteringBusController : public QObject
{
    Q_OBJECT

public:
    explicit MeteringBusController(QObject *parent = nullptr);
    ~MeteringBusController() override;

    void configure(const ModbusRtuConfig &config);
    bool isConnected() const;
    ModbusBusStatus busStatus() const;

    void connectDevice();
    void disconnectDevice();
    void startPolling();
    void stopPolling();
    void clearPollTasks();

    void addAdl200InputMeterPolling(int intervalMs = 1000, int slaveAddress = Adl200Meter::DefaultSlaveAddress);
    void addAmc16zFak24BranchPowerPolling(int intervalMs = 1000,
                                          int slaveAddress = Amc16zFak24Meter::DefaultSlaveAddress);
    void addAsj60Ld16aLeakagePolling(int intervalMs = 1000,
                                     int slaveAddress = Asj60Ld16aMonitor::DefaultSlaveAddress);
    void addWhdTemperatureHumidityPolling(
        int intervalMs = 5000,
        int slaveAddress = WhdTemperatureHumidityController::DefaultSlaveAddress);
    void addJbdBmsPolling(int basicInfoIntervalMs = 2000, int cellVoltagesIntervalMs = 10000);

signals:
    void connectedChanged(bool connected);
    void errorOccurred(const QString &message);
    void busOnline();
    void busOffline(const QString &reason);
    void busStatusChanged(const DialogG2::ModbusBusStatus &status);

    void adl200InputMeterUpdated(const DialogG2::Adl200Measurement &measurement);
    void amc16zFak24BranchPowersUpdated(QVector<DialogG2::Amc16zBranchMeasurement> measurements);
    void asj60Ld16aLeakageUpdated(QVector<DialogG2::Asj60LeakageChannel> channels);
    void whdTemperatureHumidityUpdated(const DialogG2::WhdMeasurement &measurement);
    void jbdBmsBatteryUpdated(const DialogG2::BatterySnapshot &battery);

private:
    enum class RequestType { ReadHolding, ReadInputRegs, JbdBmsBasicInfo, JbdBmsCellVoltages };
    enum class MeterKind { None, Adl200Input, Amc16zFak24BranchPower, Asj60Ld16aLeakage, WhdTemperatureHumidity };

    struct Request
    {
        RequestType type = RequestType::ReadHolding;
        int slaveAddress = 1;
        int start = 0;
        int count = 1;
        MeterKind meterKind = MeterKind::None;
    };

    struct PollTask
    {
        Request request;
        int intervalMs = 1000;
        qint64 nextDueMsec = 0;
    };

    void setupPort();
    void pollTick();
    void enqueue(const Request &request);
    void pump();
    void sendRequest(const Request &request);
    void onReadyRead();
    void onRequestTimeout();
    void handleCurrentResponse();
    void finishCurrentRequest();
    void handleRequestFailure(const QString &error);
    void updateBusMonitorSuccess();
    void updateBusMonitorFailure(const QString &error);

    static bool sameRequest(const Request &a, const Request &b);
    int expectedResponseSize() const;

    QSerialPort *m_port = nullptr;
    ModbusRtuConfig m_config;
    ModbusBusMonitor m_busMonitor;
    BatterySnapshot m_battery;

    QTimer m_scheduler;
    QTimer m_timeoutTimer;
    QVector<PollTask> m_pollTasks;
    std::deque<Request> m_queue;

    Request m_currentRequest;
    QByteArray m_rxBuffer;
    bool m_busy = false;
};

} // namespace DialogG2
