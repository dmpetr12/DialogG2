# Логирование

В проекте используется `DialogG2::Logger`.

## Файл

По умолчанию движок пишет лог рядом с исполняемым файлом:

```text
logs/system.log
```

## Ротация

По умолчанию:

```text
размер файла: 10 MB
количество архивов: 5
```

При переполнении:

```text
system.log -> system_1.log
system_1.log -> system_2.log
...
system_5.log удаляется
```

При записи системного лога на флешку HMI копирует все существующие файлы комплекта:

```text
system.log
system_1.log
...
system_5.log
```

## Уровни

```text
DEBUG
INFO
WARNING
ERROR
CRITICAL
```

Основной рабочий уровень по умолчанию: `INFO`.

Для подробного отладочного лога:

```powershell
.\build-msvc\Debug\dialog-g2-engine.exe .\state\current_state.json --log-debug
```

`qDebug()`, `qInfo()`, `qWarning()`, `qCritical()` после установки обработчика тоже попадают в файл.
