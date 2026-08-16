#include <QGuiApplication>
#include <QFont>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "PanelFacade.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    QFont font(QStringLiteral("Ubuntu"));
    font.setPixelSize(30);
    app.setFont(font);

    PanelFacade panel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("panel"), &panel);
    engine.loadFromModule(QStringLiteral("DialogG2"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
