import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()

    property var logLines: []
    property bool logVisible: false
    property string exportStatus: ""
    readonly property string batteryText: panel.batteryPercent >= 0 ? panel.batteryPercent + "%" : "-"
    readonly property string cabinetModeText: panel.modeCode === "normal" ? "РАБОЧИЙ" : panel.modeText.toUpperCase()

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648
    color: "white"

    function refreshLogs() {
        logLines = panel.readLogs(-200, 200)
    }

    Component.onCompleted: refreshLogs()
    onVisibleChanged: if (visible) refreshLogs()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 82
            spacing: 12

            BackButton {
                Layout.preferredWidth: 82
                Layout.preferredHeight: 82
                onClicked: {
                    if (root.logVisible)
                        root.logVisible = false
                    else
                        root.backRequested()
                }
            }

            Text {
                text: root.logVisible ? "Системный лог" : "Параметры системы"
                color: "#111111"
                font.pixelSize: 36
                font.family: "Arial"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            Rectangle {
                Layout.preferredWidth: 176
                Layout.preferredHeight: 82
                radius: 6
                visible: !root.logVisible
                color: "#c9c9c9"

                Text {
                    anchors.centerIn: parent
                    text: "СИСТЕМНЫЙ\nЛОГ"
                    color: "#111111"
                    font.pixelSize: 22
                    font.bold: true
                    font.family: "Arial"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    anchors.fill: parent
                    onPressed: parent.color = "#a9a9a9"
                    onReleased: parent.color = "#c9c9c9"
                    onCanceled: parent.color = "#c9c9c9"
                    onClicked: {
                        root.refreshLogs()
                        root.logVisible = true
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 176
                Layout.preferredHeight: 82
                radius: 6
                visible: root.logVisible
                color: "#c9c9c9"

                Text {
                    anchors.centerIn: parent
                    text: "ЗАПИСЬ\nНА ФЛЕШКУ"
                    color: "#111111"
                    font.pixelSize: 21
                    font.bold: true
                    font.family: "Arial"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    anchors.fill: parent
                    onPressed: parent.color = "#a9a9a9"
                    onReleased: parent.color = "#c9c9c9"
                    onCanceled: parent.color = "#c9c9c9"
                    onClicked: {
                        root.exportStatus = panel.exportSystemLogToUsb()
                        exportStatusPopup.open()
                        exportStatusTimer.restart()
                    }
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !root.logVisible
            columns: 2
            rowSpacing: 12
            columnSpacing: 12

            ParameterBox {
                title: "Режим шкафа"
                value: root.cabinetModeText
                ok: panel.modeCode === "normal"
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            ParameterBox {
                title: "Состояние системы"
                value: !panel.systemAvailable ? "НЕТ ДАННЫХ" : (panel.systemOk ? "НОРМ" : panel.healthReason.replace(/^Есть неисправность:\s*/, ""))
                ok: panel.systemOk
                longValue: !panel.systemOk && panel.systemAvailable
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            ParameterBox {
                title: "Батарея"
                value: !panel.connected || !panel.battery.communicationOk ? "НЕТ ДАННЫХ" : (panel.batteryOk ? ("НОРМ, " + root.batteryText) : "АВАР")
                ok: panel.batteryOk
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            ParameterBox {
                title: "Линии"
                value: !panel.linesAvailable ? "НЕТ ДАННЫХ" : (panel.linesOk ? "ИСПРАВНЫ" : "АВАРИЯ")
                ok: panel.linesAvailable && panel.linesOk
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            ParameterBox {
                title: "Входное напряжение"
                value: isFinite(panel.inputVoltage) ? panel.inputVoltage.toFixed(0) + " В" : "—"
                ok: panel.inputVoltage > 0
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            ParameterBox {
                title: "Входная мощность"
                value: isFinite(panel.outputPower) ? panel.outputPower.toFixed(0) + " Вт" : "—"
                ok: true
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            ParameterBox {
                title: "Входной ток"
                value: isFinite(panel.inputCurrent) ? panel.inputCurrent.toFixed(1) + " А" : "—"
                ok: true
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            ParameterBox {
                title: "Частота"
                value: isFinite(panel.inputFrequency) ? panel.inputFrequency.toFixed(1) + " Гц" : "—"
                ok: panel.inputFrequency > 0
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.logVisible
            radius: 6
            color: "#ffffff"
            border.color: "#d0d0d0"
            border.width: 2

            Text {
                anchors.centerIn: parent
                visible: root.logLines.length === 0
                text: "Записей системного лога пока нет"
                color: "#666666"
                font.pixelSize: 26
                font.family: "Arial"
            }

            ListView {
                id: logList

                anchors.fill: parent
                anchors.margins: 8
                clip: true
                spacing: 2
                visible: root.logLines.length > 0
                model: root.logLines

                delegate: Text {
                    width: logList.width - 34
                    text: modelData
                    color: modelData.indexOf("[WARN]") >= 0 || modelData.indexOf("[CRITICAL]") >= 0
                           ? "#c43b32"
                           : "#222222"
                    font.pixelSize: 18
                    font.family: "Consolas"
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    width: 28
                }
            }
        }
    }

    Popup {
        id: exportStatusPopup

        width: Math.min(440, root.width - 48)
        height: 112
        x: (root.width - width) / 2
        y: 118
        modal: false
        focus: false
        closePolicy: Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 6
            color: "#ffffff"
            border.color: root.exportStatus.indexOf("Записано") === 0 ? "#0d8a43" : "#c43b32"
            border.width: 3
        }

        contentItem: Text {
            anchors.fill: parent
            anchors.margins: 14
            text: root.exportStatus
            color: root.exportStatus.indexOf("Записано") === 0 ? "#0d8a43" : "#c43b32"
            font.pixelSize: 24
            font.family: "Arial"
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
        }
    }

    Timer {
        id: exportStatusTimer
        interval: 2500
        repeat: false
        onTriggered: exportStatusPopup.close()
    }

    component ParameterBox: Rectangle {
        property string title: ""
        property string value: ""
        property bool ok: true
        property bool longValue: false

        radius: 6
        color: "#f7f7f7"
        border.color: "#d0d0d0"
        border.width: 2

        RowLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 18

            Rectangle {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                radius: 24
                color: ok ? "#11bf5d" : "#d84236"
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: longValue ? 4 : 8

                Text {
                    text: title
                    color: "#555555"
                    font.pixelSize: longValue ? 20 : 24
                    font.family: "Arial"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Text {
                    objectName: longValue ? "systemReasonText" : ""
                    text: value
                    color: "#111111"
                    font.pixelSize: longValue ? 17 : 34
                    font.family: "Arial"
                    font.bold: true
                    wrapMode: longValue ? Text.WordWrap : Text.NoWrap
                    elide: Text.ElideRight
                    maximumLineCount: longValue ? 3 : 1
                    Layout.fillWidth: true
                }
            }
        }
    }

    component BackButton: Rectangle {
        signal clicked()
        color: "lightgray"
        radius: 6

        Image {
            anchors.fill: parent
            source: "../assets/Back.png"
            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            anchors.fill: parent
            onPressed: parent.color = "gray"
            onReleased: parent.color = "lightgray"
            onCanceled: parent.color = "lightgray"
            onClicked: parent.clicked()
        }
    }
}
