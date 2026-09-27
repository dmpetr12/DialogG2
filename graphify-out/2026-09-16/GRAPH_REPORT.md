# Graph Report - DialogG2  (2026-09-13)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 1291 nodes · 2650 edges · 60 communities (59 shown, 1 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 216 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `c0525f07`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- engine_main.cpp
- PanelFacade
- LineManager.cpp
- BatterySnapshot
- EngineRuntime
- CabinetState
- TestJournalEntry
- ModbusRtuConfig
- test_controller_tests.cpp
- Logger
- EngineInputs
- TestScheduleManager
- metering
- toJson
- ModbusController
- MeteringBusController
- CabinetSnapshot
- ModbusTcpServer
- MeteringBusController.cpp
- LineSnapshot
- ModbusController.cpp
- TestController
- Adl200Measurement
- decodeChannelHoldingRegisters
- ModbusBusMonitor
- Request
- QString
- .processIpcMessage
- ModbusTcpServer.cpp
- LineTestResult
- decodeActivePowerHoldingRegisters
- CabinetSnapshot.cpp
- MaintenanceSnapshot
- ModbusRtuCodec.cpp
- LineOperationalMonitor
- .setupWebRoutes
- Request
- setupDevice
- TestScheduleRequest
- .tick
- .start
- TestControllerInputs
- PasswordManager
- handleRequestSuccess
- TestControllerResult
- LineOperationalCheck
- ManualEmergencyController
- Request
- start
- Candidate
- numbersToJson
- setupPort
- testScheduleStartsWeekdayDuration
- dateTimeOrNull
- onReadyRead
- parityFromConfig
- TestControllerConfig
- measureLine
- putFloatRegisters
- update_panel.sh

## God Nodes (most connected - your core abstractions)
1. `EngineRuntime` - 122 edges
2. `ModbusController` - 73 edges
3. `MeteringBusController` - 63 edges
4. `PanelFacade` - 60 edges
5. `CabinetSnapshot` - 49 edges
6. `CabinetState` - 46 edges
7. `LineSnapshot` - 44 edges
8. `BatterySnapshot` - 40 edges
9. `toJson()` - 40 edges
10. `expect()` - 36 edges

## Surprising Connections (you probably didn't know these)
- `lineManagerForcesEnabledLinesOnDuringTest()` --references--> `TestScheduleManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/TestScheduleManager.h
- `lineManagerForcesSelectedLineOnDuringSetup()` --references--> `TestScheduleManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/TestScheduleManager.h
- `lineManagerPersistsLastTestResults()` --references--> `TestScheduleManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/TestScheduleManager.h
- `lineManagerUsesNormallyClosedLineOutputRelays()` --references--> `TestScheduleManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/TestScheduleManager.h
- `journalStorePersistsEntries()` --references--> `LineManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h

## Import Cycles
- None detected.

## Communities (60 total, 1 thin omitted)

### Community 0 - "engine_main.cpp"
Cohesion: 0.05
Nodes (53): QFile, QObject, QString, QTimer, QDateTime, QStringList, QVector, QByteArray (+45 more)

### Community 1 - "PanelFacade"
Cohesion: 0.07
Nodes (71): Q_INVOKABLE, main(), qint64, QJsonObject, QObject, QString, QStringList, QVariant (+63 more)

### Community 2 - "LineManager.cpp"
Cohesion: 0.07
Nodes (63): CabinetIoMap, faultLampRelay, fireInput, manualFireButton, manualStopButton, modeRelay, reserveRelay, testLampRelay (+55 more)

### Community 3 - "BatterySnapshot"
Cohesion: 0.05
Nodes (57): BatterySnapshot, alarmStatusRaw, balancingActive, cellCount, cellVoltageDelta, cellVoltages, chargeAllowed, communicationOk (+49 more)

### Community 4 - "EngineRuntime"
Cohesion: 0.04
Nodes (57): QHttpServer, QLocalServer, CabinetMode, QObject, QTimer, quint8, SystemHealth, EngineRuntime (+49 more)

### Community 5 - "CabinetState"
Cohesion: 0.07
Nodes (49): Q_ENUM, CabinetState, applyJson, batteryOk, batteryPercent, CabinetState::CabinetState(), changed, health (+41 more)

### Community 6 - "TestJournalEntry"
Cohesion: 0.07
Nodes (41): ActiveTestSnapshot, active, dueAt, durationSeconds, kind, source, startedAt, warmupSeconds (+33 more)

### Community 7 - "ModbusRtuConfig"
Cohesion: 0.08
Nodes (40): AppConfig, load, m_logging, m_meteringRtu, m_modbusTcp, m_relayRtu, m_webServer, save (+32 more)

### Community 8 - "test_controller_tests.cpp"
Cohesion: 0.16
Nodes (39): evaluate, activeTestsRequestBatteryMode(), adl200DecoderScalesRealtimeRegisters(), amc16zFak24DecoderScalesBranchPowers(), asj60Ld16aDecoderReadsChannelStatusesAndLeakage(), QString, durationJournalEntry(), durationTestUsesExpandedTolerance() (+31 more)

### Community 9 - "Logger"
Cohesion: 0.11
Nodes (35): QMessageLogContext, QMutex, QtMsgType, Level, qint64, QString, Level, qint64 (+27 more)

### Community 10 - "EngineInputs"
Cohesion: 0.07
Nodes (35): CabinetMode, QString, QStringList, SystemHealth, EngineInputs, activeTest, battery, batteryFault (+27 more)

### Community 11 - "TestScheduleManager"
Cohesion: 0.13
Nodes (35): QDateTime, QJsonObject, QString, QVariant, QVector, TestKind, entryFromJson(), entryToJson() (+27 more)

### Community 12 - "metering"
Cohesion: 0.06
Nodes (32): logging, level, baudRate, busOfflineFailureThreshold, dataBits, parity, port, retries (+24 more)

### Community 13 - "toJson"
Cohesion: 0.12
Nodes (31): batteryStateCode(), batteryStateText(), BatteryState, CabinetMode, LineKind, LineOperationalState, LineOutputState, LineState (+23 more)

### Community 14 - "ModbusController"
Cohesion: 0.06
Nodes (32): deque, Q_OBJECT, QObject, QTimer, QVector, ModbusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+24 more)

### Community 15 - "MeteringBusController"
Cohesion: 0.07
Nodes (29): Q_OBJECT, QByteArray, QObject, QTimer, QVector, MeteringBusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+21 more)

### Community 16 - "CabinetSnapshot"
Cohesion: 0.08
Nodes (24): CabinetSnapshot, activeFaults, activeTest, battery, explanation, fireInputActive, health, inputCurrent (+16 more)

### Community 17 - "ModbusTcpServer"
Cohesion: 0.09
Nodes (23): Error, RegisterType, Q_OBJECT, QObject, QTimer, ModbusTcpServer, durationTestRequested, functionalTestRequested (+15 more)

### Community 18 - "MeteringBusController.cpp"
Cohesion: 0.11
Nodes (21): QObject, Request, addAdl200InputMeterPolling, addAmc16zFak24BranchPowerPolling, addAsj60Ld16aLeakagePolling, addJbdBmsPolling, addWhdTemperatureHumidityPolling, clearPollTasks (+13 more)

### Community 19 - "LineSnapshot"
Cohesion: 0.10
Nodes (21): LineKind, LineOutputState, LineState, LineSnapshot, enabled, index, kind, lastDurationTest (+13 more)

### Community 20 - "ModbusController.cpp"
Cohesion: 0.10
Nodes (20): addAdl200InputMeterPolling, addAmc16zFak24BranchPowerPolling, addAsj60Ld16aLeakagePolling, addHoldingRegistersPolling, addInputRegistersPolling, addWaveShareModulePolling, addWhdTemperatureHumidityPolling, clearPollTasks (+12 more)

### Community 21 - "TestController"
Cohesion: 0.22
Nodes (19): QDateTime, QString, QVector, TestRunStatus, QVector, TestController, activeCandidate, activeTestDue (+11 more)

### Community 22 - "Adl200Measurement"
Cohesion: 0.12
Nodes (18): Adl200Measurement, activePower, apparentPower, current, frequency, powerFactor, reactivePower, valid (+10 more)

### Community 23 - "decodeChannelHoldingRegisters"
Cohesion: 0.12
Nodes (17): Asj60Ld16aMonitor, ChannelCount, ChannelDataHoldingCount, ChannelDataHoldingStart, decodeChannelHoldingRegisters, DefaultSlaveAddress, RegistersPerChannel, Asj60LeakageChannel (+9 more)

### Community 24 - "ModbusBusMonitor"
Cohesion: 0.15
Nodes (17): busStatus, QString, QString, ModbusBusMonitor, m_offlineFailureThreshold, m_status, markFailure, markSuccess (+9 more)

### Community 25 - "Request"
Cohesion: 0.11
Nodes (18): MeterKind, qint64, quint8, RequestPriority, RequestType, PollTask, intervalMs, nextDueMsec (+10 more)

### Community 26 - "QString"
Cohesion: 0.16
Nodes (9): QString, QStringList, defaultAppConfigPath(), defaultLogPath(), defaultStatePath(), defaultTestJournalPath(), defaultTestSchedulePath(), main() (+1 more)

### Community 27 - ".processIpcMessage"
Cohesion: 0.22
Nodes (10): QLocalSocket, LineKind, QByteArray, QJsonObject, defaultLinesConfigPath(), errorResponse(), lineModeToHmi(), okResponse() (+2 more)

### Community 28 - "ModbusTcpServer.cpp"
Cohesion: 0.32
Nodes (15): quint32, batteryState(), cabinetState(), QDateTime, quint16, dateTimeToU32(), emergencyState(), highWord() (+7 more)

### Community 29 - "LineTestResult"
Cohesion: 0.12
Nodes (16): TestRunStatus, LineTestResult, completedAt, details, measuredPower, nominalPower, status, tolerancePercent (+8 more)

### Community 30 - "decodeActivePowerHoldingRegisters"
Cohesion: 0.15
Nodes (14): Amc16zBranchMeasurement, activePower, channel, valid, Amc16zFak24Meter, ActivePowerBranchCount, ActivePowerHoldingCount, ActivePowerHoldingStart (+6 more)

### Community 31 - "CabinetSnapshot.cpp"
Cohesion: 0.46
Nodes (14): activeTestFromJson(), batteryFromJson(), QJsonObject, dateTimeFromJson(), doubleOrNaN(), intOrUnknown(), lineFromJson(), lineOperationalCheckFromJson() (+6 more)

### Community 32 - "MaintenanceSnapshot"
Cohesion: 0.14
Nodes (15): QString, MaintenanceLineStatus, lastTestAt, lineIndex, lineName, overdue, MaintenanceSnapshot, lastLongTestAt (+7 more)

### Community 33 - "ModbusRtuCodec.cpp"
Cohesion: 0.34
Nodes (13): byteAt(), QByteArray, quint16, quint8, QVector, ModbusRtuCodec, appendCrc, bitsFromReadResponse (+5 more)

### Community 34 - "LineOperationalMonitor"
Cohesion: 0.20
Nodes (13): QDateTime, QVector, QDateTime, QHash, LineOperationalMonitor, checkLine, evaluate, LineOperationalMonitor::LineOperationalMonitor() (+5 more)

### Community 35 - ".setupWebRoutes"
Cohesion: 0.24
Nodes (3): QHttpServerRequest, QHttpServerResponse, QJsonArray

### Community 36 - "Request"
Cohesion: 0.15
Nodes (13): MeterKind, qint64, RequestType, PollTask, intervalMs, nextDueMsec, request, Request (+5 more)

### Community 37 - "setupDevice"
Cohesion: 0.18
Nodes (12): DataBits, Parity, QObject, StopBits, dataBitsFromConfig(), configure, connectDevice, ModbusController::ModbusController() (+4 more)

### Community 38 - "TestScheduleRequest"
Cohesion: 0.17
Nodes (12): TestRequest, active, durationSeconds, QDateTime, QString, TestScheduleRequest, duration, entryIndex (+4 more)

### Community 39 - ".tick"
Cohesion: 0.25
Nodes (4): QDateTime, QHash, QVector, trimTestJournal()

### Community 40 - ".start"
Cohesion: 0.18
Nodes (3): qint64, defaultRuntimeTimingPath(), ipcServerName()

### Community 41 - "TestControllerInputs"
Cohesion: 0.18
Nodes (11): QDateTime, TestControllerInputs, fireInputActive, lines, manualDuration, manualFunctional, now, scheduledDuration (+3 more)

### Community 42 - "PasswordManager"
Cohesion: 0.24
Nodes (7): QSettings, QObject, QString, PasswordManager, m_settings, passwordChanged, Q_PROPERTY

### Community 43 - "handleRequestSuccess"
Cohesion: 0.31
Nodes (10): quint16, quint8, QVector, bitsFromReply, finishRequest, handleRequestSuccess, sendRequest, valuesFromReply (+2 more)

### Community 44 - "TestControllerResult"
Cohesion: 0.20
Nodes (10): TestControllerResult, activeTest, journalEntries, lines, manualTestActive, modeRelayOn, newJournalEntries, scheduledTestActive (+2 more)

### Community 45 - "LineOperationalCheck"
Cohesion: 0.22
Nodes (9): LineOperationalState, LineOperationalCheck, details, measuredPower, nominalPower, startedAt, state, tolerancePercent (+1 more)

### Community 46 - "ManualEmergencyController"
Cohesion: 0.28
Nodes (7): ManualEmergencyController, active, m_active, reset, ManualEmergencyInputs, startRequested, stopRequested

### Community 47 - "Request"
Cohesion: 0.32
Nodes (8): QString, Request, RequestPriority, enqueue, handleRequestFailure, samePeriodicRequest, sameWriteTarget, updateBusMonitorFailure

### Community 48 - "start"
Cohesion: 0.25
Nodes (8): QObject, QString, ModbusTcpServer::ModbusTcpServer(), refreshRegisters, setupServerMap, start, stop, updateSnapshot

### Community 49 - "Candidate"
Cohesion: 0.25
Nodes (8): Candidate, durationSeconds, kind, priority, source, valid, TestKind, TestSource

### Community 50 - "numbersToJson"
Cohesion: 0.38
Nodes (7): QJsonArray, QStringList, QVector, numbersFromJson(), numbersToJson(), stringsFromJson(), stringsToJson()

### Community 51 - "setupPort"
Cohesion: 0.29
Nodes (7): DataBits, StopBits, dataBitsFromConfig(), configure, connectDevice, setupPort, stopBitsFromConfig()

### Community 52 - "testScheduleStartsWeekdayDuration"
Cohesion: 0.47
Nodes (6): QDate, QTime, QDateTime, testSchedulePersistsLegacyArrayFormat(), testScheduleStartsDailyFunctionalOnce(), testScheduleStartsWeekdayDuration()

### Community 53 - "dateTimeOrNull"
Cohesion: 0.40
Nodes (6): QJsonValue, QDateTime, dateTimeOrNull(), dateTimeToJson(), intOrNull(), numberOrNull()

### Community 54 - "onReadyRead"
Cohesion: 0.40
Nodes (5): byteAt(), QByteArray, quint8, expectedResponseSize, onReadyRead

### Community 55 - "parityFromConfig"
Cohesion: 0.40
Nodes (5): Parity, QString, handleRequestFailure, updateBusMonitorFailure, parityFromConfig()

### Community 56 - "TestControllerConfig"
Cohesion: 0.40
Nodes (5): TestController::TestController(), TestControllerConfig, defaultDurationSeconds, durationToleranceMultiplier, functionalWarmupSeconds

### Community 57 - "measureLine"
Cohesion: 0.50
Nodes (4): TestKind, TestSource, measureLine, priority

### Community 58 - "putFloatRegisters"
Cohesion: 0.67
Nodes (3): quint16, QVector, putFloatRegisters()

## Knowledge Gaps
- **478 isolated node(s):** `faultLampOn`, `forceLineIndex`, `forceLinesOn`, `modeRelayOn`, `modules` (+473 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 596 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **1 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `EngineRuntime` connect `EngineRuntime` to `engine_main.cpp`, `LineManager.cpp`, `BatterySnapshot`, `TestJournalEntry`, `ModbusRtuConfig`, `EngineInputs`, `TestScheduleManager`, `toJson`, `ModbusController`, `MeteringBusController`, `CabinetSnapshot`, `ModbusTcpServer`, `TestController`, `ModbusBusMonitor`, `QString`, `.processIpcMessage`, `MaintenanceSnapshot`, `LineOperationalMonitor`, `.setupWebRoutes`, `TestScheduleRequest`, `.tick`, `.start`, `PasswordManager`, `ManualEmergencyController`?**
  _High betweenness centrality (0.364) - this node is a cross-community bridge._
- **Why does `ModbusController` connect `ModbusController` to `engine_main.cpp`, `EngineRuntime`, `setupDevice`, `ModbusRtuConfig`, `handleRequestSuccess`, `Request`, `ModbusController.cpp`, `ModbusBusMonitor`, `Request`?**
  _High betweenness centrality (0.179) - this node is a cross-community bridge._
- **Why does `BatterySnapshot` connect `BatterySnapshot` to `engine_main.cpp`, `EngineRuntime`, `EngineInputs`, `toJson`, `MeteringBusController`, `CabinetSnapshot`, `ModbusTcpServer.cpp`, `CabinetSnapshot.cpp`?**
  _High betweenness centrality (0.108) - this node is a cross-community bridge._
- **What connects `faultLampOn`, `forceLineIndex`, `forceLinesOn` to the rest of the system?**
  _478 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `engine_main.cpp` be split into smaller, more focused modules?**
  _Cohesion score 0.051615051615051616 - nodes in this community are weakly interconnected._
- **Should `PanelFacade` be split into smaller, more focused modules?**
  _Cohesion score 0.07433489827856025 - nodes in this community are weakly interconnected._
- **Should `LineManager.cpp` be split into smaller, more focused modules?**
  _Cohesion score 0.07115384615384615 - nodes in this community are weakly interconnected._