# Graph Report - DialogG2  (2026-10-03)

## Corpus Check
- 125 files · ~97,475 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 58 file(s) not represented in the graph (top: .log 41, .qml 14, (none) 2)

## Summary
- 1509 nodes · 3013 edges · 87 communities (80 shown, 7 thin omitted)
- Extraction: 91% EXTRACTED · 9% INFERRED · 0% AMBIGUOUS · INFERRED: 276 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `3c44780a`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- test_controller_tests.cpp
- PanelFacade
- LineManager
- toJson
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
- update
- MaintenanceSnapshot
- LineManager.cpp
- .processIpcMessage
- EngineRuntime.cpp
- ModbusTcpServer.cpp
- handleRequestSuccess
- ModbusRtuCodec.cpp
- .setupWebRoutes
- Request
- LineConfig
- TelemetryTestAccess
- BMS батареи: заметки по протоколу
- Modbus RTU
- TestJournalEntry
- QString
- CabinetSnapshot.cpp
- .start
- WaveSharePoint
- ActiveTestSnapshot
- ManualEmergencyController
- TestControllerResult
- CabinetIoMap
- TestScheduleRequest
- graphify reference: extra exports and benchmark
- Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический
- setupDevice
- handleCurrentResponse
- measureLine
- TestController
- numbersToJson
- TestControllerInputs
- Q: Всё исправно, но по линиям показывает нет данных
- .tick
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
- LineManagerResult
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
- start
- TestControllerConfig
- Q: Мы не потеряли логику авария и неисправность?
- LineOperationalCheck
- dateTimeOrNull
- web_ui_tests.js
- Candidate

## God Nodes (most connected - your core abstractions)
1. `EngineRuntime::Impl` - 122 edges
2. `MeteringBusController` - 84 edges
3. `ModbusController` - 76 edges
4. `PanelFacade` - 71 edges
5. `CabinetSnapshot` - 55 edges
6. `expect()` - 53 edges
7. `main()` - 52 edges
8. `LineSnapshot` - 47 edges
9. `toJson()` - 44 edges
10. `BatterySnapshot` - 40 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `QVariantList`  [INFERRED]
  tests/battery_page_tests.cpp → src/PanelFacade.h
- `insulationBreakdownIsPublishedForRemoteMonitoring()` --calls--> `toJson()`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/CabinetSnapshot.cpp
- `maintenanceCheckerReportsFailedTests()` --calls--> `toJson()`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/CabinetSnapshot.cpp
- `journalStorePersistsEntries()` --references--> `LineManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h
- `lineManagerPersistsLastTestResults()` --calls--> `loadConfig`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h

## Import Cycles
- None detected.

## Communities (87 total, 7 thin omitted)

### Community 0 - "test_controller_tests.cpp"
Cohesion: 0.05
Nodes (113): QDate, QTime, QDateTime, QVector, QDateTime, QHash, LineOperationalMonitor, checkLine (+105 more)

### Community 1 - "PanelFacade"
Cohesion: 0.07
Nodes (80): Q_INVOKABLE, qint64, QJsonObject, QObject, QString, QStringList, QVariant, Q_OBJECT (+72 more)

### Community 2 - "LineManager"
Cohesion: 0.27
Nodes (15): QVector, LineManager, addLine, applyTestResults, defaultLinePoint, defaultLines, findLineIndex, line (+7 more)

### Community 3 - "toJson"
Cohesion: 0.12
Nodes (32): batteryStateCode(), batteryStateText(), BatteryState, CabinetMode, LineKind, LineOperationalState, LineOutputState, LineState (+24 more)

### Community 4 - "TestController"
Cohesion: 0.22
Nodes (19): QDateTime, QString, QVector, TestRunStatus, QVector, TestController, activeCandidate, activeTestDue (+11 more)

### Community 5 - "EngineRuntime::Impl"
Cohesion: 0.04
Nodes (55): QHttpServer, QLocalServer, QTimer, quint8, EngineRuntime::Impl, m_battery, m_cachedMaintenance, m_config (+47 more)

### Community 6 - "BatterySnapshot"
Cohesion: 0.05
Nodes (56): BatterySnapshot, alarmStatusRaw, balancingActive, cellCount, cellVoltageDelta, cellVoltages, chargeAllowed, communicationOk (+48 more)

### Community 7 - "ModbusRtuConfig"
Cohesion: 0.07
Nodes (43): AppConfig, load, m_logging, m_meteringRtu, m_modbusTcp, m_relayRtu, m_webServer, save (+35 more)

### Community 8 - "EngineInputs"
Cohesion: 0.07
Nodes (38): CabinetMode, QString, QStringList, QVector, SystemHealth, EngineInputs, activeTest, battery (+30 more)

### Community 9 - "Logger"
Cohesion: 0.11
Nodes (35): QMessageLogContext, QMutex, QtMsgType, Level, qint64, QString, Level, qint64 (+27 more)

### Community 10 - "MeteringBusController"
Cohesion: 0.06
Nodes (35): Q_OBJECT, QByteArray, QHash, QObject, QString, QTimer, QVector, MeteringBusController (+27 more)

### Community 11 - "metering"
Cohesion: 0.06
Nodes (32): logging, level, baudRate, busOfflineFailureThreshold, dataBits, parity, port, retries (+24 more)

### Community 12 - "ModbusController"
Cohesion: 0.06
Nodes (33): deque, Q_OBJECT, QObject, QTimer, QVector, ModbusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+25 more)

### Community 13 - "MeteringBusController.cpp"
Cohesion: 0.09
Nodes (30): DataBits, Parity, QObject, Request, StopBits, dataBitsFromConfig(), addAdl200InputMeterPolling, addAsj60Ld16aLeakagePolling (+22 more)

### Community 14 - "What You Must Do When Invoked"
Cohesion: 0.08
Nodes (24): For /graphify add and --watch, For /graphify query, For the commit hook and native CLAUDE.md integration, For --update and --cluster-only, /graphify, Honesty Rules, Interpreter guard for subcommands, Part A - Structural extraction for code files (+16 more)

### Community 15 - "CabinetSnapshot"
Cohesion: 0.08
Nodes (24): CabinetSnapshot, activeFaults, activeTest, battery, explanation, fireInputActive, health, inputCurrent (+16 more)

### Community 16 - "ModbusTcpServer"
Cohesion: 0.09
Nodes (23): Error, RegisterType, Q_OBJECT, QObject, QTimer, ModbusTcpServer, durationTestRequested, functionalTestRequested (+15 more)

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

### Community 24 - "update"
Cohesion: 0.16
Nodes (21): LineOutputState, LineState, QString, QVector, SystemHealth, faultReason(), findLine(), QString (+13 more)

### Community 25 - "MaintenanceSnapshot"
Cohesion: 0.07
Nodes (33): QString, TestRunStatus, LineTestResult, completedAt, details, measuredPower, nominalPower, status (+25 more)

### Community 26 - "LineManager.cpp"
Cohesion: 0.33
Nodes (14): LineKind, QJsonObject, QString, ioMapFromJson(), ioMapToJson(), lineConfigFromJson(), lineKindFromConfig(), lineKindToConfig() (+6 more)

### Community 27 - ".processIpcMessage"
Cohesion: 0.27
Nodes (8): QLocalSocket, QByteArray, QJsonObject, defaultLinesConfigPath(), errorResponse(), okResponse(), scheduleEntryFromHmi(), scheduleEntryToHmi()

### Community 28 - "EngineRuntime.cpp"
Cohesion: 0.06
Nodes (45): Impl, QFile, QSettings, QDateTime, QStringList, QVector, QByteArray, QHash (+37 more)

### Community 29 - "ModbusTcpServer.cpp"
Cohesion: 0.30
Nodes (16): quint32, batteryState(), cabinetState(), QDateTime, quint16, dateTimeToU32(), emergencyState(), highWord() (+8 more)

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

### Community 34 - "LineConfig"
Cohesion: 0.13
Nodes (15): LineKind, QString, LineConfig, enabled, index, kind, lastDurationTest, lastFunctionalTest (+7 more)

### Community 35 - "TelemetryTestAccess"
Cohesion: 0.18
Nodes (6): QString, handleRequestFailure, requestKey, updateBusMonitorFailure, QByteArray, TelemetryTestAccess

### Community 36 - "BMS батареи: заметки по протоколу"
Cohesion: 0.17
Nodes (11): BMS батареи: заметки по протоколу, Данные из `0x03`, Данные из `0x04`, Защиты, Интерфейс, Кадр, Команды для оперативного снимка, Предупреждения (+3 more)

### Community 37 - "Modbus RTU"
Cohesion: 0.17
Nodes (11): MeteringBusController для измерений, Modbus RTU, ModbusController для реле, U, I и знак мощности линий AMC16Z-FAK24, Входной измеритель ADL200, Датчик температуры и влажности WHD, Измеритель линий AMC16Z-FAK24, Измеритель утечки ASJ60-LD16A/C (+3 more)

### Community 38 - "TestJournalEntry"
Cohesion: 0.09
Nodes (31): TestJournalEntry, finishedAt, kind, lines, reason, source, startedAt, status (+23 more)

### Community 39 - "QString"
Cohesion: 0.17
Nodes (7): QObject, QString, QStringList, defaultLogPath(), defaultTestJournalPath(), defaultTestSchedulePath(), EngineRuntime::EngineRuntime()

### Community 40 - "CabinetSnapshot.cpp"
Cohesion: 0.46
Nodes (14): activeTestFromJson(), batteryFromJson(), QJsonObject, dateTimeFromJson(), doubleOrNaN(), intOrUnknown(), lineFromJson(), lineOperationalCheckFromJson() (+6 more)

### Community 41 - ".start"
Cohesion: 0.18
Nodes (3): qint64, defaultRuntimeTimingPath(), ipcServerName()

### Community 42 - "WaveSharePoint"
Cohesion: 0.27
Nodes (10): QHash, quint8, evaluate, inputActive, pointIsUsed, requiredModuleCount, setRelayBit, WaveSharePoint (+2 more)

### Community 43 - "ActiveTestSnapshot"
Cohesion: 0.20
Nodes (10): ActiveTestSnapshot, active, dueAt, durationSeconds, kind, source, startedAt, warmupSeconds (+2 more)

### Community 44 - "ManualEmergencyController"
Cohesion: 0.29
Nodes (8): ManualEmergencyController, active, evaluate, m_active, reset, ManualEmergencyInputs, startRequested, stopRequested

### Community 45 - "TestControllerResult"
Cohesion: 0.18
Nodes (11): TestControllerResult, activeTest, journalEntries, lines, manualRequestConsumed, manualTestActive, modeRelayOn, newJournalEntries (+3 more)

### Community 46 - "CabinetIoMap"
Cohesion: 0.20
Nodes (10): CabinetIoMap, faultLampRelay, fireInput, manualFireButton, manualStopButton, modeRelay, reserveRelay, testLampRelay (+2 more)

### Community 47 - "TestScheduleRequest"
Cohesion: 0.17
Nodes (12): TestRequest, active, durationSeconds, QDateTime, QString, TestScheduleRequest, duration, entryIndex (+4 more)

### Community 48 - "graphify reference: extra exports and benchmark"
Cohesion: 0.22
Nodes (8): graphify reference: extra exports and benchmark, Step 6b - Wiki (only if --wiki flag), Step 7 - Neo4j export (only if --neo4j or --neo4j-push flag), Step 7a - FalkorDB export (only if --falkordb or --falkordb-push flag), Step 7b - SVG export (only if --svg flag), Step 7c - GraphML export (only if --graphml flag), Step 7d - MCP server (only if --mcp flag), Step 8 - Token reduction benchmark (only if total_words > 5000)

### Community 49 - "Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический, Source Nodes

### Community 50 - "setupDevice"
Cohesion: 0.22
Nodes (10): DataBits, QObject, StopBits, dataBitsFromConfig(), configure, connectDevice, ModbusController::ModbusController(), recreateClient (+2 more)

### Community 51 - "handleCurrentResponse"
Cohesion: 0.24
Nodes (9): byteAt(), QByteArray, quint8, amc16zBranchCurrentsUpdated, amc16zBranchVoltagesUpdated, expectedResponseSize, handleCurrentResponse, onReadyRead (+1 more)

### Community 52 - "measureLine"
Cohesion: 0.50
Nodes (4): TestKind, TestSource, measureLine, priority

### Community 53 - "TestController"
Cohesion: 0.25
Nodes (7): TestController, Остановка оператором, Приоритеты, Расписание, Связь с LineManager, Типы тестов, Хранение

### Community 54 - "numbersToJson"
Cohesion: 0.32
Nodes (8): QJsonArray, QStringList, QVector, numberOrNull(), numbersFromJson(), numbersToJson(), stringsFromJson(), stringsToJson()

### Community 55 - "TestControllerInputs"
Cohesion: 0.18
Nodes (11): QDateTime, TestControllerInputs, fireInputActive, lines, manualDuration, manualFunctional, now, scheduledDuration (+3 more)

### Community 56 - "Q: Всё исправно, но по линиям показывает нет данных"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Всё исправно, но по линиям показывает нет данных, Source Nodes

### Community 57 - ".tick"
Cohesion: 0.25
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
Cohesion: 0.33
Nodes (5): Изменения состояния, Логирование, Ротация, Уровни, Файл

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

### Community 68 - "LineManagerResult"
Cohesion: 0.20
Nodes (10): QStringList, QVector, LineManagerResult, faults, fireInputActive, lines, manualFireButtonActive, manualStopButtonActive (+2 more)

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

### Community 80 - "start"
Cohesion: 0.25
Nodes (8): QObject, QString, ModbusTcpServer::ModbusTcpServer(), refreshRegisters, setupServerMap, start, stop, updateSnapshot

### Community 81 - "TestControllerConfig"
Cohesion: 0.40
Nodes (5): TestController::TestController(), TestControllerConfig, defaultDurationSeconds, durationToleranceMultiplier, functionalWarmupSeconds

### Community 82 - "Q: Мы не потеряли логику авария и неисправность?"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Мы не потеряли логику авария и неисправность?, Source Nodes

### Community 83 - "LineOperationalCheck"
Cohesion: 0.22
Nodes (9): LineOperationalState, LineOperationalCheck, details, measuredPower, nominalPower, startedAt, state, tolerancePercent (+1 more)

### Community 86 - "dateTimeOrNull"
Cohesion: 0.50
Nodes (5): QDateTime, QJsonValue, dateTimeOrNull(), dateTimeToJson(), intOrNull()

### Community 87 - "web_ui_tests.js"
Cohesion: 0.16
Nodes (13): context, element(), elements, expect(), fs, html, intervals, listeners (+5 more)

### Community 89 - "Candidate"
Cohesion: 0.25
Nodes (8): Candidate, durationSeconds, kind, priority, source, valid, TestKind, TestSource

## Knowledge Gaps
- **596 isolated node(s):** `schemaVersion`, `port`, `baudRate`, `parity`, `dataBits` (+591 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 737 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **7 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Work-memory lessons

**Preferred sources** — corroborated by past sessions; start here.
- `PanelFacade` (3× useful, score=2.238326053) _(code changed — re-verify)_
- `StateEngine` (2× useful, score=1.803156794)
- `ModbusTcpServer` (2× useful, score=1.630258357)
- `EngineRuntime` (2× useful, score=1.630258357)
- `EngineRuntime::Impl` (2× useful, score=1.592188965) _(code changed — re-verify)_
- `MeteringBusController` (2× useful, score=1.434742437)
- `CabinetSnapshot` (2× useful, score=1.418816886) _(code changed — re-verify)_

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `EngineRuntime::Impl` connect `EngineRuntime::Impl` to `test_controller_tests.cpp`, `LineManager`, `toJson`, `TestController`, `BatterySnapshot`, `ModbusRtuConfig`, `EngineInputs`, `MeteringBusController`, `ModbusController`, `CabinetSnapshot`, `ModbusTcpServer`, `ModbusBusMonitor`, `update`, `MaintenanceSnapshot`, `.processIpcMessage`, `EngineRuntime.cpp`, `.setupWebRoutes`, `TestJournalEntry`, `QString`, `.start`, `ManualEmergencyController`, `TestScheduleRequest`, `.tick`, `WaveShareModuleState`?**
  _High betweenness centrality (0.265) - this node is a cross-community bridge._
- **Why does `MeteringBusController` connect `MeteringBusController` to `Request`, `PanelFacade`, `TelemetryTestAccess`, `EngineRuntime::Impl`, `BatterySnapshot`, `ModbusRtuConfig`, `MeteringBusController.cpp`, `handleCurrentResponse`, `ModbusBusMonitor`, `EngineRuntime.cpp`?**
  _High betweenness centrality (0.099) - this node is a cross-community bridge._
- **Why does `ModbusController` connect `ModbusController` to `PanelFacade`, `handleRequestFailure`, `EngineRuntime::Impl`, `ModbusRtuConfig`, `ModbusController.cpp`, `setupDevice`, `ModbusBusMonitor`, `Request`, `EngineRuntime.cpp`, `handleRequestSuccess`?**
  _High betweenness centrality (0.092) - this node is a cross-community bridge._
- **What connects `schemaVersion`, `port`, `baudRate` to the rest of the system?**
  _596 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `test_controller_tests.cpp` be split into smaller, more focused modules?**
  _Cohesion score 0.05052473763118441 - nodes in this community are weakly interconnected._
- **Should `PanelFacade` be split into smaller, more focused modules?**
  _Cohesion score 0.06526610644257703 - nodes in this community are weakly interconnected._
- **Should `toJson` be split into smaller, more focused modules?**
  _Cohesion score 0.12121212121212122 - nodes in this community are weakly interconnected._