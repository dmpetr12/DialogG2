#include "Logger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QThread>
#include <QtGlobal>
#ifdef Q_OS_WIN
#include <iostream>
#endif

namespace DialogG2 {

static QtMessageHandler previousHandler = nullptr;

static Logger::Level levelFromQtMessage(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return Logger::Level::Debug;
    case QtInfoMsg:
        return Logger::Level::Info;
    case QtWarningMsg:
        return Logger::Level::Warning;
    case QtCriticalMsg:
        return Logger::Level::Error;
    case QtFatalMsg:
        return Logger::Level::Critical;
    }
    return Logger::Level::Info;
}

static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QString line = message;
    if (context.file && context.line > 0) {
        line = QStringLiteral("%1 (%2:%3)")
                   .arg(message, QString::fromLocal8Bit(context.file))
                   .arg(context.line);
    }

    Logger::instance().write(levelFromQtMessage(type), line);

    if (previousHandler)
        previousHandler(type, context, message);

    if (type == QtFatalMsg)
        abort();
}

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::configure(const QString &filePath, qint64 maxFileSize, int maxFiles, Level minLevel)
{
    QMutexLocker locker(&m_mutex);

    if (m_file.isOpen())
        m_file.close();

    m_filePath = filePath;
    m_maxFileSize = maxFileSize > 0 ? maxFileSize : 10 * 1024 * 1024;
    m_maxFiles = maxFiles > 0 ? maxFiles : 5;
    m_minLevel = minLevel;
}

void Logger::setMinLevel(Level level)
{
    QMutexLocker locker(&m_mutex);
    m_minLevel = level;
}

Logger::Level Logger::minLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_minLevel;
}

void Logger::setConsoleOutputEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_consoleOutputEnabled = enabled;
}

bool Logger::consoleOutputEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_consoleOutputEnabled;
}

void Logger::installQtMessageHandler()
{
    previousHandler = qInstallMessageHandler(qtMessageHandler);
}

void Logger::debug(const QString &message)
{
    write(Level::Debug, message);
}

void Logger::info(const QString &message)
{
    write(Level::Info, message);
}

void Logger::warning(const QString &message)
{
    write(Level::Warning, message);
}

void Logger::error(const QString &message)
{
    write(Level::Error, message);
}

void Logger::critical(const QString &message)
{
    write(Level::Critical, message);
}

void Logger::write(Level level, const QString &message)
{
    QMutexLocker locker(&m_mutex);

    if (static_cast<int>(level) < static_cast<int>(m_minLevel))
        return;

    ensureOpen();
    if (!m_file.isOpen())
        return;

    const QString line = QStringLiteral("[%1] [%2] [tid:%3] %4")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")),
             levelName(level),
             QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()), 16),
             message);

    QTextStream out(&m_file);
    out << line << Qt::endl;
    out.flush();

    if (m_consoleOutputEnabled) {
#ifdef Q_OS_WIN
        const QByteArray localLine = line.toLocal8Bit();
        std::ostream &stream = level >= Level::Warning ? std::cerr : std::cout;
        stream << localLine.constData() << std::endl;
#else
        QTextStream console(level >= Level::Warning ? stderr : stdout);
        console << line << Qt::endl;
        console.flush();
#endif
    }

    rotateIfNeeded();
}

QString Logger::levelName(Level level)
{
    switch (level) {
    case Level::Debug:
        return QStringLiteral("DEBUG");
    case Level::Info:
        return QStringLiteral("INFO");
    case Level::Warning:
        return QStringLiteral("WARNING");
    case Level::Error:
        return QStringLiteral("ERROR");
    case Level::Critical:
        return QStringLiteral("CRITICAL");
    }
    return QStringLiteral("INFO");
}

void Logger::ensureOpen()
{
    if (m_filePath.isEmpty()) {
        const QString base = QCoreApplication::applicationDirPath();
        m_filePath = QDir(base).filePath(QStringLiteral("logs/system.log"));
    }

    if (m_file.isOpen())
        return;

    const QFileInfo info(m_filePath);
    QDir().mkpath(info.absolutePath());

    m_file.setFileName(m_filePath);
    const bool opened = m_file.open(QIODevice::Append | QIODevice::Text);
    Q_UNUSED(opened)
}

void Logger::rotateIfNeeded()
{
    if (m_file.size() < m_maxFileSize)
        return;

    m_file.close();
    rotateFiles();
    const bool opened = m_file.open(QIODevice::Append | QIODevice::Text);
    Q_UNUSED(opened)
}

void Logger::rotateFiles()
{
    const QFileInfo info(m_filePath);
    const QString dir = info.absolutePath();
    const QString baseName = info.completeBaseName();
    const QString suffix = info.suffix().isEmpty() ? QStringLiteral("log") : info.suffix();

    const auto rotatedName = [&](int index) {
        return QDir(dir).filePath(QStringLiteral("%1_%2.%3").arg(baseName).arg(index).arg(suffix));
    };

    QFile::remove(rotatedName(m_maxFiles));

    for (int i = m_maxFiles - 1; i >= 1; --i) {
        const QString src = rotatedName(i);
        if (QFile::exists(src))
            QFile::rename(src, rotatedName(i + 1));
    }

    if (QFile::exists(m_filePath))
        QFile::rename(m_filePath, rotatedName(1));
}

} // namespace DialogG2
