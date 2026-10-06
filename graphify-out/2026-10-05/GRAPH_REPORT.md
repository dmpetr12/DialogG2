# Graph Report - DialogG2  (2026-10-05)

## Corpus Check
- 125 files · ~97,621 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 58 file(s) not represented in the graph (top: .log 41, .qml 14, (none) 2)

## Summary
- 1513 nodes · 3016 edges · 90 communities (80 shown, 10 thin omitted)
- Extraction: 91% EXTRACTED · 9% INFERRED · 0% AMBIGUOUS · INFERRED: 274 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `bc537073`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- test_controller_tests.cpp
- PanelFacade
- LineManager.cpp
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
- CabinetIoMap
- Amc16zFak24Meter
- decodeChannelHoldingRegisters
- ModbusController.h
- Request
- QVector
- MaintenanceSnapshot
- LineConfig
- EngineRuntime.cpp
- CabinetSnapshot.h
- ModbusTcpServer.cpp
- handleRequestSuccess
- ModbusRtuCodec.cpp
- .setupWebRoutes
- Request
- TestControllerResult
- TelemetryTestAccess
- BMS батареи: заметки по протоколу
- Modbus RTU
- TestJournalStore
- QString
- WaveSharePoint
- .start
- Candidate
- ActiveTestSnapshot
- ManualEmergencyController
- LineOperationalMonitor
- LineTestResult
- TestControllerInputs
- graphify reference: extra exports and benchmark
- Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический
- TestJournalEntry
- MeteringBusController.h
- TestLineMeasurement
- TestController
- MaintenanceLineStatus
- parseResponse
- Q: Всё исправно, но по линиям показывает нет данных
- .tick
- LineManager
- Dialog G2 Panel
- LineManagerResult
- graphify reference: query, path, explain
- Логирование
- MODBUS TCP
- Файл состояния
- Q: Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями.
- Q: Реле общей неисправности должно включаться при аварии входного напряжения
- handleRequestFailure
- handleCurrentResponse
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
- QString
- finishActiveTest
- Q: Мы не потеряли логику авария и неисправность?
- QDateTime
- StateFileStore
- onDataWritten
- onErrorOccurred
- web_ui_tests.js
- handleRequestFailure
- .Impl

## God Nodes (most connected - your core abstractions)
1. `EngineRuntime::Impl` - 122 edges
2. `MeteringBusController` - 87 edges
3. `ModbusController` - 77 edges
4. `PanelFacade` - 71 edges
5. `CabinetSnapshot` - 55 edges
6. `expect()` - 52 edges
7. `main()` - 51 edges
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

## Communities (90 total, 10 thin omitted)

### Community 0 - "test_controller_tests.cpp"
Cohesion: 0.05
Nodes (112): QDate, QTime, QVector, evaluate, QDateTime, evaluate, evaluate, QDateTime (+104 more)

### Community 1 - "PanelFacade"
Cohesion: 0.07
Nodes (80): Q_INVOKABLE, qint64, QJsonObject, QObject, QString, QStringList, QVariant, Q_OBJECT (+72 more)

### Community 2 - "LineManager.cpp"
Cohesion: 0.27
Nodes (18): QVector, LineManager, addLine, applyTestResults, defaultIoMap, defaultLinePoint, defaultLines, findLineIndex (+10 more)

### Community 3 - "toJson"
Cohesion: 0.09
Nodes (59): activeTestFromJson(), batteryFromJson(), batteryStateCode(), batteryStateText(), BatteryState, CabinetMode, LineKind, LineOperationalState (+51 more)

### Community 4 - "TestController"
Cohesion: 0.14
Nodes (21): QDateTime, TestKind, TestSource, QVector, TestController, activeTestDue, hasActiveTest, highestRequestedTest (+13 more)

### Community 5 - "EngineRuntime::Impl"
Cohesion: 0.04
Nodes (53): QTimer, quint8, EngineRuntime::Impl, m_battery, m_cachedMaintenance, m_config, m_demoMode, m_hmiManualEmergencyStartRequested (+45 more)

### Community 6 - "BatterySnapshot"
Cohesion: 0.05
Nodes (42): BatterySnapshot, alarmStatusRaw, balancingActive, cellCount, cellVoltageDelta, cellVoltages, chargeAllowed, communicationOk (+34 more)

### Community 7 - "ModbusRtuConfig"
Cohesion: 0.08
Nodes (40): AppConfig, load, m_logging, m_meteringRtu, m_modbusTcp, m_relayRtu, m_webServer, save (+32 more)

### Community 8 - "EngineInputs"
Cohesion: 0.07
Nodes (38): CabinetMode, QString, QStringList, QVector, SystemHealth, EngineInputs, activeTest, battery (+30 more)

### Community 9 - "Logger"
Cohesion: 0.06
Nodes (49): Impl, QFile, QMessageLogContext, QMutex, QSettings, QtMsgType, Level, qint64 (+41 more)

### Community 10 - "MeteringBusController"
Cohesion: 0.06
Nodes (35): Q_OBJECT, QByteArray, QHash, QObject, QString, QTimer, QVector, MeteringBusController (+27 more)

### Community 11 - "metering"
Cohesion: 0.06
Nodes (32): logging, level, baudRate, busOfflineFailureThreshold, dataBits, parity, port, retries (+24 more)

### Community 12 - "ModbusController"
Cohesion: 0.06
Nodes (32): deque, Q_OBJECT, QObject, QTimer, QVector, ModbusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+24 more)

### Community 13 - "MeteringBusController.cpp"
Cohesion: 0.10
Nodes (26): DataBits, Parity, QObject, StopBits, dataBitsFromConfig(), addAdl200InputMeterPolling, addAsj60Ld16aLeakagePolling, addJbdBmsPolling (+18 more)

### Community 14 - "What You Must Do When Invoked"
Cohesion: 0.08
Nodes (24): For /graphify add and --watch, For /graphify query, For the commit hook and native CLAUDE.md integration, For --update and --cluster-only, /graphify, Honesty Rules, Interpreter guard for subcommands, Part A - Structural extraction for code files (+16 more)

### Community 15 - "CabinetSnapshot"
Cohesion: 0.08
Nodes (24): CabinetSnapshot, activeFaults, activeTest, battery, explanation, fireInputActive, health, inputCurrent (+16 more)

### Community 16 - "ModbusTcpServer"
Cohesion: 0.09
Nodes (25): QObject, QString, Q_OBJECT, QObject, QTimer, ModbusTcpServer, durationTestRequested, functionalTestRequested (+17 more)

### Community 17 - "LineSnapshot"
Cohesion: 0.10
Nodes (21): LineKind, LineOutputState, LineState, LineSnapshot, enabled, index, kind, lastDurationTest (+13 more)

### Community 18 - "ModbusController.cpp"
Cohesion: 0.08
Nodes (30): DataBits, QObject, StopBits, dataBitsFromConfig(), addAdl200InputMeterPolling, addAmc16zFak24BranchPowerPolling, addAsj60Ld16aLeakagePolling, addHoldingRegistersPolling (+22 more)

### Community 19 - "CabinetIoMap"
Cohesion: 0.16
Nodes (17): CabinetIoMap, faultLampRelay, fireInput, manualFireButton, manualStopButton, modeRelay, reserveRelay, testLampRelay (+9 more)

### Community 20 - "Amc16zFak24Meter"
Cohesion: 0.15
Nodes (17): Amc16zBranchMeasurement, activePower, channel, valid, Amc16zFak24Meter, ActivePowerBranchCount, ActivePowerHoldingCount, ActivePowerHoldingStart (+9 more)

### Community 21 - "decodeChannelHoldingRegisters"
Cohesion: 0.13
Nodes (17): Asj60Ld16aMonitor, ChannelCount, ChannelDataHoldingCount, ChannelDataHoldingStart, decodeChannelHoldingRegisters, DefaultSlaveAddress, RegistersPerChannel, Asj60LeakageChannel (+9 more)

### Community 22 - "ModbusController.h"
Cohesion: 0.13
Nodes (19): deque, QString, QString, ModbusBusMonitor, m_offlineFailureThreshold, m_status, markFailure, markSuccess (+11 more)

### Community 23 - "Request"
Cohesion: 0.11
Nodes (18): MeterKind, qint64, quint8, RequestPriority, RequestType, PollTask, intervalMs, nextDueMsec (+10 more)

### Community 24 - "QVector"
Cohesion: 0.05
Nodes (52): Adl200Measurement, activePower, apparentPower, current, frequency, powerFactor, reactivePower, valid (+44 more)

### Community 25 - "MaintenanceSnapshot"
Cohesion: 0.20
Nodes (10): MaintenanceSnapshot, lastLongTestAt, lastLongTestStatus, lineLimitDays, lines, longTestLimitDays, longTestOverdue, ok (+2 more)

### Community 26 - "LineConfig"
Cohesion: 0.14
Nodes (14): LineKind, QString, LineConfig, enabled, index, kind, lastDurationTest, lastFunctionalTest (+6 more)

### Community 27 - "EngineRuntime.cpp"
Cohesion: 0.18
Nodes (14): QHttpServer, QLocalServer, QLocalSocket, applyHmiLineMode(), LineKind, QByteArray, QJsonObject, defaultLinesConfigPath() (+6 more)

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

### Community 34 - "TestControllerResult"
Cohesion: 0.18
Nodes (11): TestControllerResult, activeTest, journalEntries, lines, manualRequestConsumed, manualTestActive, modeRelayOn, newJournalEntries (+3 more)

### Community 35 - "TelemetryTestAccess"
Cohesion: 0.18
Nodes (6): busStatus, onReadyRead, processReceiveBuffer, QByteArray, QJsonValue, TelemetryTestAccess

### Community 36 - "BMS батареи: заметки по протоколу"
Cohesion: 0.17
Nodes (11): BMS батареи: заметки по протоколу, Данные из `0x03`, Данные из `0x04`, Защиты, Интерфейс, Кадр, Команды для оперативного снимка, Предупреждения (+3 more)

### Community 37 - "Modbus RTU"
Cohesion: 0.17
Nodes (11): MeteringBusController для измерений, Modbus RTU, ModbusController для реле, U, I и знак мощности линий AMC16Z-FAK24, Входной измеритель ADL200, Датчик температуры и влажности WHD, Измеритель линий AMC16Z-FAK24, Измеритель утечки ASJ60-LD16A/C (+3 more)

### Community 38 - "TestJournalStore"
Cohesion: 0.36
Nodes (9): QString, QVector, QString, TestJournalStore, append, m_filePath, read, TestJournalStore::TestJournalStore() (+1 more)

### Community 39 - "QString"
Cohesion: 0.24
Nodes (8): main(), QObject, QString, QStringList, defaultAppConfigPath(), defaultLogPath(), defaultStatePath(), EngineRuntime::EngineRuntime()

### Community 40 - "WaveSharePoint"
Cohesion: 0.33
Nodes (8): QHash, quint8, evaluate, inputActive, setRelayBit, WaveSharePoint, channel, module

### Community 41 - ".start"
Cohesion: 0.17
Nodes (4): qint64, defaultRuntimeTimingPath(), start, ipcServerName()

### Community 42 - "Candidate"
Cohesion: 0.22
Nodes (9): Candidate, durationSeconds, kind, priority, source, valid, TestKind, TestSource (+1 more)

### Community 43 - "ActiveTestSnapshot"
Cohesion: 0.20
Nodes (10): ActiveTestSnapshot, active, dueAt, durationSeconds, kind, source, startedAt, warmupSeconds (+2 more)

### Community 44 - "ManualEmergencyController"
Cohesion: 0.29
Nodes (8): ManualEmergencyController, active, evaluate, m_active, reset, ManualEmergencyInputs, startRequested, stopRequested

### Community 45 - "LineOperationalMonitor"
Cohesion: 0.19
Nodes (12): QDateTime, QDateTime, QHash, LineOperationalMonitor, checkLine, LineOperationalMonitor::LineOperationalMonitor(), m_config, m_onSince (+4 more)

### Community 46 - "LineTestResult"
Cohesion: 0.25
Nodes (8): TestRunStatus, LineTestResult, completedAt, details, measuredPower, nominalPower, status, tolerancePercent

### Community 47 - "TestControllerInputs"
Cohesion: 0.14
Nodes (14): QDateTime, TestControllerInputs, fireInputActive, lines, manualDuration, manualFunctional, now, scheduledDuration (+6 more)

### Community 48 - "graphify reference: extra exports and benchmark"
Cohesion: 0.22
Nodes (8): graphify reference: extra exports and benchmark, Step 6b - Wiki (only if --wiki flag), Step 7 - Neo4j export (only if --neo4j or --neo4j-push flag), Step 7a - FalkorDB export (only if --falkordb or --falkordb-push flag), Step 7b - SVG export (only if --svg flag), Step 7c - GraphML export (only if --graphml flag), Step 7d - MCP server (only if --mcp flag), Step 8 - Token reduction benchmark (only if total_words > 5000)

### Community 49 - "Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический, Source Nodes

### Community 50 - "TestJournalEntry"
Cohesion: 0.25
Nodes (8): TestJournalEntry, finishedAt, kind, lines, reason, source, startedAt, status

### Community 51 - "MeteringBusController.h"
Cohesion: 0.20
Nodes (9): QElapsedTimer, QByteArray, JbdBmsResponse, callbackId, command, data, status, QSerialPort (+1 more)

### Community 52 - "TestLineMeasurement"
Cohesion: 0.25
Nodes (8): TestLineMeasurement, details, lineIndex, lineName, measuredPower, nominalPower, status, tolerancePercent

### Community 53 - "TestController"
Cohesion: 0.25
Nodes (7): TestController, Остановка оператором, Приоритеты, Расписание, Связь с LineManager, Типы тестов, Хранение

### Community 54 - "MaintenanceLineStatus"
Cohesion: 0.29
Nodes (7): QString, MaintenanceLineStatus, lastTestAt, lastTestStatus, lineIndex, lineName, overdue

### Community 55 - "parseResponse"
Cohesion: 0.19
Nodes (22): byteAt(), QByteArray, qint16, QString, QStringList, quint16, quint8, quint8 (+14 more)

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

### Community 60 - "LineManagerResult"
Cohesion: 0.09
Nodes (24): QHash, QStringList, quint8, QVector, LineManagerInputs, faultLampOn, forceLineIndex, forceLinesOn (+16 more)

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

### Community 68 - "handleCurrentResponse"
Cohesion: 0.33
Nodes (7): byteAt(), QByteArray, quint8, amc16zBranchCurrentsUpdated, amc16zBranchVoltagesUpdated, expectedResponseSize, handleCurrentResponse

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

### Community 80 - "QString"
Cohesion: 0.47
Nodes (6): LineKind, QString, lineKindFromConfig(), lineKindToConfig(), saveConfig, saveDefaultConfig

### Community 81 - "finishActiveTest"
Cohesion: 0.40
Nodes (5): QString, QVector, TestRunStatus, finishActiveTest, interruptionReason

### Community 82 - "Q: Мы не потеряли логику авария и неисправность?"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Мы не потеряли логику авария и неисправность?, Source Nodes

### Community 83 - "QDateTime"
Cohesion: 0.20
Nodes (10): LineOperationalState, QDateTime, LineOperationalCheck, details, measuredPower, nominalPower, startedAt, state (+2 more)

### Community 84 - "StateFileStore"
Cohesion: 0.36
Nodes (7): QString, QString, StateFileStore, m_filePath, read, StateFileStore::StateFileStore(), write

### Community 85 - "onDataWritten"
Cohesion: 0.50
Nodes (4): RegisterType, onDataWritten, processWrittenCoil, resetCoil

### Community 87 - "web_ui_tests.js"
Cohesion: 0.16
Nodes (13): context, element(), elements, expect(), fs, html, intervals, listeners (+5 more)

### Community 88 - "handleRequestFailure"
Cohesion: 0.21
Nodes (8): QString, Request, enqueue, handleRequestFailure, invalidateRequest, requestKey, sameRequest, updateBusMonitorFailure

## Knowledge Gaps
- **598 isolated node(s):** `schemaVersion`, `port`, `baudRate`, `parity`, `dataBits` (+593 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 739 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **10 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Work-memory lessons

**Preferred sources** — corroborated by past sessions; start here.
- `PanelFacade` (3× useful, score=2.142643791)
- `StateEngine` (2× useful, score=1.726076816)
- `ModbusTcpServer` (2× useful, score=1.56056931)
- `EngineRuntime` (2× useful, score=1.56056931)
- `EngineRuntime::Impl` (2× useful, score=1.52412728)
- `MeteringBusController` (2× useful, score=1.373411157) _(code changed — re-verify)_
- `CabinetSnapshot` (2× useful, score=1.358166379)

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `EngineRuntime::Impl` connect `EngineRuntime::Impl` to `test_controller_tests.cpp`, `LineManager.cpp`, `TestController`, `BatterySnapshot`, `ModbusRtuConfig`, `EngineInputs`, `Logger`, `MeteringBusController`, `ModbusController`, `CabinetSnapshot`, `ModbusTcpServer`, `ModbusController.h`, `QVector`, `MaintenanceSnapshot`, `EngineRuntime.cpp`, `.setupWebRoutes`, `TestJournalStore`, `QString`, `.start`, `ManualEmergencyController`, `LineOperationalMonitor`, `TestControllerInputs`, `TestJournalEntry`, `.tick`, `LineManagerResult`, `StateFileStore`, `.Impl`?**
  _High betweenness centrality (0.271) - this node is a cross-community bridge._
- **Why does `MeteringBusController` connect `MeteringBusController` to `Request`, `PanelFacade`, `TelemetryTestAccess`, `handleCurrentResponse`, `EngineRuntime::Impl`, `BatterySnapshot`, `ModbusRtuConfig`, `MeteringBusController.cpp`, `MeteringBusController.h`, `ModbusController.h`, `handleRequestFailure`?**
  _High betweenness centrality (0.095) - this node is a cross-community bridge._
- **Why does `ModbusController` connect `ModbusController` to `PanelFacade`, `handleRequestFailure`, `TelemetryTestAccess`, `EngineRuntime::Impl`, `ModbusRtuConfig`, `ModbusController.cpp`, `ModbusController.h`, `Request`, `handleRequestSuccess`?**
  _High betweenness centrality (0.090) - this node is a cross-community bridge._
- **What connects `schemaVersion`, `port`, `baudRate` to the rest of the system?**
  _598 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `test_controller_tests.cpp` be split into smaller, more focused modules?**
  _Cohesion score 0.05186880244088482 - nodes in this community are weakly interconnected._
- **Should `PanelFacade` be split into smaller, more focused modules?**
  _Cohesion score 0.06712564543889846 - nodes in this community are weakly interconnected._
- **Should `toJson` be split into smaller, more focused modules?**
  _Cohesion score 0.09322033898305085 - nodes in this community are weakly interconnected._