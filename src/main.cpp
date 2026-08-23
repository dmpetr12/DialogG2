#include <QGuiApplication>
#include <QFont>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    engine.loadFromModule(QStringLiteral("DialogG2"), QStringLiteral("Main"));
#else
    const QUrl mainQmlUrl(QStringLiteral("qrc:/qt/qml/DialogG2/Main.qml"));
    engine.load(mainQmlUrl);
#endif

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
