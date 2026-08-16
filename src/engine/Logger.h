#pragma once

#include <QFile>
#include <QLoggingCategory>
#include <QMutex>
#include <QString>

namespace DialogG2 {

class Logger
{
public:
    enum class Level
    {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
        Critical = 4
    };

    static Logger &instance();

    void configure(const QString &filePath,
                   qint64 maxFileSize = 10 * 1024 * 1024,
                   int maxFiles = 5,
                   Level minLevel = Level::Info);

    void setMinLevel(Level level);
    Level minLevel() const;
    void setConsoleOutputEnabled(bool enabled);
    bool consoleOutputEnabled() const;

    void installQtMessageHandler();

    void debug(const QString &message);
    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);
    void critical(const QString &message);
    void write(Level level, const QString &message);

    static QString levelName(Level level);

private:
    Logger() = default;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    void ensureOpen();
    void rotateIfNeeded();
    void rotateFiles();

    mutable QMutex m_mutex;
    QFile m_file;
    QString m_filePath;
    qint64 m_maxFileSize = 10 * 1024 * 1024;
    int m_maxFiles = 5;
    Level m_minLevel = Level::Info;
    bool m_consoleOutputEnabled = false;
};

} // namespace DialogG2

#define LOG_DEBUG(message) \
    ::DialogG2::Logger::instance().debug(message)

#define LOG_INFO(message) \
    ::DialogG2::Logger::instance().info(message)

#define LOG_WARN(message) \
    ::DialogG2::Logger::instance().warning(message)

#define LOG_ERROR(message) \
    ::DialogG2::Logger::instance().error(message)

#define LOG_CRITICAL(message) \
    ::DialogG2::Logger::instance().critical(message)
