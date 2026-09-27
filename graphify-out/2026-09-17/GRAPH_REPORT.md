# Graph Report - DialogG2  (2026-09-17)

## Corpus Check
- 117 files · ~85,890 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 19 file(s) not represented in the graph (top: .qml 14, (none) 2, .log 2)

## Summary
- 1399 nodes · 2737 edges · 77 communities (66 shown, 11 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 231 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `c0525f07`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- CabinetSnapshot.h
- PanelFacade
- LineManager
- handleCurrentResponse
- EngineRuntime::Impl
- What You Must Do When Invoked
- TestJournalEntry
- ModbusRtuConfig
- test_controller_tests.cpp
- Logger
- EngineInputs
- .Impl
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
- QVector
- decodeChannelHoldingRegisters
- ModbusBusMonitor
- Request
- evaluate
- .processIpcMessage
- ModbusTcpServer.cpp
- LineTestResult
- decodeActivePowerHoldingRegisters
- LineConfig
- EngineRuntime.cpp
- sendRequest
- handleRequestFailure
- QHttpServerResponse
- Request
- setupDevice
- ModbusController.h
- LineManager.cpp
- LineManagerResult
- TestControllerInputs
- BatterySnapshot
- handleRequestSuccess
- TestControllerResult
- MeteringBusController.h
- Request
- start
- BMS батареи: заметки по протоколу
- .start
- Modbus RTU
- onReadyRead
- handleRequestFailure
- ActiveTestSnapshot
- update_panel.sh
- .tick
- graphify reference: extra exports and benchmark
- CabinetIoMap
- WaveSharePoint
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

## God Nodes (most connected - your core abstractions)
1. `EngineRuntime::Impl` - 122 edges
2. `ModbusController` - 73 edges
3. `MeteringBusController` - 69 edges
4. `PanelFacade` - 68 edges
5. `CabinetSnapshot` - 49 edges
6. `LineSnapshot` - 44 edges
7. `toJson()` - 40 edges
8. `BatterySnapshot` - 40 edges
9. `expect()` - 36 edges
10. `main()` - 35 edges

## Surprising Connections (you probably didn't know these)
- `journalStorePersistsEntries()` --references--> `LineManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `lineManagerPersistsLastTestResults()` --calls--> `loadConfig`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `lineManagerPersistsLastTestResults()` --calls--> `line`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `maintenanceCheckerAcceptsFreshTests()` --calls--> `evaluate`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/MaintenanceChecker.h
- `maintenanceCheckerReportsMissingTests()` --calls--> `evaluate`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/MaintenanceChecker.h

## Import Cycles
- None detected.

## Communities (77 total, 11 thin omitted)

### Community 0 - "CabinetSnapshot.h"
Cohesion: 0.09
Nodes (26): LineOperationalState, QDateTime, QString, LineOperationalCheck, details, measuredPower, nominalPower, startedAt (+18 more)

### Community 1 - "PanelFacade"
Cohesion: 0.07
Nodes (78): Q_INVOKABLE, main(), qint64, QJsonObject, QObject, QString, QStringList, QVariant (+70 more)

### Community 2 - "LineManager"
Cohesion: 0.26
Nodes (16): QVector, LineManager, addLine, applyTestResults, defaultLinePoint, defaultLines, findLineIndex, line (+8 more)

### Community 3 - "handleCurrentResponse"
Cohesion: 0.10
Nodes (42): byteAt(), QByteArray, qint16, QString, QStringList, quint16, quint8, QByteArray (+34 more)

### Community 4 - "EngineRuntime::Impl"
Cohesion: 0.04
Nodes (54): CabinetMode, QTimer, quint8, SystemHealth, EngineRuntime::Impl, m_battery, m_cachedMaintenance, m_config (+46 more)

### Community 5 - "What You Must Do When Invoked"
Cohesion: 0.08
Nodes (24): For /graphify add and --watch, For /graphify query, For the commit hook and native CLAUDE.md integration, For --update and --cluster-only, /graphify, Honesty Rules, Interpreter guard for subcommands, Part A - Structural extraction for code files (+16 more)

### Community 6 - "TestJournalEntry"
Cohesion: 0.17
Nodes (17): TestJournalEntry, finishedAt, kind, lines, reason, source, startedAt, status (+9 more)

### Community 7 - "ModbusRtuConfig"
Cohesion: 0.08
Nodes (40): AppConfig, load, m_logging, m_meteringRtu, m_modbusTcp, m_relayRtu, m_webServer, save (+32 more)

### Community 8 - "test_controller_tests.cpp"
Cohesion: 0.05
Nodes (101): QDate, QTime, QVector, evaluate, ManualEmergencyController, active, evaluate, m_active (+93 more)

### Community 9 - "Logger"
Cohesion: 0.08
Nodes (43): QFile, QMessageLogContext, QMutex, QtMsgType, Level, qint64, QString, Level (+35 more)

### Community 10 - "EngineInputs"
Cohesion: 0.07
Nodes (35): CabinetMode, QString, QStringList, SystemHealth, EngineInputs, activeTest, battery, batteryFault (+27 more)

### Community 11 - ".Impl"
Cohesion: 0.17
Nodes (5): applyHmiLineMode(), LineKind, defaultLinesConfigPath(), lineModeToHmi(), maxModule()

### Community 12 - "metering"
Cohesion: 0.06
Nodes (32): logging, level, baudRate, busOfflineFailureThreshold, dataBits, parity, port, retries (+24 more)

### Community 13 - "CabinetSnapshot.cpp"
Cohesion: 0.09
Nodes (58): activeTestFromJson(), batteryFromJson(), batteryStateCode(), batteryStateText(), BatteryState, CabinetMode, LineKind, LineOperationalState (+50 more)

### Community 14 - "ModbusController"
Cohesion: 0.06
Nodes (31): deque, Q_OBJECT, QObject, QTimer, QVector, ModbusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+23 more)

### Community 15 - "MeteringBusController"
Cohesion: 0.07
Nodes (28): Q_OBJECT, QByteArray, QObject, QTimer, QVector, MeteringBusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+20 more)

### Community 16 - "CabinetSnapshot"
Cohesion: 0.08
Nodes (24): CabinetSnapshot, activeFaults, activeTest, battery, explanation, fireInputActive, health, inputCurrent (+16 more)

### Community 17 - "ModbusTcpServer"
Cohesion: 0.09
Nodes (23): Error, RegisterType, Q_OBJECT, QObject, QTimer, ModbusTcpServer, durationTestRequested, functionalTestRequested (+15 more)

### Community 18 - "MeteringBusController.cpp"
Cohesion: 0.11
Nodes (23): DataBits, Parity, QString, StopBits, dataBitsFromConfig(), addAdl200InputMeterPolling, addAmc16zFak24BranchPowerPolling, addAsj60Ld16aLeakagePolling (+15 more)

### Community 19 - "LineSnapshot"
Cohesion: 0.10
Nodes (21): LineKind, LineOutputState, LineState, LineSnapshot, enabled, index, kind, lastDurationTest (+13 more)

### Community 20 - "ModbusController.cpp"
Cohesion: 0.10
Nodes (20): addAdl200InputMeterPolling, addAmc16zFak24BranchPowerPolling, addAsj60Ld16aLeakagePolling, addHoldingRegistersPolling, addInputRegistersPolling, addWaveShareModulePolling, addWhdTemperatureHumidityPolling, clearPollTasks (+12 more)

### Community 21 - "TestController"
Cohesion: 0.15
Nodes (27): QDateTime, QString, QVector, TestKind, TestRunStatus, TestSource, TestController, activeCandidate (+19 more)

### Community 22 - "QVector"
Cohesion: 0.05
Nodes (42): Adl200Measurement, activePower, apparentPower, current, frequency, powerFactor, reactivePower, valid (+34 more)

### Community 23 - "decodeChannelHoldingRegisters"
Cohesion: 0.13
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
Cohesion: 0.20
Nodes (10): QLocalSocket, QByteArray, qint64, QJsonArray, QJsonObject, defaultTestSchedulePath(), errorResponse(), okResponse() (+2 more)

### Community 28 - "ModbusTcpServer.cpp"
Cohesion: 0.32
Nodes (15): quint32, batteryState(), cabinetState(), QDateTime, quint16, dateTimeToU32(), emergencyState(), highWord() (+7 more)

### Community 29 - "LineTestResult"
Cohesion: 0.12
Nodes (16): TestRunStatus, LineTestResult, completedAt, details, measuredPower, nominalPower, status, tolerancePercent (+8 more)

### Community 30 - "decodeActivePowerHoldingRegisters"
Cohesion: 0.16
Nodes (14): Amc16zBranchMeasurement, activePower, channel, valid, Amc16zFak24Meter, ActivePowerBranchCount, ActivePowerHoldingCount, ActivePowerHoldingStart (+6 more)

### Community 31 - "LineConfig"
Cohesion: 0.14
Nodes (14): LineKind, QString, LineConfig, enabled, index, kind, lastDurationTest, lastFunctionalTest (+6 more)

### Community 32 - "EngineRuntime.cpp"
Cohesion: 0.15
Nodes (13): QHttpServer, QHttpServerRequest, QLocalServer, main(), QObject, QString, QStringList, defaultAppConfigPath() (+5 more)

### Community 33 - "sendRequest"
Cohesion: 0.40
Nodes (6): Request, enqueue, finishCurrentRequest, onRequestTimeout, sameRequest, sendRequest

### Community 34 - "handleRequestFailure"
Cohesion: 0.40
Nodes (5): Parity, QString, handleRequestFailure, updateBusMonitorFailure, parityFromConfig()

### Community 36 - "Request"
Cohesion: 0.15
Nodes (13): MeterKind, qint64, RequestType, PollTask, intervalMs, nextDueMsec, request, Request (+5 more)

### Community 37 - "setupDevice"
Cohesion: 0.22
Nodes (10): DataBits, QObject, StopBits, dataBitsFromConfig(), configure, connectDevice, ModbusController::ModbusController(), recreateClient (+2 more)

### Community 39 - "LineManager.cpp"
Cohesion: 0.33
Nodes (14): LineKind, QJsonObject, QString, ioMapFromJson(), ioMapToJson(), lineConfigFromJson(), lineKindFromConfig(), lineKindToConfig() (+6 more)

### Community 40 - "LineManagerResult"
Cohesion: 0.09
Nodes (24): QHash, QStringList, quint8, QVector, LineManagerInputs, faultLampOn, forceLineIndex, forceLinesOn (+16 more)

### Community 41 - "TestControllerInputs"
Cohesion: 0.14
Nodes (14): QDateTime, TestControllerInputs, fireInputActive, lines, manualDuration, manualFunctional, now, scheduledDuration (+6 more)

### Community 42 - "BatterySnapshot"
Cohesion: 0.05
Nodes (42): Impl, QSettings, BatterySnapshot, alarmStatusRaw, balancingActive, cellCount, cellVoltageDelta, cellVoltages (+34 more)

### Community 43 - "handleRequestSuccess"
Cohesion: 0.31
Nodes (10): quint16, quint8, QVector, bitsFromReply, finishRequest, handleRequestSuccess, sendRequest, valuesFromReply (+2 more)

### Community 44 - "TestControllerResult"
Cohesion: 0.11
Nodes (19): Candidate, durationSeconds, kind, priority, source, valid, QVector, TestKind (+11 more)

### Community 47 - "Request"
Cohesion: 0.60
Nodes (5): Request, RequestPriority, enqueue, samePeriodicRequest, sameWriteTarget

### Community 48 - "start"
Cohesion: 0.25
Nodes (8): QObject, QString, ModbusTcpServer::ModbusTcpServer(), refreshRegisters, setupServerMap, start, stop, updateSnapshot

### Community 49 - "BMS батареи: заметки по протоколу"
Cohesion: 0.17
Nodes (11): BMS батареи: заметки по протоколу, Данные из `0x03`, Данные из `0x04`, Защиты, Интерфейс, Кадр, Команды для оперативного снимка, Предупреждения (+3 more)

### Community 53 - "Modbus RTU"
Cohesion: 0.20
Nodes (9): MeteringBusController для измерений, Modbus RTU, ModbusController для реле, Входной измеритель ADL200, Датчик температуры и влажности WHD, Измеритель линий AMC16Z-FAK24, Измеритель утечки ASJ60-LD16A/C, Контроль шины (+1 more)

### Community 54 - "onReadyRead"
Cohesion: 0.40
Nodes (5): byteAt(), QByteArray, quint8, expectedResponseSize, onReadyRead

### Community 55 - "handleRequestFailure"
Cohesion: 0.18
Nodes (8): QObject, disconnectDevice, handleRequestFailure, invalidateMeasurements, invalidateRequest, MeteringBusController::MeteringBusController(), QJsonValue, TelemetryTestAccess

### Community 57 - "ActiveTestSnapshot"
Cohesion: 0.20
Nodes (10): ActiveTestSnapshot, active, dueAt, durationSeconds, kind, source, startedAt, warmupSeconds (+2 more)

### Community 61 - ".tick"
Cohesion: 0.29
Nodes (4): QDateTime, QHash, QVector, trimTestJournal()

### Community 62 - "graphify reference: extra exports and benchmark"
Cohesion: 0.22
Nodes (8): graphify reference: extra exports and benchmark, Step 6b - Wiki (only if --wiki flag), Step 7 - Neo4j export (only if --neo4j or --neo4j-push flag), Step 7a - FalkorDB export (only if --falkordb or --falkordb-push flag), Step 7b - SVG export (only if --svg flag), Step 7c - GraphML export (only if --graphml flag), Step 7d - MCP server (only if --mcp flag), Step 8 - Token reduction benchmark (only if total_words > 5000)

### Community 63 - "CabinetIoMap"
Cohesion: 0.20
Nodes (10): CabinetIoMap, faultLampRelay, fireInput, manualFireButton, manualStopButton, modeRelay, reserveRelay, testLampRelay (+2 more)

### Community 64 - "WaveSharePoint"
Cohesion: 0.33
Nodes (8): QHash, quint8, evaluate, inputActive, setRelayBit, WaveSharePoint, channel, module

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

## Knowledge Gaps
- **558 isolated node(s):** `schemaVersion`, `port`, `baudRate`, `parity`, `dataBits` (+553 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 694 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **11 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `EngineRuntime::Impl` connect `EngineRuntime::Impl` to `CabinetSnapshot.h`, `PanelFacade`, `LineManager`, `TestJournalEntry`, `ModbusRtuConfig`, `test_controller_tests.cpp`, `Logger`, `EngineInputs`, `.Impl`, `ModbusController`, `MeteringBusController`, `CabinetSnapshot`, `ModbusTcpServer`, `TestController`, `QVector`, `ModbusBusMonitor`, `evaluate`, `.processIpcMessage`, `EngineRuntime.cpp`, `QHttpServerResponse`, `LineManagerResult`, `TestControllerInputs`, `BatterySnapshot`, `.start`, `.tick`?**
  _High betweenness centrality (0.283) - this node is a cross-community bridge._
- **Why does `ModbusController` connect `ModbusController` to `handleRequestFailure`, `EngineRuntime::Impl`, `setupDevice`, `ModbusController.h`, `ModbusRtuConfig`, `handleRequestSuccess`, `Request`, `ModbusController.cpp`, `ModbusBusMonitor`, `Request`?**
  _High betweenness centrality (0.099) - this node is a cross-community bridge._
- **Why does `MeteringBusController` connect `MeteringBusController` to `sendRequest`, `PanelFacade`, `handleCurrentResponse`, `Request`, `EngineRuntime::Impl`, `ModbusController.h`, `ModbusRtuConfig`, `BatterySnapshot`, `MeteringBusController.h`, `MeteringBusController.cpp`, `onReadyRead`, `handleRequestFailure`, `ModbusBusMonitor`?**
  _High betweenness centrality (0.098) - this node is a cross-community bridge._
- **What connects `schemaVersion`, `port`, `baudRate` to the rest of the system?**
  _558 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `CabinetSnapshot.h` be split into smaller, more focused modules?**
  _Cohesion score 0.08866995073891626 - nodes in this community are weakly interconnected._
- **Should `PanelFacade` be split into smaller, more focused modules?**
  _Cohesion score 0.06670584778136938 - nodes in this community are weakly interconnected._
- **Should `handleCurrentResponse` be split into smaller, more focused modules?**
  _Cohesion score 0.09620721554116558 - nodes in this community are weakly interconnected._