# Web Server

Движок поднимает HTTP server для удаленного просмотра и управления из секции `web` в `config/app.json`.

По умолчанию:

```json
{
    "enabled": true,
    "bind": "0.0.0.0",
    "port": 8080,
    "root": "web"
}
```

Локально страница доступна по адресу:

```text
http://127.0.0.1:8080/
```

На щите с другого устройства:

```text
http://<ip-адрес-щита>:8080/
```

## API

Без авторизации:

| Метод | Путь | Назначение |
| --- | --- | --- |
| `GET` | `/api/health` | Проверка доступности |
| `GET` | `/api/state` | Текущее состояние для веб-панели |
| `GET` | `/api/lines` | Список линий |
| `GET` | `/api/lines/<index>` | Одна линия |
| `GET` | `/api/journal` | Журнал тестов |
| `GET` | `/api/logs?offset=-200&limit=200` | Системный лог, свежие строки первыми |
| `GET` | `/api/schedule` | Расписание тестов |
| `POST` | `/api/login` | Вход по паролю, возвращает bearer token |

Тело входа:

```json
{"password": "1234"}
```

С авторизацией `Authorization: Bearer <token>`:

| Метод | Путь | Назначение |
| --- | --- | --- |
| `POST` | `/api/manual-emergency/start` | Включить ручную аварию |
| `POST` | `/api/manual-emergency/stop` | Снять ручную аварию |
| `POST` | `/api/test/start-functional` | Запустить тест исправности |
| `POST` | `/api/test/start-duration` | Запустить тест длительности |
| `POST` | `/api/test/stop` | Остановить текущий тест |
| `POST` | `/api/schedule/add` | Добавить запись расписания |
| `POST` | `/api/schedule/<index>/update` | Изменить запись расписания |
| `POST` | `/api/schedule/<index>/remove` | Удалить запись расписания |
| `POST` | `/api/password/change` | Сменить пароль |
| `POST` | `/api/system/time` | Установить системное время |

Примеры тел команд:

```json
{"warmupSec": 600}
```

```json
{"durationSec": 3600}
```

```json
{"msec": 1787490000000}
```
