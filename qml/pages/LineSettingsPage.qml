import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: lineEditPage

    property int lineIndex: 0
    property int lineMode: 0
    property string lineDescription: "Линия " + (lineIndex + 1)
    property double lineMpower: 100
    property double lineTolerance: 5
    property double measuredPower: 0
    property double measuredVoltage: 0
    property double measuredCurrent: 0
    property bool measuredAvailable: false
    property int maxPower: 2000
    property int maxTolerance: 50

    signal backRequested()
    signal inputRequested(string nameText, string propertyName, real initialValue, int maxValue)

    function loadLine() {
        if (typeof panel === "undefined")
            return

        var ln = panel.lineAt(lineIndex)
        if (!ln)
            return

        lineDescription = ln.description || ("Линия " + (lineIndex + 1))
        lineMpower = Number(ln.mpower || 0)
        lineTolerance = Number(ln.tolerance || 0)
        lineMode = Number(ln.mode || 0)
        measuredPower = Number(ln.power || 0)
        measuredVoltage = Number(ln.voltage || 0)
        measuredCurrent = Number(ln.current || 0)
        measuredAvailable = Boolean(ln.powerAvailable || ln.voltageAvailable || ln.currentAvailable)
    }

    function refreshMeasurements() {
        if (typeof panel === "undefined")
            return

        var ln = panel.lineAt(lineIndex)
        if (!ln)
            return

        measuredPower = Number(ln.power || 0)
        measuredVoltage = Number(ln.voltage || 0)
        measuredCurrent = Number(ln.current || 0)
        measuredAvailable = Boolean(ln.powerAvailable || ln.voltageAvailable || ln.currentAvailable)
    }

    function saveLine() {
        if (typeof panel === "undefined")
            return

        panel.updateLine(lineIndex, {
            "description": lineNameField.text,
            "mpower": lineMpower,
            "tolerance": lineTolerance,
            "mode": lineMode
        })
        panel.applyLineModes()
        panel.saveLines()
    }

    Component.onCompleted: {
        loadLine()
        panel.setLineSetupActive(lineIndex, true)
    }
    Component.onDestruction: {
        if (panel.testRunning)
            panel.stopCurrentTest()
        panel.setLineSetupActive(lineIndex, false)
    }

    Connections {
        target: panel

        function onChanged() {
            lineEditPage.refreshMeasurements()
        }
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 20

        Text {
            text: "Настройка линии: " + lineDescription
            font.pixelSize: 30
        }

        GridLayout {
            columns: 2
            rowSpacing: 10
            columnSpacing: 10

            Text {
                text: "Описание:"
                font.pixelSize: 30
                Layout.alignment: Qt.AlignRight
            }

            Rectangle {
                width: 580
                height: 48
                color: "#ffffff"
                border.color: "#9fb5ca"
                border.width: 2
                radius: 4

                TextField {
                    id: lineNameField
                    anchors.fill: parent
                    anchors.margins: 2
                    text: lineDescription
                    font.pixelSize: 30
                    selectByMouse: true
                }
            }

            Rectangle {
                width: 250
                height: 90
                radius: 12
                color: panel.testRunning ? "#d84236" : "orange"

                Text {
                    anchors.centerIn: parent
                    text: panel.testRunning ? "Стоп" : "Тест"
                    font.pixelSize: 30
                    color: "white"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (panel.testRunning) {
                            panel.stopCurrentTest()
                        } else {
                            lineEditPage.saveLine()
                            panel.startFunctionalTest(10 * 60)
                        }
                    }
                }
            }

            Rectangle {
                width: 250
                height: 90
                radius: 12
                color: "orange"

                Text {
                    anchors.centerIn: parent
                    text: "ввести \nизмеренное"
                    font.pixelSize: 30
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (measuredAvailable)
                            lineMpower = measuredPower
                    }
                }
            }

            Text {
                Layout.row: 4
                text: "Установочная мощность, Вт:"
                font.pixelSize: 30
                Layout.alignment: Qt.AlignRight
            }

            RowLayout {
                spacing: 20

                Text {
                    font.pixelSize: 30
                    text: lineMpower.toFixed(1)
                }

                Button {
                    text: "ввести значение"
                    font.pixelSize: 30
                    onClicked: lineEditPage.inputRequested(
                                   "Введите мощность",
                                   "lineMpower",
                                   lineMpower,
                                   maxPower)
                }
            }

            Text {
                Layout.row: 5
                text: "Мощность, Вт:"
                font.pixelSize: 30
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: measuredAvailable ? measuredPower.toFixed(1) : "—"
                font.pixelSize: 30
            }

            Text {
                Layout.row: 6
                text: "Напряжение, В:"
                font.pixelSize: 30
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: measuredAvailable ? measuredVoltage.toFixed(1) : "—"
                font.pixelSize: 30
            }

            Text {
                Layout.row: 7
                text: "Ток, А:"
                font.pixelSize: 30
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: measuredAvailable ? measuredCurrent.toFixed(3) : "—"
                font.pixelSize: 30
            }

            Text {
                Layout.row: 8
                text: "Допуск, %"
                font.pixelSize: 30
                Layout.alignment: Qt.AlignRight
            }

            RowLayout {
                spacing: 20

                Text {
                    font.pixelSize: 30
                    text: lineTolerance.toFixed(0)
                }

                Button {
                    text: "ввести значение"
                    font.pixelSize: 30
                    onClicked: lineEditPage.inputRequested(
                                   "Введите допуск, %",
                                   "lineTolerance",
                                   lineTolerance,
                                   maxTolerance)
                }
            }

            Text {
                text: "Режим работы:"
                font.pixelSize: 35
                Layout.alignment: Qt.AlignRight
            }

            Rectangle {
                width: 400
                height: 40

                ComboBox {
                    id: control
                    anchors.fill: parent
                    font.pixelSize: 35

                    delegate: ItemDelegate {
                        width: control.width
                        text: modelData
                        font.weight: control.currentIndex === index ? Font.DemiBold : Font.Normal
                        highlighted: control.highlightedIndex === index
                        hoverEnabled: control.hoverEnabled
                    }

                    model: ["постоянный", "непостоянный", "линия отключена"]
                    currentIndex: lineMode

                    onActivated: function(index) {
                        lineMode = index
                    }
                }
            }
        }

        Rectangle {
            id: btnRet
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
                onEntered: btnRet.color = "lightgray"
                onExited: btnRet.color = "lightgray"
                onPressed: btnRet.color = "gray"
                onReleased: btnRet.color = "lightgray"
                onClicked: {
                    lineEditPage.saveLine()
                    lineEditPage.backRequested()
                }
            }
        }
    }
}
