#include <QGuiApplication>
#include <QFile>
#include <QFont>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include <array>
#include <utility>

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
    const std::array<std::pair<QString, QUrl>, 4> mainQmlLocations = {{
        {QStringLiteral(":/DialogG2/qml/Main.qml"), QUrl(QStringLiteral("qrc:/DialogG2/qml/Main.qml"))},
        {QStringLiteral(":/qt/qml/DialogG2/qml/Main.qml"), QUrl(QStringLiteral("qrc:/qt/qml/DialogG2/qml/Main.qml"))},
        {QStringLiteral(":/qt/qml/DialogG2/Main.qml"), QUrl(QStringLiteral("qrc:/qt/qml/DialogG2/Main.qml"))},
        {QStringLiteral(":/DialogG2/Main.qml"), QUrl(QStringLiteral("qrc:/DialogG2/Main.qml"))},
    }};

    for (const auto &location : mainQmlLocations) {
        if (!QFile::exists(location.first))
            continue;

        engine.load(location.second);
        break;
    }
#endif

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
