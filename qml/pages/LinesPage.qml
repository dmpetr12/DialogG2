import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648
    color: "white"

    function modeText(mode) {
        if (mode === 0)
            return "ОТКЛ"
        if (mode === 1)
            return "ПОСТ"
        return "НЕПОСТ"
    }

    function numberText(value, available, digits, suffix) {
        if (available !== undefined && !available)
            return "-"
        if (value === undefined || isNaN(value))
            return "-"
        return Number(value).toFixed(digits) + " " + suffix
    }

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
                onClicked: root.backRequested()
            }

            Text {
                text: "Линии"
                color: "#111111"
                font.pixelSize: 40
                font.family: "Arial"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            Rectangle {
                Layout.preferredWidth: 150
                Layout.preferredHeight: 82
                radius: 6
                color: panel.linesOk ? "#11bf5d" : "#d84236"

                Text {
                    anchors.centerIn: parent
                    text: panel.linesOk ? "ИСПР" : "АВАР"
                    color: "#ffffff"
                    font.pixelSize: 30
                    font.bold: true
                    font.family: "Arial"
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ffffff"

            Text {
                anchors.centerIn: parent
                visible: panel.lines.length === 0
                text: "Линии не настроены"
                color: "#666666"
                font.pixelSize: 34
                font.family: "Arial"
            }

            ListView {
                id: lineList

                anchors.fill: parent
                clip: true
                spacing: 10
                model: panel.lines
                visible: panel.lines.length > 0

                delegate: Rectangle {
                    width: lineList.width - 26
                    height: 128
                    x: 0
                    radius: 6
                    color: "#f7f7f7"
                    border.color: "#d0d0d0"
                    border.width: 2

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 18

                        Rectangle {
                            Layout.preferredWidth: 56
                            Layout.preferredHeight: 56
                            radius: 28
                            color: modelData.mode === 0 ? "#b6b6b6" : "#11bf5d"

                            Text {
                                anchors.centerIn: parent
                                text: index + 1
                                color: "#ffffff"
                                font.pixelSize: 28
                                font.bold: true
                                font.family: "Arial"
                            }
                        }

                        ColumnLayout {
                            Layout.preferredWidth: 290
                            Layout.fillHeight: true
                            spacing: 6

                            Text {
                                Layout.fillWidth: true
                                text: modelData.description || ("Линия " + (index + 1))
                                color: "#111111"
                                font.pixelSize: 29
                                font.bold: true
                                font.family: "Arial"
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                text: root.modeText(modelData.mode) + "  ном. " + root.numberText(modelData.mpower, true, 0, "Вт")
                                color: "#555555"
                                font.pixelSize: 24
                                font.family: "Arial"
                                elide: Text.ElideRight
                            }
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            columns: 2
                            rowSpacing: 4
                            columnSpacing: 18

                            MetricText {
                                title: "P"
                                value: root.numberText(modelData.power, modelData.powerAvailable, 0, "Вт")
                            }

                            MetricText {
                                title: "U"
                                value: root.numberText(modelData.voltage, modelData.voltageAvailable, 0, "В")
                            }

                            MetricText {
                                title: "I"
                                value: root.numberText(modelData.current, modelData.currentAvailable, 2, "А")
                            }

                            MetricText {
                                title: "Утечка"
                                value: root.numberText(modelData.leakage, true, 1, "мА")
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    width: 28
                }
            }
        }
    }

    component MetricText: RowLayout {
        property string title: ""
        property string value: ""

        spacing: 8

        Text {
            text: title
            color: "#555555"
            font.pixelSize: 24
            font.family: "Arial"
            Layout.preferredWidth: 70
            elide: Text.ElideRight
        }

        Text {
            text: value
            color: "#111111"
            font.pixelSize: 28
            font.bold: true
            font.family: "Arial"
            Layout.fillWidth: true
            elide: Text.ElideRight
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
