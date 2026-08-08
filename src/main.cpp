#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "CabinetState.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    CabinetState cabinet;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("cabinet"), &cabinet);
    engine.loadFromModule(QStringLiteral("DialogG2"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
