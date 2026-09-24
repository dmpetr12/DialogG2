import QtQuick
import QtQuick.Controls

Item {
    id: root

    property bool unlocked: false
    signal loginRequested()
    signal logoutRequested()
    signal testRequested()
    signal settingsRequested()
    signal scheduleRequested()
    signal journalRequested()
    signal systemRequested()
    signal linesRequested()
    signal batteryRequested()
    property var confirmedAction: null

    readonly property bool normalMode: panel.modeCode === "normal"
    readonly property bool fireActive: panel.modeCode === "fire" || panel.manualEmergencyActive
    readonly property bool systemOk: panel.systemOk
    readonly property bool linesOk: panel.linesOk
    readonly property string batteryText: panel.batteryPercent >= 0 ? panel.batteryPercent + "%" : "-"

    Rectangle {
        anchors.fill: parent
        color: "#ffffff"
    }

    Column {
        id: menu

        anchors.left: parent.left
        anchors.leftMargin: 13
        anchors.top: parent.top
        anchors.topMargin: 6
        spacing: 10

        HmiButton {
            text: root.unlocked ? "ВЫХОД" : "ВХОД"
            onClicked: root.unlocked ? root.logoutRequested() : root.loginRequested()
        }
        HmiButton {
            text: panel.manualEmergencyActive ? "СТОП" : "ПУСК"
            enabled: root.unlocked
            buttonColor: root.unlocked ? (panel.manualEmergencyActive ? "#ffa000" : "#d84236") : "#dddddd"
            textColor: root.unlocked ? "#ffffff" : "#8a8a8a"
            onClicked: {
                if (panel.manualEmergencyActive) {
                    root.askConfirmation("Снять пожар?",
                                         "Ручной пожар от панели будет остановлен.",
                                         "СТОП",
                                         function() { panel.stopManualEmergency() })
                } else {
                    root.askConfirmation("Включить пожар?",
                                         "Шкаф перейдет в режим пожар от команды оператора.",
                                         "ПУСК",
                                         function() { panel.startManualEmergency() })
                }
            }
        }
        HmiButton {
            objectName: "testButton"
            text: "ТЕСТ"
            enabled: root.unlocked && !root.fireActive
            locked: !root.unlocked || root.fireActive
            onClicked: root.testRequested()
        }
        HmiButton {
            objectName: "settingsButton"
            text: "НАСТРОЙКА"
            enabled: root.unlocked && !root.fireActive
            locked: !root.unlocked || root.fireActive
            onClicked: root.settingsRequested()
        }
        HmiButton {
            objectName: "scheduleButton"
            text: "РАСПИСАНИЕ"
            enabled: root.unlocked && !root.fireActive
            locked: !root.unlocked || root.fireActive
            onClicked: root.scheduleRequested()
        }
        HmiButton {
            objectName: "journalButton"
            text: "ЖУРНАЛ"
            enabled: root.unlocked && !root.fireActive
            locked: !root.unlocked || root.fireActive
            onClicked: root.journalRequested()
        }
    }

    Column {
        anchors.left: menu.right
        anchors.leftMargin: 26
        anchors.top: parent.top
        anchors.topMargin: 22
        spacing: 18

        StateLine {
            label: "Режим"
            value: root.normalMode ? "РАБОЧИЙ" : panel.modeText.toUpperCase()
            ok: root.normalMode
            faultColor: panel.modeColor
        }

        StateLine {
            label: "Система"
            value: !panel.systemAvailable ? "НЕТ ДАННЫХ" : (root.systemOk ? "НОРМ" : "АВАР")
            ok: root.systemOk
            labelBadge: true
            clickableBadge: true
            onBadgeClicked: root.systemRequested()
        }

        StateLine {
            label: "Батарея"
            value: !panel.connected || !panel.battery.communicationOk ? "НЕТ ДАННЫХ" : (panel.batteryOk ? "НОРМ" : "АВАР")
            ok: panel.batteryOk
            labelBadge: true
            clickableBadge: true
            onBadgeClicked: root.batteryRequested()
        }

        StateLine {
            label: "Линии"
            value: !panel.linesAvailable ? "НЕТ ДАННЫХ" : (root.linesOk ? "ИСПР" : "АВАР")
            ok: panel.linesAvailable && root.linesOk
            labelBadge: true
            clickableBadge: true
            onBadgeClicked: root.linesRequested()
        }

        MetricLine {
            label: "Напряжение"
            value: isFinite(panel.inputVoltage) ? panel.inputVoltage.toFixed(0) + "В" : "—"
        }

        MetricLine {
            label: "Мощность"
            value: isFinite(panel.outputPower) ? panel.outputPower.toFixed(0) + "Вт" : "—"
        }

        MetricLine {
            label: "Заряд"
            value: root.batteryText
        }
    }

    function askConfirmation(title, text, actionText, action) {
        confirmTitle.text = title
        confirmText.text = text
        confirmOk.text = actionText
        confirmedAction = action
        confirmPopup.open()
    }

    Popup {
        id: confirmPopup

        parent: Overlay.overlay
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        width: 520
        height: 300
        onClosed: root.confirmedAction = null

        Rectangle {
            anchors.fill: parent
            radius: 10
            color: "#dce9f7"
            border.color: "#315f8f"
            border.width: 2

            Column {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 18

                Text {
                    id: confirmTitle
                    width: parent.width
                    color: "#17395c"
                    font.pixelSize: 34
                    font.family: "Arial"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    id: confirmText
                    width: parent.width
                    height: 92
                    color: "#17395c"
                    wrapMode: Text.WordWrap
                    font.pixelSize: 24
                    font.family: "Arial"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 22

                    Button {
                        width: 200
                        height: 72
                        text: "Отмена"
                        font.pixelSize: 30
                        contentItem: Text {
                            text: parent.text
                            color: "#17395c"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 30
                            font.family: "Arial"
                        }
                        background: Rectangle {
                            radius: 8
                            color: "#f6f8fb"
                            border.color: "#9fb5ca"
                            border.width: 2
                        }
                        onClicked: confirmPopup.close()
                    }

                    Button {
                        id: confirmOk
                        width: 200
                        height: 72
                        font.pixelSize: 30
                        contentItem: Text {
                            text: parent.text
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 30
                            font.family: "Arial"
                        }
                        background: Rectangle {
                            radius: 8
                            color: "#315f8f"
                        }
                        onClicked: {
                            var action = root.confirmedAction
                            root.confirmedAction = null
                            confirmPopup.close()
                            if (action)
                                action()
                        }
                    }
                }
            }
        }
    }

    component HmiButton: Button {
        property bool selected: false
        property bool locked: false
        property color buttonColor: "#c6c6c6"
        property color textColor: "#111111"

        width: 384
        height: 87

        background: Rectangle {
            radius: 16
            color: parent.locked ? "#dddddd" : parent.buttonColor
        }

        contentItem: Text {
            text: parent.text
            color: parent.locked ? "#8a8a8a" : parent.textColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 41
            font.family: "Arial"
        }
    }

    component StateLine: Row {
        signal badgeClicked()
        property string label: ""
        property string value: ""
        property bool ok: true
        property bool labelBadge: false
        property bool clickableBadge: false
        property color faultColor: "#d84236"

        spacing: 20
        height: 72

        Rectangle {
            width: 62
            height: 62
            radius: 31
            color: ok ? "#11bf5d" : faultColor
        }

        Item {
            height: 62
            width: 205

            Text {
                visible: !labelBadge
                height: 62
                text: label
                color: "#111111"
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 41
                font.family: "Arial"
            }

            Rectangle {
                visible: labelBadge
                width: 205
                height: 62
                radius: 13
                color: "#c9c9c9"

                Text {
                    id: labelBadgeText
                    anchors.centerIn: parent
                    text: label
                    color: "#111111"
                    font.pixelSize: 41
                    font.family: "Arial"
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: clickableBadge
                    onClicked: badgeClicked()
                }
            }
        }

        Text {
            height: 62
            text: value
            color: "#111111"
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 41
            font.family: "Arial"
        }
    }

    component MetricLine: Row {
        property string label: ""
        property string value: ""

        spacing: 31
        height: 52

        Text {
            width: 251
            text: label
            color: "#111111"
            font.pixelSize: 41
            font.family: "Arial"
        }

        Text {
            width: 210
            text: value
            color: "#111111"
            font.pixelSize: 41
            font.family: "Arial"
        }
    }
}
