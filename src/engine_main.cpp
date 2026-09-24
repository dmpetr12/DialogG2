#include "EngineRuntime.h"

#include "engine/Logger.h"

#include <QCoreApplication>
#include <QStringList>

using namespace DialogG2;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QStringList args = app.arguments();
    Logger::instance().configure(defaultLogPath(),
                                 10 * 1024 * 1024,
                                 5,
                                 args.contains(QStringLiteral("--log-debug"))
                                     ? Logger::Level::Debug
                                     : Logger::Level::Info);
    Logger::instance().setConsoleOutputEnabled(args.contains(QStringLiteral("--log-debug")));
    Logger::instance().installQtMessageHandler();
    LOG_INFO(QStringLiteral("dialog-g2-engine started"));
    LOG_DEBUG(QStringLiteral("Debug log enabled"));

    const bool demoMode = args.contains(QStringLiteral("--demo"));

    AppConfig config;
    QString configError;
    const QString appConfigPath = defaultAppConfigPath();
    if (!config.load(appConfigPath, &configError)) {
        LOG_WARN(QStringLiteral("App config not loaded: %1. Using defaults").arg(configError));
        config.save(appConfigPath);
    }
    LOG_INFO(QStringLiteral("Relay RTU port: %1, baud=%2")
                 .arg(config.relayRtu().port)
                 .arg(config.relayRtu().baudRate));
    LOG_INFO(QStringLiteral("Metering RTU port: %1, baud=%2")
                 .arg(config.meteringRtu().port)
                 .arg(config.meteringRtu().baudRate));

    QString statePath = defaultStatePath();
    for (const QString &arg : args.mid(1)) {
        if (!arg.startsWith(QStringLiteral("--"))) {
            statePath = arg;
            break;
        }
    }

    EngineRuntime runtime(config, statePath, demoMode);
    runtime.start();

    return app.exec();
}
