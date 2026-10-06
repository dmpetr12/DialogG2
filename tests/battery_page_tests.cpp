#include <QGuiApplication>
#include <QAbstractItemModel>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQmlPropertyMap>
#include <QJSValue>
#include <QQuickItem>
#include <QQuickWindow>
#include <QThread>
#include <cstdio>
#include <cmath>

int main(int argc, char **argv) {
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &message) { fprintf(stderr, "%s\n", message.toUtf8().constData()); });
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    QQmlPropertyMap panel;
    QVariantMap battery{{"connected", true}, {"communicationOk", true}, {"cellCount", 16},
                        {"cellVoltages", QVariantList(16, 3.2)}, {"temperatures", QVariantList{24.0,25.0}},
                        {"socPercent",80}, {"voltage",51.2},
                        {"faults", QVariantList{"fault1","fault2","fault3"}}, {"warnings", QVariantList{"warn1","warn2"}}};
    panel.insert("battery", battery); panel.insert("batteryOk", true);
    engine.rootContext()->setContextProperty("panel", &panel);
    QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(BATTERY_QML_PATH)));
    auto *page = qobject_cast<QQuickItem *>(component.create());
    if (!page) { qCritical() << component.errors(); return 1; }
    QQuickWindow window; window.resize(1024,648); page->setParentItem(window.contentItem());
    window.show();
    auto settle = [&] { for(int i=0;i<10;++i) { app.processEvents(); QThread::msleep(5); } };
    settle();
    auto *list = page->findChild<QObject *>("batteryList");
    if (!list) { delete page; return 2; }
    list->setProperty("contentY", list->property("contentHeight").toDouble() - list->property("height").toDouble()); settle();
    const double y = list->property("contentY").toDouble();
    int failures=0;
    auto check = [&](const char *message) { settle(); if(std::abs(list->property("contentY").toDouble()-y)>1.0) { fprintf(stderr,"FAIL: %s\n",message); ++failures; } };
    battery["voltage"] = 52.0; panel.insert("battery",battery);
    check("value update must preserve scroll");
    auto *model = page->findChild<QAbstractItemModel *>("batteryRows");
    bool updated = false;
    if (model) {
        const auto roles = model->roleNames();
        for (int row=0; row<model->rowCount(); ++row)
            for (auto it=roles.cbegin(); it!=roles.cend(); ++it)
                if (it.value() == "value" && model->data(model->index(row,0),it.key()).toString() == QString::fromUtf8("52.00 В")) updated = true;
    }
    if (!updated) { fprintf(stderr,"FAIL: measurements must keep updating\n"); ++failures; }
    battery["cellVoltages"] = QVariantList{}; panel.insert("battery",battery);
    check("missing cell sample must preserve scroll");
    panel.insert("battery",QVariantMap{{"communicationOk",false}});
    check("disconnect must preserve scroll");
    battery["cellVoltages"] = QVariantList(16,3.3); panel.insert("battery",battery);
    check("reconnect must preserve scroll");
    delete page;

    panel.insert("modeCode", "fire");
    panel.insert("modeText", QString::fromUtf8("Пожар"));
    panel.insert("modeColor", "#d84236");
    panel.insert("manualEmergencyActive", true);
    panel.insert("connected", true);
    panel.insert("systemAvailable", true);
    panel.insert("linesAvailable", true);
    panel.insert("systemOk", true);
    panel.insert("linesOk", true);
    panel.insert("batteryPercent", 80);
    panel.insert("inputVoltage", 230.0);
    panel.insert("outputPower", 100.0);
    panel.insert("battery", battery);
    QQmlComponent startComponent(&engine, QUrl::fromLocalFile(QStringLiteral(START_QML_PATH)));
    auto *startPage = qobject_cast<QQuickItem *>(startComponent.create());
    if (!startPage) { qCritical() << startComponent.errors(); return 3; }
    startPage->setProperty("unlocked", true);
    const QStringList blockedButtons = {"testButton", "settingsButton", "scheduleButton", "journalButton"};
    for (const QString &name : blockedButtons) {
        QObject *button = startPage->findChild<QObject *>(name);
        if (!button || button->property("enabled").toBool()) {
            fprintf(stderr, "FAIL: %s must be disabled during fire\n", name.toUtf8().constData());
            ++failures;
        }
    }
    panel.insert("modeCode", "normal");
    panel.insert("manualEmergencyActive", false);
    settle();
    for (const QString &name : blockedButtons) {
        QObject *button = startPage->findChild<QObject *>(name);
        if (!button || !button->property("enabled").toBool()) {
            fprintf(stderr, "FAIL: %s must be enabled after fire\n", name.toUtf8().constData());
            ++failures;
        }
    }
    delete startPage;

    panel.insert("systemOk", false);
    panel.insert("healthReason", QString::fromUtf8("Есть неисправность: АКБ: нет данных BMS, линия 1, линия 2"));
    panel.insert("readLogs", QVariant::fromValue(engine.evaluate("(function() { return []; })")));
    QQmlComponent systemComponent(&engine, QUrl::fromLocalFile(QStringLiteral(SYSTEM_QML_PATH)));
    auto *systemPage = qobject_cast<QQuickItem *>(systemComponent.create());
    if (!systemPage) { qCritical() << systemComponent.errors(); return 6; }
    systemPage->setParentItem(window.contentItem());
    settle();
    QObject *systemReason = systemPage->findChild<QObject *>("systemReasonText");
    if (!systemReason
        || systemReason->property("text").toString() != QString::fromUtf8("АКБ: нет данных BMS, линия 1, линия 2")
        || systemReason->property("wrapMode").toInt() != 1
        || systemReason->property("truncated").toBool()) {
        fprintf(stderr, "FAIL: system card must show the complete fault reasons without redundant prefix\n");
        ++failures;
    }
    panel.insert("healthReason", QString::fromUtf8(
        "Есть неисправность: связь Modbus, АКБ: нет данных BMS, ток утечки, температура, линия 1, линия 2, линия 3"));
    settle();
    if (!systemReason || systemReason->property("truncated").toBool()) {
        fprintf(stderr, "FAIL: system card must keep a longer fault list visible\n");
        ++failures;
    }
    delete systemPage;

    panel.insert("systemAvailable", false);
    panel.insert("systemOk", false);
    panel.insert("testRunning", true);
    panel.insert("testRemainingSec", 30);
    engine.globalObject().setProperty("functionalStartSeconds", 0);
    panel.insert("startFunctionalTest", QVariant::fromValue(engine.evaluate(
        "(function(seconds) { functionalStartSeconds = seconds; return true; })")));
    QQmlComponent testComponent(&engine, QUrl::fromLocalFile(QStringLiteral(TEST_QML_PATH)));
    auto *testPage = qobject_cast<QQuickItem *>(testComponent.create());
    if (!testPage) { qCritical() << testComponent.errors(); return 4; }
    settle();
    QObject *cabinetStatus = testPage->findChild<QObject *>("cabinetStatus");
    if (!cabinetStatus || cabinetStatus->property("text").toString() != QString::fromUtf8("НЕТ ДАННЫХ")) {
        fprintf(stderr, "FAIL: unavailable system data must not be shown as cabinet fault\n");
        ++failures;
    }
    panel.insert("systemAvailable", true);
    settle();
    if (!cabinetStatus || cabinetStatus->property("text").toString() != QString::fromUtf8("Авария шкафа")) {
        fprintf(stderr, "FAIL: confirmed cabinet fault must remain visible\n");
        ++failures;
    }
    panel.insert("systemOk", true);
    settle();
    if (!cabinetStatus || !cabinetStatus->property("text").toString().isEmpty()) {
        fprintf(stderr, "FAIL: normal cabinet must not show a fault message\n");
        ++failures;
    }
    panel.insert("testRunning", false);
    settle();
    QObject *startStopButton = testPage->findChild<QObject *>("startStopTestButton");
    if (startStopButton)
        QMetaObject::invokeMethod(startStopButton, "clicked");
    settle();
    if (engine.globalObject().property("functionalStartSeconds").toInt() != 120) {
        fprintf(stderr, "FAIL: test start button must run immediately without confirmation popup\n");
        ++failures;
    }
    delete testPage;

    panel.insert("lines", QVariantList{QVariantMap{
        {"index", 1}, {"description", QString::fromUtf8("Гараж")},
        {"mode", 0}, {"displayModeText", QString::fromUtf8("ПОСТОЯН.")},
        {"displayStateText", QString::fromUtf8("ВКЛ")}, {"displayStateOk", true},
        {"power", QVariant()}, {"voltage", QVariant()}, {"current", QVariant()},
        {"leakage", QVariant()}, {"powerAvailable", false}, {"voltageAvailable", false},
        {"currentAvailable", false}, {"leakageAvailable", false}
    }});
    QQmlComponent linesComponent(&engine, QUrl::fromLocalFile(QStringLiteral(LINES_QML_PATH)));
    auto *linesPage = qobject_cast<QQuickItem *>(linesComponent.create());
    if (!linesPage) { qCritical() << linesComponent.errors(); return 5; }
    if (linesPage->property("wType").toInt() < 140
        || linesPage->property("typeFontSize").toInt() >= linesPage->property("tableFontSize").toInt()) {
        fprintf(stderr, "FAIL: HMI line type column must be wider and use a smaller font\n");
        ++failures;
    }
    delete linesPage;

    fprintf(stdout,"%d battery page failures\n",failures);
    return failures ? 1 : 0;
}
