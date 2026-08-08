# Dialog G2 Panel

Чистый старт проекта шкафа аварийного освещения Dialog G2.

## Состав

- `dialog-g2-engine` - постоянный процесс управления шкафом.
- `dialog-g2-hmi` - HMI как потребитель состояния и источник команд оператора.
- `ModbusController` - релейная шина WaveShare.
- `MeteringBusController` - измерительная шина: Modbus RTU приборы и BMS JBD.

## Сборка

```powershell
cmake --preset msvc-debug
cmake --build --preset debug
```

## Движок

`dialog-g2-engine` запускается как постоянный процесс. Он опрашивает релейную и измерительную шины, пересчитывает состояние шкафа и циклически записывает JSON-снимок.

```powershell
$env:PATH="C:\Qt\6.10.3\msvc2022_64\bin;$env:PATH"
.\build-msvc\Debug\dialog-g2-engine.exe .\state\current_state.json
```

Для подробного лога:

```powershell
.\build-msvc\Debug\dialog-g2-engine.exe .\state\current_state.json --log-debug
```

Приоритет режима:

1. `Пожар` - внешний пожарный вход или ручная авария.
2. `Авария` - контакт реле контроля напряжения.
3. `Тест ручной` - ручной тест.
4. `Тест по расписанию` - плановый тест.
5. `Норма` - штатная работа.
