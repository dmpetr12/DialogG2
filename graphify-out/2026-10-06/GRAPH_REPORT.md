# Graph Report - DialogG2  (2026-10-05)

## Corpus Check
- 125 files · ~98,035 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 58 file(s) not represented in the graph (top: .log 41, .qml 14, (none) 2)

## Summary
- 1518 nodes · 3035 edges · 92 communities (83 shown, 9 thin omitted)
- Extraction: 91% EXTRACTED · 9% INFERRED · 0% AMBIGUOUS · INFERRED: 277 edges (avg confidence: 0.83)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `e5d8861d`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- TestScheduleManager
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
- test_controller_tests.cpp
- Amc16zFak24Meter
- decodeChannelHoldingRegisters
- ModbusBusMonitor
- Request
- Adl200Measurement
- MaintenanceSnapshot
- evaluate
- .processIpcMessage
- CabinetSnapshot.h
- EngineRuntime.cpp
- handleRequestSuccess
- ModbusRtuCodec.cpp
- .setupWebRoutes
- Request
- TestControllerResult
- TelemetryTestAccess
- BMS батареи: заметки по протоколу
- Modbus RTU
- TestJournalEntry
- QString
- update
- QFile
- Candidate
- ActiveTestSnapshot
- ManualEmergencyController
- LineOperationalMonitor
- decodeChannel1RealtimeRegisters
- TestControllerInputs
- graphify reference: extra exports and benchmark
- Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический
- testLine
- JbdBmsProtocol
- TestLineMeasurement
- TestController
- PasswordManager
- parseResponse
- Q: Всё исправно, но по линиям показывает нет данных
- .start
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
- LineManagerResult
- setupDevice
- Q: Мы не потеряли логику авария и неисправность?
- LineOperationalCheck
- StateFileStore
- testScheduleStartsWeekdayDuration
- handleRequestFailure
- web_ui_tests.js
- invalidateRequest
- StateLogEvent
- StateChangeTracker
- lineModeToHmi

## God Nodes (most connected - your core abstractions)
1. `EngineRuntime::Impl` - 124 edges
2. `MeteringBusController` - 87 edges
3. `ModbusController` - 77 edges
4. `PanelFacade` - 71 edges
5. `CabinetSnapshot` - 55 edges
6. `expect()` - 53 edges
7. `main()` - 52 edges
8. `LineSnapshot` - 47 edges
9. `toJson()` - 46 edges
10. `BatterySnapshot` - 40 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `QVariantList`  [INFERRED]
  tests/battery_page_tests.cpp → src/PanelFacade.h
- `insulationBreakdownIsPublishedForRemoteMonitoring()` --calls--> `toJson()`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/CabinetSnapshot.cpp
- `interruptedDurationTestRecoversFromHeartbeat()` --calls--> `toJson()`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/CabinetSnapshot.cpp
- `maintenanceCheckerReportsFailedTests()` --calls--> `toJson()`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/CabinetSnapshot.cpp
- `journalStorePersistsEntries()` --references--> `LineManager`  [INFERRED]
  tests/test_controller_tests.cpp → src/engine/LineManager.h

## Import Cycles
- None detected.

## Communities (92 total, 9 thin omitted)

### Community 0 - "TestScheduleManager"
Cohesion: 0.09
Nodes (43): QDateTime, QJsonObject, QString, QVariant, QVector, TestKind, entryFromJson(), entryToJson() (+35 more)

### Community 1 - "PanelFacade"
Cohesion: 0.07
Nodes (79): Q_INVOKABLE, qint64, QJsonObject, QObject, QString, QStringList, QVariant, Q_OBJECT (+71 more)

### Community 2 - "LineManager.cpp"
Cohesion: 0.07
Nodes (64): CabinetIoMap, faultLampRelay, fireInput, manualFireButton, manualStopButton, modeRelay, reserveRelay, testLampRelay (+56 more)

### Community 3 - "toJson"
Cohesion: 0.09
Nodes (59): activeTestFromJson(), batteryFromJson(), batteryStateCode(), batteryStateText(), BatteryState, CabinetMode, LineKind, LineOperationalState (+51 more)

### Community 4 - "TestController"
Cohesion: 0.14
Nodes (23): QDateTime, QJsonObject, QString, QVector, TestKind, TestRunStatus, TestSource, QVector (+15 more)

### Community 5 - "EngineRuntime::Impl"
Cohesion: 0.04
Nodes (57): QHttpServer, QLocalServer, QTimer, quint8, EngineRuntime::Impl, m_battery, m_cachedMaintenance, m_config (+49 more)

### Community 6 - "BatterySnapshot"
Cohesion: 0.07
Nodes (29): BatterySnapshot, alarmStatusRaw, balancingActive, cellCount, cellVoltageDelta, cellVoltages, chargeAllowed, communicationOk (+21 more)

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
Nodes (35): QElapsedTimer, Q_OBJECT, QByteArray, QHash, QObject, QString, QTimer, QVector (+27 more)

### Community 11 - "metering"
Cohesion: 0.06
Nodes (32): logging, level, baudRate, busOfflineFailureThreshold, dataBits, parity, port, retries (+24 more)

### Community 12 - "ModbusController"
Cohesion: 0.06
Nodes (33): deque, Q_OBJECT, QObject, QTimer, QVector, ModbusController, adl200InputMeterUpdated, amc16zFak24BranchPowersUpdated (+25 more)

### Community 13 - "MeteringBusController.cpp"
Cohesion: 0.11
Nodes (23): DataBits, Parity, StopBits, dataBitsFromConfig(), addAdl200InputMeterPolling, addAsj60Ld16aLeakagePolling, addJbdBmsPolling, addWhdTemperatureHumidityPolling (+15 more)

### Community 14 - "What You Must Do When Invoked"
Cohesion: 0.08
Nodes (24): For /graphify add and --watch, For /graphify query, For the commit hook and native CLAUDE.md integration, For --update and --cluster-only, /graphify, Honesty Rules, Interpreter guard for subcommands, Part A - Structural extraction for code files (+16 more)

### Community 15 - "CabinetSnapshot"
Cohesion: 0.08
Nodes (24): CabinetSnapshot, activeFaults, activeTest, battery, explanation, fireInputActive, health, inputCurrent (+16 more)

### Community 16 - "ModbusTcpServer"
Cohesion: 0.08
Nodes (47): Error, quint32, RegisterType, batteryState(), cabinetState(), QDateTime, QObject, QString (+39 more)

### Community 17 - "LineSnapshot"
Cohesion: 0.10
Nodes (21): LineKind, LineOutputState, LineState, LineSnapshot, enabled, index, kind, lastDurationTest (+13 more)

### Community 18 - "ModbusController.cpp"
Cohesion: 0.10
Nodes (20): addAdl200InputMeterPolling, addAmc16zFak24BranchPowerPolling, addAsj60Ld16aLeakagePolling, addHoldingRegistersPolling, addInputRegistersPolling, addWaveShareModulePolling, addWhdTemperatureHumidityPolling, clearPollTasks (+12 more)

### Community 19 - "test_controller_tests.cpp"
Cohesion: 0.15
Nodes (37): evaluate, adl200DecoderScalesRealtimeRegisters(), adl200UsesReservedSafeAddress(), amc16zFak24DecoderScalesBranchPowers(), amc16zRmsDecoderKeepsUnitsAndMissingData(), asj60Ld16aDecoderReadsChannelStatusesAndLeakage(), QString, quint16 (+29 more)

### Community 20 - "Amc16zFak24Meter"
Cohesion: 0.15
Nodes (17): Amc16zBranchMeasurement, activePower, channel, valid, Amc16zFak24Meter, ActivePowerBranchCount, ActivePowerHoldingCount, ActivePowerHoldingStart (+9 more)

### Community 21 - "decodeChannelHoldingRegisters"
Cohesion: 0.12
Nodes (17): Asj60Ld16aMonitor, ChannelCount, ChannelDataHoldingCount, ChannelDataHoldingStart, decodeChannelHoldingRegisters, DefaultSlaveAddress, RegistersPerChannel, Asj60LeakageChannel (+9 more)

### Community 22 - "ModbusBusMonitor"
Cohesion: 0.16
Nodes (16): QString, QString, ModbusBusMonitor, m_offlineFailureThreshold, m_status, markFailure, markSuccess, ModbusBusMonitor::ModbusBusMonitor() (+8 more)

### Community 23 - "Request"
Cohesion: 0.11
Nodes (18): MeterKind, qint64, quint8, RequestPriority, RequestType, PollTask, intervalMs, nextDueMsec (+10 more)

### Community 24 - "Adl200Measurement"
Cohesion: 0.12
Nodes (18): Adl200Measurement, activePower, apparentPower, current, frequency, powerFactor, reactivePower, valid (+10 more)

### Community 25 - "MaintenanceSnapshot"
Cohesion: 0.09
Nodes (25): QString, TestRunStatus, LineTestResult, completedAt, details, measuredPower, nominalPower, status (+17 more)

### Community 26 - "evaluate"
Cohesion: 0.15
Nodes (19): QDateTime, QString, QVector, TestRunStatus, MaintenanceChecker, evaluate, isCompletedTestStatus, latestCompletedDurationTest (+11 more)

### Community 27 - ".processIpcMessage"
Cohesion: 0.27
Nodes (8): QLocalSocket, QByteArray, QJsonObject, defaultLinesConfigPath(), errorResponse(), okResponse(), scheduleEntryFromHmi(), scheduleEntryToHmi()

### Community 29 - "EngineRuntime.cpp"
Cohesion: 0.26
Nodes (8): QDateTime, QStringList, QVector, QHash, deque, TelemetryTestAccess, TelemetryTestAccess, QTimer

### Community 30 - "handleRequestSuccess"
Cohesion: 0.22
Nodes (15): quint16, quint8, QVector, Request, RequestPriority, bitsFromReply, enqueue, finishRequest (+7 more)

### Community 31 - "ModbusRtuCodec.cpp"
Cohesion: 0.34
Nodes (13): byteAt(), QByteArray, quint16, quint8, QVector, ModbusRtuCodec, appendCrc, bitsFromReadResponse (+5 more)

### Community 32 - ".setupWebRoutes"
Cohesion: 0.20
Nodes (4): QHttpServerRequest, QHttpServerResponse, qint64, QJsonArray

### Community 33 - "Request"
Cohesion: 0.15
Nodes (13): MeterKind, qint64, RequestType, PollTask, intervalMs, nextDueMsec, request, Request (+5 more)

### Community 34 - "TestControllerResult"
Cohesion: 0.13
Nodes (19): QVector, evaluate, TestControllerResult, activeTest, journalEntries, lines, manualRequestConsumed, manualTestActive (+11 more)

### Community 35 - "TelemetryTestAccess"
Cohesion: 0.14
Nodes (8): addAmc16zFak24BranchPowerPolling, busStatus, clearPollTasks, onReadyRead, processReceiveBuffer, QByteArray, QJsonValue, TelemetryTestAccess

### Community 36 - "BMS батареи: заметки по протоколу"
Cohesion: 0.17
Nodes (11): BMS батареи: заметки по протоколу, Данные из `0x03`, Данные из `0x04`, Защиты, Интерфейс, Кадр, Команды для оперативного снимка, Предупреждения (+3 more)

### Community 37 - "Modbus RTU"
Cohesion: 0.17
Nodes (11): MeteringBusController для измерений, Modbus RTU, ModbusController для реле, U, I и знак мощности линий AMC16Z-FAK24, Входной измеритель ADL200, Датчик температуры и влажности WHD, Измеритель линий AMC16Z-FAK24, Измеритель утечки ASJ60-LD16A/C (+3 more)

### Community 38 - "TestJournalEntry"
Cohesion: 0.17
Nodes (17): TestJournalEntry, finishedAt, kind, lines, reason, source, startedAt, status (+9 more)

### Community 39 - "QString"
Cohesion: 0.15
Nodes (8): QObject, QString, QStringList, defaultLogPath(), defaultTestJournalPath(), defaultTestSchedulePath(), EngineRuntime::EngineRuntime(), ipcServerName()

### Community 40 - "update"
Cohesion: 0.28
Nodes (14): LineOutputState, LineState, QString, QVector, SystemHealth, faultReason(), findLine(), isOperationalLineState() (+6 more)

### Community 41 - "QFile"
Cohesion: 0.18
Nodes (8): Impl, QFile, EngineRuntime, m_impl, start, QObject, main(), unique_ptr

### Community 42 - "Candidate"
Cohesion: 0.14
Nodes (14): Candidate, durationSeconds, kind, priority, source, valid, TestKind, TestSource (+6 more)

### Community 43 - "ActiveTestSnapshot"
Cohesion: 0.20
Nodes (10): ActiveTestSnapshot, active, dueAt, durationSeconds, kind, source, startedAt, warmupSeconds (+2 more)

### Community 44 - "ManualEmergencyController"
Cohesion: 0.29
Nodes (8): ManualEmergencyController, active, evaluate, m_active, reset, ManualEmergencyInputs, startRequested, stopRequested

### Community 45 - "LineOperationalMonitor"
Cohesion: 0.18
Nodes (12): QDateTime, QDateTime, QHash, LineOperationalMonitor, checkLine, LineOperationalMonitor::LineOperationalMonitor(), m_config, m_onSince (+4 more)

### Community 46 - "decodeChannel1RealtimeRegisters"
Cohesion: 0.18
Nodes (12): quint16, QVector, signedTenths(), WhdMeasurement, humidity, temperature, valid, WhdTemperatureHumidityController (+4 more)

### Community 47 - "TestControllerInputs"
Cohesion: 0.14
Nodes (14): QDateTime, TestControllerInputs, fireInputActive, lines, manualDuration, manualFunctional, now, scheduledDuration (+6 more)

### Community 48 - "graphify reference: extra exports and benchmark"
Cohesion: 0.22
Nodes (8): graphify reference: extra exports and benchmark, Step 6b - Wiki (only if --wiki flag), Step 7 - Neo4j export (only if --neo4j or --neo4j-push flag), Step 7a - FalkorDB export (only if --falkordb or --falkordb-push flag), Step 7b - SVG export (only if --svg flag), Step 7c - GraphML export (only if --graphml flag), Step 7d - MCP server (only if --mcp flag), Step 8 - Token reduction benchmark (only if total_words > 5000)

### Community 49 - "Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: посмотри по модели, где берется и обрабатывается режим сигнала пожар электрический, Source Nodes

### Community 50 - "testLine"
Cohesion: 0.26
Nodes (12): evaluate, activeTestsRequestBatteryMode(), durationTestUsesExpandedTolerance(), functionalTestPasses(), functionalTestUsesRequestedWarmup(), lineDataAvailabilityDependsOnlyOnPower(), manualEmergencyControllerLatchesUntilStop(), manualTestBlockedByFireIsConsumed() (+4 more)

### Community 51 - "JbdBmsProtocol"
Cohesion: 0.18
Nodes (11): quint8, JbdBmsProtocol, CommandBasicInfo, CommandCellVoltages, CommandHardwareVersion, CommandProtectionCounters, JbdBmsResponse, callbackId (+3 more)

### Community 52 - "TestLineMeasurement"
Cohesion: 0.25
Nodes (8): TestLineMeasurement, details, lineIndex, lineName, measuredPower, nominalPower, status, tolerancePercent

### Community 53 - "TestController"
Cohesion: 0.25
Nodes (7): TestController, Остановка оператором, Приоритеты, Расписание, Связь с LineManager, Типы тестов, Хранение

### Community 54 - "PasswordManager"
Cohesion: 0.24
Nodes (7): QSettings, QObject, QString, PasswordManager, m_settings, passwordChanged, Q_PROPERTY

### Community 55 - "parseResponse"
Cohesion: 0.29
Nodes (16): byteAt(), QByteArray, qint16, QString, QStringList, quint16, quint8, i16At() (+8 more)

### Community 56 - "Q: Всё исправно, но по линиям показывает нет данных"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Всё исправно, но по линиям показывает нет данных, Source Nodes

### Community 57 - ".start"
Cohesion: 0.16
Nodes (5): QDateTime, QHash, QVector, defaultRuntimeTimingPath(), trimTestJournal()

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

### Community 68 - "handleCurrentResponse"
Cohesion: 0.33
Nodes (6): byteAt(), QByteArray, quint8, amc16zBranchCurrentsUpdated, amc16zBranchVoltagesUpdated, handleCurrentResponse

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

### Community 80 - "LineManagerResult"
Cohesion: 0.20
Nodes (10): QStringList, QVector, LineManagerResult, faults, fireInputActive, lines, manualFireButtonActive, manualStopButtonActive (+2 more)

### Community 81 - "setupDevice"
Cohesion: 0.22
Nodes (10): DataBits, QObject, StopBits, dataBitsFromConfig(), configure, connectDevice, ModbusController::ModbusController(), recreateClient (+2 more)

### Community 82 - "Q: Мы не потеряли логику авария и неисправность?"
Cohesion: 0.40
Nodes (4): Answer, Outcome, Q: Мы не потеряли логику авария и неисправность?, Source Nodes

### Community 83 - "LineOperationalCheck"
Cohesion: 0.22
Nodes (9): LineOperationalState, LineOperationalCheck, details, measuredPower, nominalPower, startedAt, state, tolerancePercent (+1 more)

### Community 84 - "StateFileStore"
Cohesion: 0.36
Nodes (7): QString, QString, StateFileStore, m_filePath, read, StateFileStore::StateFileStore(), write

### Community 85 - "testScheduleStartsWeekdayDuration"
Cohesion: 0.47
Nodes (6): QDate, QTime, QDateTime, testSchedulePersistsLegacyArrayFormat(), testScheduleStartsDailyFunctionalOnce(), testScheduleStartsWeekdayDuration()

### Community 86 - "handleRequestFailure"
Cohesion: 0.50
Nodes (4): QString, handleRequestFailure, requestKey, updateBusMonitorFailure

### Community 87 - "web_ui_tests.js"
Cohesion: 0.16
Nodes (13): context, element(), elements, expect(), fs, html, intervals, listeners (+5 more)

### Community 88 - "invalidateRequest"
Cohesion: 0.22
Nodes (8): QObject, Request, disconnectDevice, enqueue, invalidateMeasurements, invalidateRequest, MeteringBusController::MeteringBusController(), sameRequest

### Community 89 - "StateLogEvent"
Cohesion: 0.50
Nodes (4): QString, StateLogEvent, message, warning

### Community 90 - "StateChangeTracker"
Cohesion: 0.67
Nodes (3): StateChangeTracker, m_initialized, m_previous

## Knowledge Gaps
- **600 isolated node(s):** `schemaVersion`, `port`, `baudRate`, `parity`, `dataBits` (+595 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 741 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **9 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Work-memory lessons

**Preferred sources** — corroborated by past sessions; start here.
- `PanelFacade` (3× useful, score=2.142643791)
- `StateEngine` (2× useful, score=1.726076816)
- `ModbusTcpServer` (2× useful, score=1.56056931)
- `EngineRuntime` (2× useful, score=1.56056931)
- `EngineRuntime::Impl` (2× useful, score=1.52412728) _(code changed — re-verify)_
- `MeteringBusController` (2× useful, score=1.373411157) _(code changed — re-verify)_
- `CabinetSnapshot` (2× useful, score=1.358166379)

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `EngineRuntime::Impl` connect `EngineRuntime::Impl` to `TestScheduleManager`, `LineManager.cpp`, `toJson`, `TestController`, `BatterySnapshot`, `ModbusRtuConfig`, `EngineInputs`, `MeteringBusController`, `ModbusController`, `CabinetSnapshot`, `ModbusTcpServer`, `ModbusBusMonitor`, `MaintenanceSnapshot`, `evaluate`, `.processIpcMessage`, `EngineRuntime.cpp`, `.setupWebRoutes`, `TestJournalEntry`, `QString`, `ManualEmergencyController`, `LineOperationalMonitor`, `TestControllerInputs`, `PasswordManager`, `.start`, `WaveShareModuleState`, `StateFileStore`, `StateChangeTracker`?**
  _High betweenness centrality (0.266) - this node is a cross-community bridge._
- **Why does `ModbusController` connect `ModbusController` to `PanelFacade`, `handleRequestFailure`, `TelemetryTestAccess`, `EngineRuntime::Impl`, `ModbusRtuConfig`, `setupDevice`, `ModbusController.cpp`, `ModbusBusMonitor`, `Request`, `EngineRuntime.cpp`, `handleRequestSuccess`?**
  _High betweenness centrality (0.088) - this node is a cross-community bridge._
- **Why does `MeteringBusController` connect `MeteringBusController` to `Request`, `PanelFacade`, `TelemetryTestAccess`, `handleCurrentResponse`, `EngineRuntime::Impl`, `BatterySnapshot`, `ModbusRtuConfig`, `MeteringBusController.cpp`, `handleRequestFailure`, `ModbusBusMonitor`, `invalidateRequest`, `EngineRuntime.cpp`?**
  _High betweenness centrality (0.075) - this node is a cross-community bridge._
- **What connects `schemaVersion`, `port`, `baudRate` to the rest of the system?**
  _600 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `TestScheduleManager` be split into smaller, more focused modules?**
  _Cohesion score 0.09408033826638477 - nodes in this community are weakly interconnected._
- **Should `PanelFacade` be split into smaller, more focused modules?**
  _Cohesion score 0.06788128122245078 - nodes in this community are weakly interconnected._
- **Should `LineManager.cpp` be split into smaller, more focused modules?**
  _Cohesion score 0.0703962703962704 - nodes in this community are weakly interconnected._