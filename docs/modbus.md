# Modbus RTU

В шкафу две внутренние линии Modbus RTU. Порты и параметры обмена хранятся в:

```text
config/app.json
```

Дефолт:

```json
{
  "modbus": {
    "relay": {
      "port": "/dev/rs485_relay",
      "baudRate": 9600,
      "parity": "none",
      "dataBits": 8,
      "stopBits": 1,
      "timeoutMs": 250,
      "retries": 0,
      "busOfflineFailureThreshold": 3
    },
    "metering": {
      "port": "/dev/rs485_metering",
      "baudRate": 9600,
      "parity": "none",
      "dataBits": 8,
      "stopBits": 1,
      "timeoutMs": 250,
      "retries": 0,
      "busOfflineFailureThreshold": 3
    }
  }
}
```

На Ubuntu панели лучше использовать стабильные alias `/dev/rs485_relay` и `/dev/rs485_metering`, а не прямые `/dev/ttyUSB0`.
Alias потом зададим через `udev`, чтобы программа не зависела от случайного номера USB-порта.

## Разделение шин

`relay` - внутренняя шина WaveShare:

```text
WaveShare входы/реле
пожарный вход
реле контроля напряжения
команды линий
```

`metering` - внутренняя шина измерительных приборов:

```text
измерение входного напряжения
вводной мощностомер
выходные мощностомеры
измеритель утечки
температура шкафа
BMS JBD/Jiabaida
```

В коде это два разных владельца портов:

```text
relayRtu()    -> ModbusController, старый Qt QModbusRtuSerialClient для WaveShare
meteringRtu() -> MeteringBusController, свой QSerialPort для Modbus RTU приборов и BMS JBD
```

Текущая BMS из паспорта `BMS батареи.pdf` работает не по Modbus RTU, а по протоколу Jiabaida/JBD с кадрами `DD A5 ... 77`.
Поэтому измерительная шина использует `MeteringBusController`: он владеет одним `QSerialPort` и по очереди отправляет обычные Modbus RTU-кадры измерителям и raw JBD-кадры BMS.

## Контроль шины

`ModbusBusMonitor` хранит состояние обмена:

```text
online
consecutiveFailures
lastError
```

Реальный `ModbusController` должен вызывать:

```text
markSuccess()          после любого успешного ответа устройства
markFailure(error)     после таймаута или ошибки запроса
```

После `busOfflineFailureThreshold` подряд ошибок шина считается offline. Это станет неисправностью `связь Modbus` для общего состояния шкафа.

## ModbusController для реле

`ModbusController` оставлен для шины `relay`. Он открывает порт через `QModbusRtuSerialClient`, держит очередь запросов и управляет WaveShare.

Очереди:

```text
High   = записи, например полный байт реле WaveShare
Normal = основной опрос входов и важных регистров
Low    = редкий/диагностический опрос, например чтение состояния реле
```

Для одинаковых периодических чтений действует coalescing: если такой запрос уже стоит в очереди, второй не добавляется.
Для записи реле WaveShare остается только последняя запись на тот же модуль, чтобы не гонять устаревшие состояния.

Готовые операции WaveShare:

```text
readWaveShareInputs(module)
readWaveShareRelays(module)
writeWaveShareRelayByte(module, bits)
```

Планировщик поддерживает разные циклы опроса WaveShare:

```text
addWaveShareModulePolling(module, inputsIntervalMs, relaysIntervalMs)
```

## MeteringBusController для измерений

`MeteringBusController` - владелец одного `QSerialPort` на шине `metering`.
Приоритетов здесь нет: приборов мало, все запросы информативные, поэтому запросы идут простой очередью по мере наступления своих интервалов.

Он умеет:

```text
addAdl200InputMeterPolling(intervalMs, slaveAddress)
addAmc16zFak24BranchPowerPolling(intervalMs, slaveAddress)
addAsj60Ld16aLeakagePolling(intervalMs, slaveAddress)
addWhdTemperatureHumidityPolling(intervalMs, slaveAddress)
addJbdBmsPolling(basicInfoIntervalMs, cellVoltagesIntervalMs)
```

Для обычных приборов `MeteringBusController` вручную собирает Modbus RTU-запросы через `ModbusRtuCodec`:

```text
slave | function | start | count | CRC16
```

Для BMS он отправляет кадры `JbdBmsProtocol`:

```text
DD A5 03 00 FF FD 77
DD A5 04 00 FF FC 77
```

Наружу он отдает уже разобранные события:

```text
adl200InputMeterUpdated(...)
amc16zFak24BranchPowersUpdated(...)
asj60Ld16aLeakageUpdated(...)
whdTemperatureHumidityUpdated(...)
jbdBmsBatteryUpdated(...)
```

Низкоуровневые сырые регистры для измерительной шины сейчас намеренно не являются основным интерфейсом: нам нужны готовые данные приборов.

Диагностический сырой Modbus RTU по-прежнему можно читать старым `ModbusController`, но не на том же физическом порту, который уже открыт `MeteringBusController`.

Старые вспомогательные методы `ModbusController` для общего Modbus остаются в коде, но для шины `metering` новый путь такой:

```text
ModbusRtuCodec + MeteringBusController
```

Старые generic-методы:

```text
addHoldingRegistersPolling(slave, start, count, intervalMs)
addInputRegistersPolling(slave, start, count, intervalMs)
```

## Входной измеритель ADL200

Входной измеритель мощности стоит на шине `metering`, адрес Modbus RTU по умолчанию `1`.
По спецификации ADL200 читаем holding registers:

```text
slave = 1
function = 03 Read Holding Registers
start = 0x000B
count = 7
```

Один запрос возвращает:

```text
0x000B = Voltage, 0.1 V
0x000C = Current, 0.01 A
0x000D = Active power, 0.001 kW = W
0x000E = Reactive power, 0.001 kvar = var
0x000F = Apparent power, 0.001 kVA = VA
0x0010 = Power factor, 0.001
0x0011 = Frequency, 0.01 Hz
```

В коде разбор делает `Adl200Meter::decodeRealtimeHoldingRegisters()`.
`MeteringBusController::addAdl200InputMeterPolling()` добавляет периодический опрос и при успешном ответе отдаёт сигнал `adl200InputMeterUpdated(...)`.

## Измеритель линий AMC16Z-FAK24

AMC16Z-FAK24 стоит на шине `metering`, адрес Modbus RTU по умолчанию `2`.
По паспорту FAK24/FAK48 занимает два адреса на одной шине: при базовом адресе `2` адрес `3` тоже занят этим прибором.

Для мощностей линий читаем holding registers:

```text
slave = 2
function = 03 Read Holding Registers
start = 0x00C0
count = 48
```

Это 24 значения активной мощности отходящих линий:

```text
0x00C0..0x00C1 = линия 1, float, kW
0x00C2..0x00C3 = линия 2, float, kW
...
0x00EE..0x00EF = линия 24, float, kW
```

В коде разбор делает `Amc16zFak24Meter::decodeActivePowerHoldingRegisters()`.
Значения сразу переводятся из `kW` в `W`, чтобы их можно было класть в `LineSnapshot.outputPower` и сравнивать с `nominalPower`.
`MeteringBusController::addAmc16zFak24BranchPowerPolling()` добавляет периодический опрос и при успешном ответе отдаёт сигнал `amc16zFak24BranchPowersUpdated(...)`.

## Измеритель утечки ASJ60-LD16A/C

ASJ60-LD16A/C стоит на шине `metering`, адрес Modbus RTU по умолчанию `4`.
Адреса `2` и `3` занимает AMC16Z-FAK24, поэтому следующий свободный адрес - `4`.

Для токов утечки и статусов 16 каналов читаем holding registers:

```text
slave = 4
function = 03 Read Holding Registers
start = 0x0011
count = 32
```

Каждый канал занимает два регистра:

```text
0x0011 = статус канала 1
0x0012 = ток утечки канала 1, mA
0x0013 = статус канала 2
0x0014 = ток утечки канала 2, mA
...
0x002F = статус канала 16
0x0030 = ток утечки канала 16, mA
```

Статус канала:

```text
0 = Норма
1 = Предупреждение
2 = Авария
```

В коде разбор делает `Asj60Ld16aMonitor::decodeChannelHoldingRegisters()`.
`MeteringBusController::addAsj60Ld16aLeakagePolling()` добавляет периодический опрос и при успешном ответе отдаёт сигнал `asj60Ld16aLeakageUpdated(...)`.

## Датчик температуры и влажности WHD

WHD стоит на шине `metering`, адрес Modbus RTU по умолчанию в нашем проекте `5`.
Адреса `1`, `2/3` и `4` уже заняты ADL200, AMC16Z-FAK24 и ASJ60-LD16A/C.

Для первого канала читаем holding registers:

```text
slave = 5
function = 03 Read Holding Registers
start = 0x0001
count = 2
```

Один запрос возвращает:

```text
0x0001 = температура канала 1, signed int, 0.1 °C
0x0002 = влажность канала 1, signed int, 0.1 %RH
```

По паспорту WHD эти же регистры допускают чтение функцией `04`, но в нашем контроллере используем `03`, как в примере паспорта.

В коде разбор делает `WhdTemperatureHumidityController::decodeChannel1RealtimeRegisters()`.
`MeteringBusController::addWhdTemperatureHumidityPolling()` добавляет периодический опрос и при успешном ответе отдаёт сигнал `whdTemperatureHumidityUpdated(...)`.
