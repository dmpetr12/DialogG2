import QtQuick
import QtQuick.Controls

Item {
    id: root
    objectName: "testPage"

    property bool unlocked: false
    property int selectedTest: 0
    property int functionalDurationMinutes: 2
    property int durationHours: 1
    readonly property bool systemOk: panel.systemOk

    signal backRequested()
    signal inputRequested(string nameText, string propertyName, real initialValue, int maxValue)

    function remainingText() {
        var sec = panel.testRemainingSec
        var min = Math.floor(sec / 60)
        var rest = sec % 60
        return min + ":" + (rest < 10 ? "0" + rest : rest)
    }

    function startOrStopTest() {
        if (panel.testRunning) {
            panel.stopCurrentTest()
            return
        }

        if (selectedTest === 0)
            panel.startFunctionalTest(functionalDurationMinutes * 60)
        else
            panel.startDurationTest(durationHours * 3600)
    }

    Component.onDestruction: {
        if (panel.testRunning)
            panel.stopCurrentTest()
    }

    Rectangle {
        anchors.fill: parent
        color: "#ffffff"
    }

    Item {
        anchors.left: parent.left
        anchors.leftMargin: 36
        anchors.right: parent.right
        anchors.rightMargin: 36
        anchors.top: parent.top
        anchors.topMargin: 18
        anchors.bottom: backButton.top
        anchors.bottomMargin: 14

        Text {
            id: pageTitle

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            text: "Тестирование линий"
            color: "#111111"
            font.pixelSize: 40
            font.family: "Arial"
            horizontalAlignment: Text.AlignHCenter
        }

        Column {
            id: selector

            anchors.left: parent.left
            anchors.top: pageTitle.bottom
            anchors.topMargin: 36
            spacing: 12

            TestModeButton {
                text: "Тест всех\nлиний"
                active: root.selectedTest === 0
                onClicked: root.selectedTest = 0
            }

            TestModeButton {
                text: "Тест на время"
                active: root.selectedTest === 1
                onClicked: root.selectedTest = 1
            }
        }

        Rectangle {
            id: testPanel

            anchors.left: selector.right
            anchors.leftMargin: 46
            anchors.top: pageTitle.bottom
            anchors.topMargin: 36
            anchors.right: parent.right
            height: 330
            color: "#ffffff"

            Text {
                id: title

                anchors.left: parent.left
                anchors.top: parent.top
                text: root.selectedTest === 0 ? "ТЕСТ: все линии" : "ТЕСТ: на время"
                color: "#111111"
                font.pixelSize: 32
                font.family: "Arial"
            }

            Row {
                anchors.left: parent.left
                anchors.top: title.bottom
                anchors.topMargin: 18
                spacing: 90

                TouchButton {
                    objectName: "startStopTestButton"
                    text: panel.testRunning ? "Стоп" : "Старт"
                    width: 200
                    height: 72
                    buttonColor: "#ffa000"
                    textColor: "#ffffff"
                    onClicked: root.startOrStopTest()
                }

                Rectangle {
                    width: 200
                    height: 72
                    radius: 8
                    color: "#ffffff"
                    border.color: "#d0d0d0"
                    border.width: 2

                    Text {
                        anchors.centerIn: parent
                        text: panel.testRunning ? root.remainingText() : "0:00"
                        color: "#111111"
                        font.pixelSize: 36
                        font.family: "Arial"
                    }
                }
            }

            MetricEditRow {
                anchors.left: parent.left
                anchors.top: title.bottom
                anchors.topMargin: 108
                label: root.selectedTest === 0 ? "Время теста, мин:" : "Время теста, ч:"
                value: root.selectedTest === 0 ? root.functionalDurationMinutes.toString() : root.durationHours.toString()
                onClicked: {
                    if (root.selectedTest === 0)
                        root.inputRequested("Введите длительность, мин", "functionalDurationMinutes", root.functionalDurationMinutes, 59)
                    else
                        root.inputRequested("Введите длительность, ч", "durationHours", root.durationHours, 3)
                }
            }

            Text {
                objectName: "cabinetStatus"
                anchors.left: parent.left
                anchors.top: title.bottom
                anchors.topMargin: 162
                text: !panel.systemAvailable ? "НЕТ ДАННЫХ" : (root.systemOk ? "" : "Авария шкафа")
                color: panel.systemAvailable ? "#ff4040" : "#8a8a8a"
                font.pixelSize: 22
                font.family: "Arial"
            }
        }
    }

    BackButton {
        id: backButton

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 12
        onClicked: root.backRequested()
    }

    component TestModeButton: Button {
        property bool active: false

        width: 320
        height: 86

        background: Rectangle {
            radius: 14
            color: parent.active ? "#c77a00" : "#ffa000"
            border.color: parent.active ? "#ffcc4d" : "transparent"
            border.width: parent.active ? 2 : 0
        }

        contentItem: Text {
            text: parent.text
            color: "#ffffff"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 34
            font.family: "Arial"
            wrapMode: Text.WordWrap
        }
    }

    component TouchButton: Button {
        property color buttonColor: "#c6c6c6"
        property color textColor: "#111111"

        background: Rectangle {
            radius: 8
            color: parent.buttonColor
        }

        contentItem: Text {
            text: parent.text
            color: parent.textColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 32
            font.family: "Arial"
        }
    }

    component MetricEditRow: Row {
        property string label: ""
        property string value: ""
        signal clicked()

        spacing: 34

        Text {
            width: 255
            text: label
            color: "#111111"
            font.pixelSize: 30
            font.family: "Arial"
            verticalAlignment: Text.AlignVCenter
        }

        Rectangle {
            width: 172
            height: 58
            radius: 8
            color: "#fff6dc"
            border.color: "#ffa000"
            border.width: 3

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 68
                text: value
                color: "#111111"
                font.pixelSize: 30
                font.family: "Arial"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 42
                color: "#ffa000"
                radius: 8

                Text {
                    anchors.centerIn: parent
                    text: "..."
                    color: "#ffffff"
                    font.pixelSize: 28
                    font.family: "Arial"
                    font.bold: true
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: parent.parent.clicked()
            }
        }
    }

    component BackButton: Rectangle {
        signal clicked()
        width: 150
        height: 150
        color: "lightgray"
        radius: 6

        Image {
            anchors.fill: parent
            source: "../assets/Back.png"
            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.color = "lightgray"
            onExited: parent.color = "lightgray"
            onPressed: parent.color = "gray"
            onReleased: parent.color = "lightgray"
            onClicked: parent.clicked()
        }
    }
}
