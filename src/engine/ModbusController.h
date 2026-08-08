#pragma once

#include "Adl200Meter.h"
#include "Amc16zFak24Meter.h"
#include "AppConfig.h"
#include "Asj60Ld16aMonitor.h"
#include "ModbusBusMonitor.h"
#include "WhdTemperatureHumidityController.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QTimer>
#include <QVector>

#include <deque>

class QModbusReply;
class QModbusRtuSerialClient;

namespace DialogG2 {

class ModbusController : public QObject
{
    Q_OBJECT

public:
    explicit ModbusController(QObject *parent = nullptr);
    ~ModbusController() override;

    void configure(const ModbusRtuConfig &config);
    void setWaveShareBaseAddress(int address);
    void setWaveShareModuleCount(int count);

    bool isConnected() const;
    ModbusBusStatus busStatus() const;

    void connectDevice();
    void disconnectDevice();

    void startPolling();
    void stopPolling();
    void clearPollTasks();
    void addWaveShareModulePolling(int module, int inputsIntervalMs, int relaysIntervalMs);
    void addAdl200InputMeterPolling(int intervalMs = 1000, int slaveAddress = Adl200Meter::DefaultSlaveAddress);
    void addAmc16zFak24BranchPowerPolling(int intervalMs = 1000,
                                          int slaveAddress = Amc16zFak24Meter::DefaultSlaveAddress);
    void addAsj60Ld16aLeakagePolling(int intervalMs = 1000,
                                     int slaveAddress = Asj60Ld16aMonitor::DefaultSlaveAddress);
    void addWhdTemperatureHumidityPolling(
        int intervalMs = 5000,
        int slaveAddress = WhdTemperatureHumidityController::DefaultSlaveAddress);
    void addHoldingRegistersPolling(int slaveAddress, int start, int count, int intervalMs);
    void addInputRegistersPolling(int slaveAddress, int start, int count, int intervalMs);

    void readWaveShareInputs(int module);
    void readWaveShareRelays(int module);
    void writeWaveShareRelayByte(int module, quint8 bits);

signals:
    void connectedChanged(bool connected);
    void errorOccurred(const QString &message);

    void busOnline();
    void busOffline(const QString &reason);
    void busStatusChanged(const DialogG2::ModbusBusStatus &status);

    void waveShareInputsUpdated(int module, quint8 bits);
    void waveShareRelaysUpdated(int module, quint8 bits);
    void holdingRegistersUpdated(int slaveAddress, int start, QVector<quint16> values);
    void inputRegistersUpdated(int slaveAddress, int start, QVector<quint16> values);
    void adl200InputMeterUpdated(const DialogG2::Adl200Measurement &measurement);
    void amc16zFak24BranchPowersUpdated(QVector<DialogG2::Amc16zBranchMeasurement> measurements);
    void asj60Ld16aLeakageUpdated(QVector<DialogG2::Asj60LeakageChannel> channels);
    void whdTemperatureHumidityUpdated(const DialogG2::WhdMeasurement &measurement);

private:
    enum class RequestPriority { High, Normal, Low };
    enum class RequestType { WriteCoils8, ReadInputs, ReadCoils, ReadHolding, ReadInputRegs };
    enum class MeterKind { None, Adl200Input, Amc16zFak24BranchPower, Asj60Ld16aLeakage, WhdTemperatureHumidity };

    struct Request
    {
        RequestType type = RequestType::ReadInputs;
        int slaveAddress = 1;
        int start = 0;
        int count = 1;
        int module = 0;
        quint8 coilsBits = 0;
        MeterKind meterKind = MeterKind::None;
    };

    struct PollTask
    {
        Request request;
        int intervalMs = 1000;
        qint64 nextDueMsec = 0;
        RequestPriority priority = RequestPriority::Normal;
    };

    void setupDevice();
    void recreateClient();
    void pollTick();
    void enqueue(Request request, RequestPriority priority);
    void pump();
    void sendRequest(const Request &request);
    void finishRequest(const Request &request, QModbusReply *reply);
    void handleRequestSuccess(const Request &request, QModbusReply *reply);
    void handleRequestFailure(const Request &request, const QString &error);
    void updateBusMonitorSuccess();
    void updateBusMonitorFailure(const QString &error);
    void clearQueues();

    static bool samePeriodicRequest(const Request &a, const Request &b);
    static bool sameWriteTarget(const Request &a, const Request &b);
    static QVector<quint16> valuesFromReply(QModbusReply *reply);
    static quint8 bitsFromReply(QModbusReply *reply);

    QModbusRtuSerialClient *m_client = nullptr;
    ModbusRtuConfig m_config;
    ModbusBusMonitor m_busMonitor;

    int m_waveShareBaseAddress = 1;
    int m_waveShareModuleCount = 1;

    QTimer m_scheduler;
    QVector<PollTask> m_pollTasks;

    std::deque<Request> m_highQueue;
    std::deque<Request> m_normalQueue;
    std::deque<Request> m_lowQueue;
    bool m_busy = false;
};

} // namespace DialogG2
