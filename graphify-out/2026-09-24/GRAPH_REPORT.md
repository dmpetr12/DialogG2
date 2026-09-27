# Graph Report - DialogG2  (2026-09-24)

## Corpus Check
- 120 files · ~92,114 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 58 file(s) not represented in the graph (top: .log 41, .qml 14, (none) 2)

## Summary
- 1451 nodes · 2884 edges · 86 communities (78 shown, 8 thin omitted)
- Extraction: 91% EXTRACTED · 9% INFERRED · 0% AMBIGUOUS · INFERRED: 260 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `c0525f07`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- test_controller_tests.cpp
- PanelFacade
- LineManager.cpp
- CabinetSnapshot.cpp
- TestController
- EngineRuntime::Impl
- BatterySnapshot
- ModbusRtuConfig
- EngineInputs
- Logger
- MeteringBusController
- metering
- ModbusController
- MeteringBusController.cpp
- What You Must Do When Invoked
- CabinetSnapshot
- ModbusTcpServer
- LineSnapshot
- ModbusController.cpp
- Adl200Measurement
- Amc16zFak24Meter
- decodeChannelHoldingRegisters
- ModbusBusMonitor
- Request
- ModbusTcpServer.cpp
- LineTestResult
- QObject
- EngineRuntime.cpp
- MeteringBusController.h
- TestJournalEntry
- handleRequestSuccess
- ModbusRtuCodec.cpp
- .setupWebRoutes
- Request
- decodeChannel1RealtimeRegisters
- TelemetryTestAccess
- BMS батареи: заметки по протоколу
- Modbus RTU
- CabinetSnapshot.h
- QString
- TestScheduleRequest
- .start
- PasswordManager
- ActiveTestSnapshot
- ManualEmergencyController
- sendRequest
- setupDevice
- parseResponse
- graphify reference: extra exports and benchmark
- LineOperationalCheck
- MaintenanceSnapshot
- handleCurrentResponse
- StateFileStore
- TestController
- LineOperationalMonitor
- TestControllerInputs
- LineManagerResult
- .Impl
- LineManager
- Dialog G2 Panel
- WaveShareModuleState
- graphify reference: query, path, explain
- Логирование
- MODBUS TCP
- Файл состояния
- Q: Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями.
- Q: Реле общей неисправности должно включаться при аварии входного напряжения
- handleRequestFailure
- Candidate
- graphify reference: add a URL and watch a folder
- graphify reference: commit hook and native CLAUDE.md integration
- graphify reference: incremental update and cluster-only
- Dialog G2: заметки по дизайну
- graphify reference: GitHub clone and cross-repo merge
- graphify reference: transcribe video and audio
- Web Server
- AGENTS.md
- extraction-spec.md
- line-operational-monitor.md
- update_panel.sh
- TestLineMeasurement
- TestControllerConfig
- Q: Мы не потеряли логику авария и неисправность?
- JbdBmsProtocol
- onDataWritten
- .branchPollsCorrect

## God Nodes (most connected - your core abstractions)
1. `EngineRuntime::Impl` - 124 edges
2. `MeteringBusController` - 82 edges
3. `ModbusController` - 73 edges
4. `PanelFacade` - 70 edges
5. `CabinetSnapshot` - 51 edges
6. `expect()` - 47 edges
7. `main()` - 46 edges
8. `LineSnapshot` - 44 edges
9. `toJson()` - 42 edges
10. `BatterySnapshot` - 40 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `QVariantList`  [INFERRED]
  tests/battery_page_tests.cpp → src/PanelFacade.h
- `insulationBreakdownIsPublishedForRemoteMonitoring()` --calls--> `toJson()`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/CabinetSnapshot.cpp
- `journalStorePersistsEntries()` --references--> `LineManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `lineManagerPersistsLastTestResults()` --calls--> `loadConfig`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `lineManagerPersistsLastTestResults()` --calls--> `line`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h

## Import Cycles
- None detected.

## Communities (86 total, 8 thin omitted)

### Community 0 - "test_controller_tests.cpp"
Cohesion: 0.05
Nodes (107): QDate, QTime, QVector, evaluate, evaluate, TestControllerResult, activeTest, journalEntries (+99 more)

### Community 1 - "PanelFacade"
Cohesion: 0.07
Nodes (78): Q_INVOKABLE, qint64, QJsonObject, QObject, QString, QStringList, QVariant, Q_OBJECT (+70 more)

### Community 2 - "LineManager.cpp"
Cohesion: 0.07
Nodes (63): CabinetIoMap, faultLampRelay, fireInput, manualFireButton, manualStopButton, modeRelay, reserveRelay, testLampRelay (+55 more)

### Community 3 - "CabinetSnapshot.cpp"
Cohesion: 0.09
Nodes (59): activeTestFromJson(), batteryFromJson(), batteryStateCode(), batteryStateText(), BatteryState, CabinetMode, LineKind, LineOperationalState (+51 more)

### Community 4 - "TestController"
Cohesion: 0.17
Nodes (19): QDateTime, QString, QVector, TestKind, TestRunStatus, TestSource, TestController, activeTestDue (+11 more)

### Community 5 - "EngineRuntime::Impl"
Cohesion: 0.04
Nodes (56): CabinetMode, QTimer, quint8, SystemHealth, EngineRuntime::Impl, m_battery, m_cachedMaintenance, m_config (+48 more)

### Community 6 - "BatterySnapshot"
Cohesion: 0.07
Nodes (29): BatterySnapshot, alarmStatusRaw, balancingActive, cellCount, cellVoltageDelta, cellVoltages, chargeAllowed, communicationOk (+21 more)

### Community 7 - "ModbusRtuConfig"
Cohesion: 0.08
Nodes (40): AppConfig, load, m_logging, m_meteringRtu, m_modbusTcp, m_relayRtu, m_webServer, save (+32 more)

### Community 8 - "EngineInputs"
Cohesion: 0.07
Nodes (36): CabinetMode, QString, QStringList, SystemHealth, EngineInputs, activeTest, battery, batteryFault (+28 more)

### Community 9 - "Logger"
Cohesion: 0.10
Nodes (37): QFile, QMessageLogContext, QMutex, QtMsgType, Level, qint64, QString, Level (+29 more)

### Community 10 - "MeteringBusController"
Cohesion: 0.06
Nodes (34): Q_OBJECT, QByteArray, QHash, QObject, QString, QTimer, QVector, MeteringBusController (+26 more)

### Community 11 - "metering"
Cohesion: 0.06
Nodes (32): logging, level, baudRate, busOfflineFailureThreshold, dataBits, parity, port, retries (+24 more)

### Community 12 - "ModbusController"
Cohesion: 0.06
Nodes (32): deque, Q_OBJECT, QObject, QTimer, QVector, ModbusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+24 more)

### Community 13 - "MeteringBusController.cpp"
Cohesion: 0.10
Nodes (24): DataBits, Parity, QObject, StopBits, dataBitsFromConfig(), addAdl200InputMeterPolling, addAsj60Ld16aLeakagePolling, addJbdBmsPolling (+16 more)

### Community 14 - "What You Must Do When Invoked"
Cohesion: 0.08
Nodes (24): For /graphify add and --watch, For /graphify query, For the commit hook and native CLAUDE.md integration, For --update and --cluster-only, /graphify, Honesty Rules, Interpreter guard for subcommands, Part A - Structural extraction for code files (+16 more)

### Community 15 - "CabinetSnapshot"
Cohesion: 0.08
Nodes (24): CabinetSnapshot, activeFaults, activeTest, battery, explanation, fireInputActive, health, inputCurrent (+16 more)

### Community 16 - "ModbusTcpServer"
Cohesion: 0.09
Nodes (27): Error, QObject, QString, Q_OBJECT, QObject, QTimer, ModbusTcpServer, durationTestRequested (+19 more)

### Community 17 - "LineSnapshot"
Cohesion: 0.10
Nodes (21): LineKind, LineOutputState, LineState, LineSnapshot, enabled, index, kind, lastDurationTest (+13 more)

### Community 18 - "ModbusController.cpp"
Cohesion: 0.10
Nodes (20): addAdl200InputMeterPolling, addAmc16zFak24BranchPowerPolling, addAsj60Ld16aLeakagePolling, addHoldingRegistersPolling, addInputRegistersPolling, addWaveShareModulePolling, addWhdTemperatureHumidityPolling, clearPollTasks (+12 more)

### Community 19 - "Adl200Measurement"
Cohesion: 0.12
Nodes (18): Adl200Measurement, activePower, apparentPower, current, frequency, powerFactor, reactivePower, valid (+10 more)

### Community 20 - "Amc16zFak24Meter"
Cohesion: 0.15
Nodes (17): Amc16zBranchMeasurement, activePower, channel, valid, Amc16zFak24Meter, ActivePowerBranchCount, ActivePowerHoldingCount, ActivePowerHoldingStart (+9 more)

### Community 21 - "decodeChannelHoldingRegisters"
Cohesion: 0.12
Nodes (17): Asj60Ld16aMonitor, ChannelCount, ChannelDataHoldingCount, ChannelDataHoldingStart, decodeChannelHoldingRegisters, DefaultSlaveAddress, RegistersPerChannel, Asj60LeakageChannel (+9 more)

### Community 22 - "ModbusBusMonitor"
Cohesion: 0.15
Nodes (17): busStatus, QString, QString, ModbusBusMonitor, m_offlineFailureThreshold, m_status, markFailure, markSuccess (+9 more)

### Community 23 - "Request"
Cohesion: 0.11
Nodes (18): MeterKind, qint64, quint8, RequestPriority, RequestType, PollTask, intervalMs, nextDueMsec (+10 more)

### Community 24 - "ModbusTcpServer.cpp"
Cohesion: 0.30
Nodes (16): quint32, batteryState(), cabinetState(), QDateTime, quint16, dateTimeToU32(), emergencyState(), highWord() (+8 more)

### Community 25 - "LineTestResult"
Cohesion: 0.25
Nodes (8): TestRunStatus, LineTestResult, completedAt, details, measuredPower, nominalPower, status, tolerancePercent

### Community 26 - "QObject"
Cohesion: 0.22
Nodes (7): Impl, QStringList, EngineRuntime, m_impl, start, QObject, unique_ptr

### Community 27 - "EngineRuntime.cpp"
Cohesion: 0.17
Nodes (15): QHttpServer, QLocalServer, QLocalSocket, applyHmiLineMode(), LineKind, QByteArray, QJsonObject, defaultLinesConfigPath() (+7 more)

### Community 28 - "MeteringBusController.h"
Cohesion: 0.35
Nodes (4): QVector, deque, TelemetryTestAccess, QTimer

### Community 29 - "TestJournalEntry"
Cohesion: 0.09
Nodes (31): TestJournalEntry, finishedAt, kind, lines, reason, source, startedAt, status (+23 more)

### Community 30 - "handleRequestSuccess"
Cohesion: 0.22
Nodes (15): quint16, quint8, QVector, Request, RequestPriority, bitsFromReply, enqueue, finishRequest (+7 more)

### Community 31 - "ModbusRtuCodec.cpp"
Cohesion: 0.34
Nodes (13): byteAt(), QByteArray, quint16, quint8, QVector, ModbusRtuCodec, appendCrc, bitsFromReadResponse (+5 more)

### Community 32 - ".setupWebRoutes"
Cohesion: 0.24
Nodes (3): QHttpServerRequest, QHttpServerResponse, QJsonArray

### Community 33 - "Request"
Cohesion: 0.15
Nodes (13): MeterKind, qint64, RequestType, PollTask, intervalMs, nextDueMsec, request, Request (+5 more)

### Community 34 - "decodeChannel1RealtimeRegisters"
Cohesion: 0.18
Nodes (12): quint16, QVector, signedTenths(), WhdMeasurement, humidity, temperature, valid, WhdTemperatureHumidityController (+4 more)

### Community 35 - "TelemetryTestAccess"
Cohesion: 0.13
Nodes (10): QString, handleRequestFailure, invalidateRequest, onReadyRead, processReceiveBuffer, requestKey, updateBusMonitorFailure, QByteArray (+2 more)

### Community 36 - "BMS батареи: заметки по протоколу"
Cohesion: 0.17
Nodes (11): BMS батареи: заметки по протоколу, Данные из `0x03`, Данные из `0x04`, Защиты, Интерфейс, Кадр, Команды для оперативного снимка, Предупреждения (+3 more)

### Community 37 - "Modbus RTU"
Cohesion: 0.17
Nodes (11): MeteringBusController для измерений, Modbus RTU, ModbusController для реле, U, I и знак мощности линий AMC16Z-FAK24, Входной измеритель ADL200, Датчик температуры и влажности WHD, Измеритель линий AMC16Z-FAK24, Измеритель утечки ASJ60-LD16A/C (+3 more)

### Community 38 - "CabinetSnapshot.h"
Cohesion: 0.39
Nodes (3): QDateTime, QByteArray, QString

### Community 39 - "QString"
Cohesion: 0.24
Nodes (8): main(), QObject, QString, QStringList, defaultAppConfigPath(), defaultLogPath(), defaultStatePath(), EngineRuntime::EngineRuntime()

### Community 40 - "TestScheduleRequest"
Cohesion: 0.17
Nodes (12): TestRequest, active, durationSeconds, QDateTime, QString, TestScheduleRequest, duration, entryIndex (+4 more)

### Community 41 - ".start"
Cohesion: 0.18
Nodes (3): qint64, defaultRuntimeTimingPath(), ipcServerName()

### Community 42 - "PasswordManager"
Cohesion: 0.24
Nodes (7): QSettings, QObject, QString, PasswordManager, m_settings, passwordChanged, Q_PROPERTY

### Community 43 - "ActiveTestSnapshot"
Cohesion: 0.20
Nodes (10): ActiveTestSnapshot, active, dueAt, durationSeconds, kind, source, startedAt, warmupSeconds (+2 more)

### Community 44 - "ManualEmergencyController"
Cohesion: 0.29
Nodes (8): ManualEmergencyController, active, evaluate, m_active, reset, ManualEmergencyInputs, startRequested, stopRequested

### Community 45 - "sendRequest"
Cohesion: 0.33
Nodes (6): Request, enqueue, finishCurrentRequest, onRequestTimeout, sameRequest, sendRequest

### Community 46 - "setupDevice"
Cohesion: 0.22
Nodes (10): DataBits, QObject, StopBits, dataBitsFromConfig(), configure, connectDevice, ModbusController::ModbusController(), recreateClient (+2 more)

### Community 47 - "parseResponse"
Cohesion: 0.29
Nodes (16): byteAt(), QByteArray, qint16, QString, QStringList, quint16, quint8, i16At() (+8 more)

### Community 48 - "graphify reference: extra exports and benchmark"
Cohesion: 0.22
Nodes (8): graphify reference: extra exports and benchmark, Step 6b - Wiki (only if --wiki flag), Step 7 - Neo4j export (only if --neo4j or --neo4j-push flag), Step 7a - FalkorDB export (only if --falkordb or --falkordb-push flag), Step 7b - SVG export (only if --svg flag), Step 7c - GraphML export (only if --graphml flag), Step 7d - MCP server (only if --mcp flag), Step 8 - Token reduction benchmark (only if total_words > 5000)

### Community 49 - "LineOperationalCheck"
Cohesion: 0.20
Nodes (10): LineOperationalState, QString, LineOperationalCheck, details, measuredPower, nominalPower, startedAt, state (+2 more)

### Community 50 - "MaintenanceSnapshot"
Cohesion: 0.14
Nodes (14): MaintenanceLineStatus, lastTestAt, lineIndex, lineName, overdue, MaintenanceSnapshot, lastLongTestAt, lineLimitDays (+6 more)

### Community 51 - "handleCurrentResponse"
Cohesion: 0.20
Nodes (10): quint8, JbdBmsResponse, callbackId, command, data, status, byteAt(), QByteArray (+2 more)

### Community 52 - "StateFileStore"
Cohesion: 0.36
Nodes (7): QString, QString, StateFileStore, m_filePath, read, StateFileStore::StateFileStore(), write

### Community 53 - "TestController"
Cohesion: 0.25
Nodes (7): TestController, Остановка оператором, Приоритеты, Расписание, Связь с LineManager, Типы тестов, Хранение

### Community 54 - "LineOperationalMonitor"
Cohesion: 0.17
Nodes (13): QHash, QDateTime, QDateTime, QHash, LineOperationalMonitor, checkLine, LineOperationalMonitor::LineOperationalMonitor(), m_config (+5 more)

### Community 55 - "TestControllerInputs"
Cohesion: 0.17
Nodes (12): QDateTime, QVector, TestControllerInputs, fireInputActive, lines, manualDuration, manualFunctional, now (+4 more)

### Community 56 - "LineManagerResult"
Cohesion: 0.20
Nodes (10): QStringList, QVector, LineManagerResult, faults, fireInputActive, lines, manualFireButtonActive, manualStopButtonActive (+2 more)

### Community 57 - ".Impl"
Cohesion: 0.19
Nodes (4): QDateTime, QHash, QVector, trimTestJournal()

### Community 58 - "LineManager"
Cohesion: 0.29
Nodes (6): LineManager, Логика, Нумерация линий, Первый модуль WaveShare, Ручки для настройки линий, Следующие модули WaveShare

### Community 59 - "Dialog G2 Panel"
Cohesion: 0.29
Nodes (6): Dialog G2 Panel, Движок, Отсутствие данных на панели, Релейные выходы WaveShare, Сборка, Состав

### Community 60 - "WaveShareModuleState"
Cohesion: 0.15
Nodes (13): quint8, LineManagerInputs, faultLampOn, forceLineIndex, forceLinesOn, modeRelayOn, modules, testLampOn (+5 more)

### Community 61 - "graphify reference: query, path, explain"
Cohesion: 0.33
Nodes (5): For /graphify explain, For /graphify path, graphify reference: query, path, explain, Step 0 — Constrained query expansion (REQUIRED before traversal), Step 1 — Traversal

### Community 62 - "Логирование"
Cohesion: 0.40
Nodes (4): Логирование, Ротация, Уровни, Файл

### Community 63 - "MODBUS TCP"
Cohesion: 0.40
Nodes (4): Coils, Input registers, Lines, MODBUS TCP

### Community 64 - "Файл состояния"
Cohesion: 0.40
Nodes (4): АКБ, Линии, Тесты, Файл состояния

### Community 65 - "Q: Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями."
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями., Source Nodes

### Community 66 - "Q: Реле общей неисправности должно включаться при аварии входного напряжения"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Реле общей неисправности должно включаться при аварии входного напряжения, Source Nodes

### Community 67 - "handleRequestFailure"
Cohesion: 0.40
Nodes (5): Parity, QString, handleRequestFailure, updateBusMonitorFailure, parityFromConfig()

### Community 68 - "Candidate"
Cohesion: 0.22
Nodes (9): Candidate, durationSeconds, kind, priority, source, valid, TestKind, TestSource (+1 more)

### Community 69 - "graphify reference: add a URL and watch a folder"
Cohesion: 0.50
Nodes (3): For /graphify add, For --watch, graphify reference: add a URL and watch a folder

### Community 70 - "graphify reference: commit hook and native CLAUDE.md integration"
Cohesion: 0.50
Nodes (3): For git commit hook, For native CLAUDE.md integration, graphify reference: commit hook and native CLAUDE.md integration

### Community 71 - "graphify reference: incremental update and cluster-only"
Cohesion: 0.50
Nodes (3): For --cluster-only, For --update (incremental re-extraction), graphify reference: incremental update and cluster-only

### Community 72 - "Dialog G2: заметки по дизайну"
Cohesion: 0.50
Nodes (3): Dialog G2: заметки по дизайну, Главный экран, Термины

### Community 80 - "TestLineMeasurement"
Cohesion: 0.25
Nodes (8): TestLineMeasurement, details, lineIndex, lineName, measuredPower, nominalPower, status, tolerancePercent

### Community 81 - "TestControllerConfig"
Cohesion: 0.33
Nodes (6): highestRequestedTest, TestController::TestController(), TestControllerConfig, defaultDurationSeconds, durationToleranceMultiplier, functionalWarmupSeconds

### Community 82 - "Q: Мы не потеряли логику авария и неисправность?"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Мы не потеряли логику авария и неисправность?, Source Nodes

### Community 83 - "JbdBmsProtocol"
Cohesion: 0.40
Nodes (5): JbdBmsProtocol, CommandBasicInfo, CommandCellVoltages, CommandHardwareVersion, CommandProtectionCounters

### Community 84 - "onDataWritten"
Cohesion: 0.50
Nodes (4): RegisterType, onDataWritten, processWrittenCoil, resetCoil

## Knowledge Gaps
- **572 isolated node(s):** `schemaVersion`, `port`, `baudRate`, `parity`, `dataBits` (+567 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 710 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **8 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Work-memory lessons

**Preferred sources** — corroborated by past sessions; start here.
- `CabinetSnapshot` (2× useful, score=1.765629543)

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `EngineRuntime::Impl` connect `EngineRuntime::Impl` to `test_controller_tests.cpp`, `PanelFacade`, `LineManager.cpp`, `TestController`, `BatterySnapshot`, `ModbusRtuConfig`, `EngineInputs`, `MeteringBusController`, `ModbusController`, `CabinetSnapshot`, `ModbusTcpServer`, `ModbusBusMonitor`, `EngineRuntime.cpp`, `TestJournalEntry`, `.setupWebRoutes`, `QString`, `TestScheduleRequest`, `.start`, `PasswordManager`, `ManualEmergencyController`, `MaintenanceSnapshot`, `StateFileStore`, `LineOperationalMonitor`, `.Impl`, `WaveShareModuleState`?**
  _High betweenness centrality (0.282) - this node is a cross-community bridge._
- **Why does `MeteringBusController` connect `MeteringBusController` to `Request`, `PanelFacade`, `TelemetryTestAccess`, `EngineRuntime::Impl`, `BatterySnapshot`, `ModbusRtuConfig`, `MeteringBusController.cpp`, `sendRequest`, `handleCurrentResponse`, `.branchPollsCorrect`, `ModbusBusMonitor`, `MeteringBusController.h`?**
  _High betweenness centrality (0.109) - this node is a cross-community bridge._
- **Why does `ModbusController` connect `ModbusController` to `handleRequestFailure`, `EngineRuntime::Impl`, `ModbusRtuConfig`, `setupDevice`, `ModbusController.cpp`, `ModbusBusMonitor`, `Request`, `MeteringBusController.h`, `handleRequestSuccess`?**
  _High betweenness centrality (0.090) - this node is a cross-community bridge._
- **What connects `schemaVersion`, `port`, `baudRate` to the rest of the system?**
  _572 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `test_controller_tests.cpp` be split into smaller, more focused modules?**
  _Cohesion score 0.05334692490655794 - nodes in this community are weakly interconnected._
- **Should `PanelFacade` be split into smaller, more focused modules?**
  _Cohesion score 0.06729356450191008 - nodes in this community are weakly interconnected._
- **Should `LineManager.cpp` be split into smaller, more focused modules?**
  _Cohesion score 0.07211538461538461 - nodes in this community are weakly interconnected._