#pragma once

#include "CabinetSnapshot.h"

#include <QHash>
#include <QString>
#include <QVector>

namespace DialogG2 {

struct WaveSharePoint
{
    int module = 0;   // 1-based module address/index in config
    int channel = 0;  // 1..8

    bool isValid() const { return module > 0 && channel >= 1 && channel <= 8; }
};

struct LineConfig
{
    int index = 0;
    QString name;
    bool enabled = true;
    LineKind kind = LineKind::Constant;
    double nominalPower = std::numeric_limits<double>::quiet_NaN();
    double powerTestTolerancePercent = std::numeric_limits<double>::quiet_NaN();
    double leakageCurrentLimit = 30.0;
    WaveSharePoint requestInput;
    WaveSharePoint outputRelay;
    LineTestResult lastFunctionalTest;
    LineTestResult lastDurationTest;
};

struct CabinetIoMap
{
    WaveSharePoint fireInput;
    WaveSharePoint manualFireButton;
    WaveSharePoint manualStopButton;
    WaveSharePoint voltageControlInput;

    WaveSharePoint modeRelay;
    WaveSharePoint faultLampRelay;
    WaveSharePoint testLampRelay;
    WaveSharePoint reserveRelay;
};

struct WaveShareModuleState
{
    int module = 0;
    quint8 inputs = 0;
    quint8 relays = 0;
    bool online = true;
};

struct LineManagerInputs
{
    QHash<int, WaveShareModuleState> modules;
    bool modeRelayOn = false;
    bool faultLampOn = false;
    bool testLampOn = false;
};

struct LineManagerResult
{
    bool fireInputActive = false;
    bool voltageControlOk = true;
    bool manualFireButtonActive = false;
    bool manualStopButtonActive = false;
    QVector<LineSnapshot> lines;
    // Full 8-channel relay output byte per WaveShare module.
    // The Modbus layer should write each byte as one WriteCoils8 operation.
    QHash<int, quint8> relayOutputBytes;
    QStringList faults;
};

class LineManager
{
public:
    bool loadConfig(const QString &filePath, QString *error = nullptr);
    bool saveConfig(const QString &filePath, QString *error = nullptr) const;
    bool saveDefaultConfig(const QString &filePath, QString *error = nullptr) const;

    const QVector<LineConfig> &lines() const;
    const CabinetIoMap &ioMap() const;
    const LineConfig *line(int index) const;

    bool addLine(LineConfig line, QString *error = nullptr);
    bool removeLine(int index, QString *error = nullptr);
    bool updateLine(const LineConfig &line, QString *error = nullptr);
    void applyTestResults(const QVector<TestJournalEntry> &entries);
    LineConfig makeNextLine(LineKind kind = LineKind::NonConstant) const;
    WaveSharePoint nextDefaultLinePoint() const;
    int nextLineIndex() const;

    LineManagerResult evaluate(const LineManagerInputs &inputs) const;

    static QVector<LineConfig> defaultLines();
    static CabinetIoMap defaultIoMap();
    static WaveSharePoint defaultLinePoint(int lineIndex);

private:
    int findLineIndex(int index) const;
    bool pointIsUsed(const WaveSharePoint &point) const;

    static bool inputActive(const QHash<int, WaveShareModuleState> &modules, const WaveSharePoint &point);
    static void setRelayBit(QHash<int, quint8> *outputBytes, const WaveSharePoint &point, bool on);

    QVector<LineConfig> m_lines = defaultLines();
    CabinetIoMap m_ioMap = defaultIoMap();
};

} // namespace DialogG2
