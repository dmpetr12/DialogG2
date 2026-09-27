# Graph Report - DialogG2  (2026-09-20)

## Corpus Check
- 118 files · ~87,530 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 41 file(s) not represented in the graph (top: .log 24, .qml 14, (none) 2)

## Summary
- 1420 nodes · 2789 edges · 80 communities (72 shown, 8 thin omitted)
- Extraction: 91% EXTRACTED · 9% INFERRED · 0% AMBIGUOUS · INFERRED: 239 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `c0525f07`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- ModbusTcpServer.cpp
- PanelFacade
- LineManager.cpp
- BatterySnapshot
- EngineRuntime::Impl
- What You Must Do When Invoked
- TestJournalEntry
- ModbusRtuConfig
- test_controller_tests.cpp
- Logger
- EngineInputs
- parseResponse
- metering
- CabinetSnapshot.cpp
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
- evaluate
- .processIpcMessage
- start
- LineTestResult
- Amc16zFak24Meter
- ModbusRtuCodec.cpp
- QString
- MaintenanceSnapshot
- LineOperationalMonitor
- .setupWebRoutes
- Request
- setupDevice
- decodeChannel1RealtimeRegisters
- EngineRuntime.cpp
- PasswordManager
- TestControllerInputs
- LineOperationalCheck
- handleRequestSuccess
- TestControllerResult
- StateFileStore
- TestScheduleRequest
- handleRequestFailure
- CabinetSnapshot.h
- BMS батареи: заметки по протоколу
- .start
- LineManagerInputs
- QFile
- Modbus RTU
- EngineRuntime
- TelemetryTestAccess
- handleCurrentResponse
- ActiveTestSnapshot
- update_panel.sh
- .tick
- graphify reference: extra exports and benchmark
- TestController
- LineManager
- graphify reference: query, path, explain
- Dialog G2 Panel
- Логирование
- MODBUS TCP
- Файл состояния
- Q: Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями.
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
- TestControllerConfig

## God Nodes (most connected - your core abstractions)
1. `EngineRuntime::Impl` - 124 edges
2. `MeteringBusController` - 76 edges
3. `ModbusController` - 73 edges
4. `PanelFacade` - 68 edges
5. `CabinetSnapshot` - 49 edges
6. `LineSnapshot` - 44 edges
7. `toJson()` - 40 edges
8. `BatterySnapshot` - 40 edges
9. `expect()` - 38 edges
10. `main()` - 37 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `QVariantList`  [INFERRED]
  tests/battery_page_tests.cpp → src/PanelFacade.h
- `journalStorePersistsEntries()` --references--> `LineManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `lineManagerPersistsLastTestResults()` --calls--> `loadConfig`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `lineManagerPersistsLastTestResults()` --calls--> `line`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `maintenanceCheckerAcceptsFreshTests()` --calls--> `evaluate`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/MaintenanceChecker.h

## Import Cycles
- None detected.

## Communities (80 total, 8 thin omitted)

### Community 0 - "ModbusTcpServer.cpp"
Cohesion: 0.32
Nodes (15): quint32, batteryState(), cabinetState(), QDateTime, quint16, dateTimeToU32(), emergencyState(), highWord() (+7 more)

### Community 1 - "PanelFacade"
Cohesion: 0.07
Nodes (77): Q_INVOKABLE, qint64, QJsonObject, QObject, QString, QStringList, QVariant, Q_OBJECT (+69 more)

### Community 2 - "LineManager.cpp"
Cohesion: 0.05
Nodes (79): CabinetIoMap, faultLampRelay, fireInput, manualFireButton, manualStopButton, modeRelay, reserveRelay, testLampRelay (+71 more)

### Community 3 - "BatterySnapshot"
Cohesion: 0.07
Nodes (29): BatterySnapshot, alarmStatusRaw, balancingActive, cellCount, cellVoltageDelta, cellVoltages, chargeAllowed, communicationOk (+21 more)

### Community 4 - "EngineRuntime::Impl"
Cohesion: 0.03
Nodes (58): QHttpServer, QLocalServer, CabinetMode, QTimer, quint8, SystemHealth, EngineRuntime::Impl, m_battery (+50 more)

### Community 5 - "What You Must Do When Invoked"
Cohesion: 0.08
Nodes (24): For /graphify add and --watch, For /graphify query, For the commit hook and native CLAUDE.md integration, For --update and --cluster-only, /graphify, Honesty Rules, Interpreter guard for subcommands, Part A - Structural extraction for code files (+16 more)

### Community 6 - "TestJournalEntry"
Cohesion: 0.17
Nodes (17): TestJournalEntry, finishedAt, kind, lines, reason, source, startedAt, status (+9 more)

### Community 7 - "ModbusRtuConfig"
Cohesion: 0.07
Nodes (43): AppConfig, load, m_logging, m_meteringRtu, m_modbusTcp, m_relayRtu, m_webServer, save (+35 more)

### Community 8 - "test_controller_tests.cpp"
Cohesion: 0.06
Nodes (94): QDate, QTime, QVector, evaluate, ManualEmergencyController, active, evaluate, m_active (+86 more)

### Community 9 - "Logger"
Cohesion: 0.11
Nodes (35): QMessageLogContext, QMutex, QtMsgType, Level, qint64, QString, Level, qint64 (+27 more)

### Community 10 - "EngineInputs"
Cohesion: 0.07
Nodes (35): CabinetMode, QString, QStringList, SystemHealth, EngineInputs, activeTest, battery, batteryFault (+27 more)

### Community 11 - "parseResponse"
Cohesion: 0.21
Nodes (21): byteAt(), QByteArray, qint16, QString, QStringList, quint16, quint8, i16At() (+13 more)

### Community 12 - "metering"
Cohesion: 0.06
Nodes (32): logging, level, baudRate, busOfflineFailureThreshold, dataBits, parity, port, retries (+24 more)

### Community 13 - "CabinetSnapshot.cpp"
Cohesion: 0.09
Nodes (58): activeTestFromJson(), batteryFromJson(), batteryStateCode(), batteryStateText(), BatteryState, CabinetMode, LineKind, LineOperationalState (+50 more)

### Community 14 - "ModbusController"
Cohesion: 0.06
Nodes (32): deque, Q_OBJECT, QObject, QTimer, QVector, ModbusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+24 more)

### Community 15 - "MeteringBusController"
Cohesion: 0.07
Nodes (31): Q_OBJECT, QByteArray, QObject, QTimer, QVector, MeteringBusController, addAmc16zFak24BranchPowerPolling, adl200InputMeterUpdated (+23 more)

### Community 16 - "CabinetSnapshot"
Cohesion: 0.08
Nodes (24): CabinetSnapshot, activeFaults, activeTest, battery, explanation, fireInputActive, health, inputCurrent (+16 more)

### Community 17 - "ModbusTcpServer"
Cohesion: 0.09
Nodes (23): Error, RegisterType, Q_OBJECT, QObject, QTimer, ModbusTcpServer, durationTestRequested, functionalTestRequested (+15 more)

### Community 18 - "MeteringBusController.cpp"
Cohesion: 0.09
Nodes (31): DataBits, Parity, QObject, QString, Request, StopBits, dataBitsFromConfig(), addAdl200InputMeterPolling (+23 more)

### Community 19 - "LineSnapshot"
Cohesion: 0.10
Nodes (21): LineKind, LineOutputState, LineState, LineSnapshot, enabled, index, kind, lastDurationTest (+13 more)

### Community 20 - "ModbusController.cpp"
Cohesion: 0.10
Nodes (20): addAdl200InputMeterPolling, addAmc16zFak24BranchPowerPolling, addAsj60Ld16aLeakagePolling, addHoldingRegistersPolling, addInputRegistersPolling, addWaveShareModulePolling, addWhdTemperatureHumidityPolling, clearPollTasks (+12 more)

### Community 21 - "TestController"
Cohesion: 0.19
Nodes (22): QDateTime, QString, QVector, TestKind, TestRunStatus, TestSource, TestController, activeCandidate (+14 more)

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

### Community 26 - "evaluate"
Cohesion: 0.22
Nodes (14): QDateTime, QString, QVector, TestRunStatus, MaintenanceChecker, evaluate, isCompletedTestStatus, latestCompletedDurationTest (+6 more)

### Community 27 - ".processIpcMessage"
Cohesion: 0.21
Nodes (10): QLocalSocket, LineKind, QByteArray, QJsonObject, defaultLinesConfigPath(), errorResponse(), lineModeToHmi(), okResponse() (+2 more)

### Community 28 - "start"
Cohesion: 0.25
Nodes (8): QObject, QString, ModbusTcpServer::ModbusTcpServer(), refreshRegisters, setupServerMap, start, stop, updateSnapshot

### Community 29 - "LineTestResult"
Cohesion: 0.12
Nodes (16): TestRunStatus, LineTestResult, completedAt, details, measuredPower, nominalPower, status, tolerancePercent (+8 more)

### Community 30 - "Amc16zFak24Meter"
Cohesion: 0.15
Nodes (17): Amc16zBranchMeasurement, activePower, channel, valid, Amc16zFak24Meter, ActivePowerBranchCount, ActivePowerHoldingCount, ActivePowerHoldingStart (+9 more)

### Community 31 - "ModbusRtuCodec.cpp"
Cohesion: 0.34
Nodes (13): byteAt(), QByteArray, quint16, quint8, QVector, ModbusRtuCodec, appendCrc, bitsFromReadResponse (+5 more)

### Community 32 - "QString"
Cohesion: 0.16
Nodes (8): QObject, QString, QStringList, defaultLogPath(), defaultTestJournalPath(), defaultTestSchedulePath(), EngineRuntime::EngineRuntime(), maxModule()

### Community 33 - "MaintenanceSnapshot"
Cohesion: 0.14
Nodes (15): QString, MaintenanceLineStatus, lastTestAt, lineIndex, lineName, overdue, MaintenanceSnapshot, lastLongTestAt (+7 more)

### Community 34 - "LineOperationalMonitor"
Cohesion: 0.19
Nodes (12): QHash, QDateTime, QDateTime, QHash, LineOperationalMonitor, checkLine, LineOperationalMonitor::LineOperationalMonitor(), m_config (+4 more)

### Community 35 - ".setupWebRoutes"
Cohesion: 0.24
Nodes (3): QHttpServerRequest, QHttpServerResponse, QJsonArray

### Community 36 - "Request"
Cohesion: 0.15
Nodes (13): MeterKind, qint64, RequestType, PollTask, intervalMs, nextDueMsec, request, Request (+5 more)

### Community 37 - "setupDevice"
Cohesion: 0.22
Nodes (10): DataBits, QObject, StopBits, dataBitsFromConfig(), configure, connectDevice, ModbusController::ModbusController(), recreateClient (+2 more)

### Community 38 - "decodeChannel1RealtimeRegisters"
Cohesion: 0.18
Nodes (12): quint16, QVector, signedTenths(), WhdMeasurement, humidity, temperature, valid, WhdTemperatureHumidityController (+4 more)

### Community 39 - "EngineRuntime.cpp"
Cohesion: 0.29
Nodes (6): QDateTime, QVector, deque, TelemetryTestAccess, QObject, QTimer

### Community 40 - "PasswordManager"
Cohesion: 0.24
Nodes (7): QSettings, QObject, QString, PasswordManager, m_settings, passwordChanged, Q_PROPERTY

### Community 41 - "TestControllerInputs"
Cohesion: 0.17
Nodes (12): QDateTime, QVector, TestControllerInputs, fireInputActive, lines, manualDuration, manualFunctional, now (+4 more)

### Community 42 - "LineOperationalCheck"
Cohesion: 0.22
Nodes (9): LineOperationalState, LineOperationalCheck, details, measuredPower, nominalPower, startedAt, state, tolerancePercent (+1 more)

### Community 43 - "handleRequestSuccess"
Cohesion: 0.22
Nodes (15): quint16, quint8, QVector, Request, RequestPriority, bitsFromReply, enqueue, finishRequest (+7 more)

### Community 44 - "TestControllerResult"
Cohesion: 0.12
Nodes (18): Candidate, durationSeconds, kind, priority, source, valid, TestKind, TestSource (+10 more)

### Community 45 - "StateFileStore"
Cohesion: 0.36
Nodes (7): QString, QString, StateFileStore, m_filePath, read, StateFileStore::StateFileStore(), write

### Community 46 - "TestScheduleRequest"
Cohesion: 0.17
Nodes (12): TestRequest, active, durationSeconds, QDateTime, QString, TestScheduleRequest, duration, entryIndex (+4 more)

### Community 47 - "handleRequestFailure"
Cohesion: 0.40
Nodes (5): Parity, QString, handleRequestFailure, updateBusMonitorFailure, parityFromConfig()

### Community 49 - "BMS батареи: заметки по протоколу"
Cohesion: 0.17
Nodes (11): BMS батареи: заметки по протоколу, Данные из `0x03`, Данные из `0x04`, Защиты, Интерфейс, Кадр, Команды для оперативного снимка, Предупреждения (+3 more)

### Community 50 - ".start"
Cohesion: 0.18
Nodes (3): qint64, defaultRuntimeTimingPath(), ipcServerName()

### Community 51 - "LineManagerInputs"
Cohesion: 0.29
Nodes (7): LineManagerInputs, faultLampOn, forceLineIndex, forceLinesOn, modeRelayOn, modules, testLampOn

### Community 52 - "QFile"
Cohesion: 0.33
Nodes (3): QFile, QStringList, main()

### Community 53 - "Modbus RTU"
Cohesion: 0.17
Nodes (11): MeteringBusController для измерений, Modbus RTU, ModbusController для реле, U, I и знак мощности линий AMC16Z-FAK24, Входной измеритель ADL200, Датчик температуры и влажности WHD, Измеритель линий AMC16Z-FAK24, Измеритель утечки ASJ60-LD16A/C (+3 more)

### Community 54 - "EngineRuntime"
Cohesion: 0.40
Nodes (5): Impl, EngineRuntime, m_impl, start, unique_ptr

### Community 55 - "TelemetryTestAccess"
Cohesion: 0.16
Nodes (7): handleRequestFailure, invalidateRequest, onReadyRead, processReceiveBuffer, QByteArray, QJsonValue, TelemetryTestAccess

### Community 56 - "handleCurrentResponse"
Cohesion: 0.15
Nodes (13): quint8, JbdBmsResponse, callbackId, command, data, status, byteAt(), QByteArray (+5 more)

### Community 57 - "ActiveTestSnapshot"
Cohesion: 0.20
Nodes (10): ActiveTestSnapshot, active, dueAt, durationSeconds, kind, source, startedAt, warmupSeconds (+2 more)

### Community 61 - ".tick"
Cohesion: 0.29
Nodes (4): QDateTime, QHash, QVector, trimTestJournal()

### Community 62 - "graphify reference: extra exports and benchmark"
Cohesion: 0.22
Nodes (8): graphify reference: extra exports and benchmark, Step 6b - Wiki (only if --wiki flag), Step 7 - Neo4j export (only if --neo4j or --neo4j-push flag), Step 7a - FalkorDB export (only if --falkordb or --falkordb-push flag), Step 7b - SVG export (only if --svg flag), Step 7c - GraphML export (only if --graphml flag), Step 7d - MCP server (only if --mcp flag), Step 8 - Token reduction benchmark (only if total_words > 5000)

### Community 65 - "TestController"
Cohesion: 0.25
Nodes (7): TestController, Остановка оператором, Приоритеты, Расписание, Связь с LineManager, Типы тестов, Хранение

### Community 66 - "LineManager"
Cohesion: 0.29
Nodes (6): LineManager, Логика, Нумерация линий, Первый модуль WaveShare, Ручки для настройки линий, Следующие модули WaveShare

### Community 67 - "graphify reference: query, path, explain"
Cohesion: 0.33
Nodes (5): For /graphify explain, For /graphify path, graphify reference: query, path, explain, Step 0 — Constrained query expansion (REQUIRED before traversal), Step 1 — Traversal

### Community 68 - "Dialog G2 Panel"
Cohesion: 0.29
Nodes (6): Dialog G2 Panel, Движок, Отсутствие данных на панели, Релейные выходы WaveShare, Сборка, Состав

### Community 69 - "Логирование"
Cohesion: 0.40
Nodes (4): Логирование, Ротация, Уровни, Файл

### Community 70 - "MODBUS TCP"
Cohesion: 0.40
Nodes (4): Coils, Input registers, Lines, MODBUS TCP

### Community 71 - "Файл состояния"
Cohesion: 0.40
Nodes (4): АКБ, Линии, Тесты, Файл состояния

### Community 72 - "Q: Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями."
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями., Source Nodes

### Community 73 - "graphify reference: add a URL and watch a folder"
Cohesion: 0.50
Nodes (3): For /graphify add, For --watch, graphify reference: add a URL and watch a folder

### Community 74 - "graphify reference: commit hook and native CLAUDE.md integration"
Cohesion: 0.50
Nodes (3): For git commit hook, For native CLAUDE.md integration, graphify reference: commit hook and native CLAUDE.md integration

### Community 75 - "graphify reference: incremental update and cluster-only"
Cohesion: 0.50
Nodes (3): For --cluster-only, For --update (incremental re-extraction), graphify reference: incremental update and cluster-only

### Community 76 - "Dialog G2: заметки по дизайну"
Cohesion: 0.50
Nodes (3): Dialog G2: заметки по дизайну, Главный экран, Термины

### Community 85 - "TestControllerConfig"
Cohesion: 0.40
Nodes (5): TestController::TestController(), TestControllerConfig, defaultDurationSeconds, durationToleranceMultiplier, functionalWarmupSeconds

## Knowledge Gaps
- **563 isolated node(s):** `schemaVersion`, `port`, `baudRate`, `parity`, `dataBits` (+558 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 698 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **8 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `EngineRuntime::Impl` connect `EngineRuntime::Impl` to `PanelFacade`, `LineManager.cpp`, `BatterySnapshot`, `TestJournalEntry`, `ModbusRtuConfig`, `test_controller_tests.cpp`, `EngineInputs`, `ModbusController`, `MeteringBusController`, `CabinetSnapshot`, `ModbusTcpServer`, `TestController`, `ModbusBusMonitor`, `evaluate`, `.processIpcMessage`, `QString`, `MaintenanceSnapshot`, `LineOperationalMonitor`, `.setupWebRoutes`, `EngineRuntime.cpp`, `PasswordManager`, `StateFileStore`, `TestScheduleRequest`, `.start`, `.tick`?**
  _High betweenness centrality (0.278) - this node is a cross-community bridge._
- **Why does `MeteringBusController` connect `MeteringBusController` to `PanelFacade`, `BatterySnapshot`, `Request`, `EngineRuntime::Impl`, `ModbusRtuConfig`, `EngineRuntime.cpp`, `MeteringBusController.cpp`, `TelemetryTestAccess`, `handleCurrentResponse`, `ModbusBusMonitor`?**
  _High betweenness centrality (0.104) - this node is a cross-community bridge._
- **Why does `ModbusController` connect `ModbusController` to `EngineRuntime::Impl`, `setupDevice`, `ModbusRtuConfig`, `EngineRuntime.cpp`, `handleRequestSuccess`, `handleRequestFailure`, `ModbusController.cpp`, `ModbusBusMonitor`, `Request`?**
  _High betweenness centrality (0.091) - this node is a cross-community bridge._
- **What connects `schemaVersion`, `port`, `baudRate` to the rest of the system?**
  _563 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `PanelFacade` be split into smaller, more focused modules?**
  _Cohesion score 0.06744956338452274 - nodes in this community are weakly interconnected._
- **Should `LineManager.cpp` be split into smaller, more focused modules?**
  _Cohesion score 0.05123456790123457 - nodes in this community are weakly interconnected._
- **Should `BatterySnapshot` be split into smaller, more focused modules?**
  _Cohesion score 0.06896551724137931 - nodes in this community are weakly interconnected._