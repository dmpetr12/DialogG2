---
type: "query"
date: "2026-09-13T20:40:20.749837+00:00"
question: "Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями."
contributor: "graphify"
outcome: "useful"
source_nodes: ["EngineRuntime", "PanelFacade", "CabinetSnapshot", "LineManager", "ModbusController", "MeteringBusController", "ModbusTcpServer"]
---

# Q: Используя knowledge graph Graphify, опиши архитектуру этого проекта и основные связи между QML, backend и моделями.

## Answer

Expanded via graph vocab: engine, runtime, panel, facade, cabinet, snapshot, state, line, manager, modbus, server, web. Architecture summary: QML/HMI binds to PanelFacade; PanelFacade exposes Q_PROPERTY/Q_INVOKABLE state and commands and talks to backend state/API; EngineRuntime is backend orchestrator in engine_main.cpp; CabinetSnapshot is DTO/state model shared to HMI, web, persistence and Modbus TCP; LineManager, TestController, TestScheduleManager, StateEngine, StateFileStore, TestJournalStore, ModbusController, MeteringBusController and ModbusTcpServer are main backend services around EngineRuntime.

## Outcome

- Signal: useful

## Source Nodes

- EngineRuntime
- PanelFacade
- CabinetSnapshot
- LineManager
- ModbusController
- MeteringBusController
- ModbusTcpServer