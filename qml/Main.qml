import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 800
    height: 480
    visible: true
    title: "Шкаф Dialog G2"
    color: "#eef3eb"

    readonly property color panelBg: "#f8faf6"
    readonly property color textMain: "#171a17"
    readonly property color muted: "#5c665c"
    readonly property color border: "#c9d5c7"

    header: Rectangle {
        height: 72
        color: panelBg

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            spacing: 16

            Rectangle {
                width: 54
                height: 42
                color: "#2f3833"

                Text {
                    anchors.centerIn: parent
                    text: "СТ"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }
            }

            Text {
                Layout.fillWidth: true
                text: "Шкаф аварийного освещения Dialog G2"
                color: textMain
                font.pixelSize: 26
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                text: Qt.formatDateTime(new Date(), "hh:mm  dd.MM.yyyy")
                color: textMain
                font.pixelSize: 24
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: panelBg
                border.color: border
                radius: 6

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 14

                    Text {
                        text: "Режим"
                        color: muted
                        font.pixelSize: 22
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 18

                        Rectangle {
                            width: 46
                            height: 46
                            radius: 23
                            color: cabinet.modeColor
                        }

                        Text {
                            Layout.fillWidth: true
                            text: cabinet.modeText
                            color: textMain
                            font.pixelSize: 46
                            font.bold: true
                            elide: Text.ElideRight
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: border
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 18

                        StatusButton {
                            Layout.fillWidth: true
                            title: "Система"
                            value: cabinet.healthText
                            accent: cabinet.healthColor
                        }

                        StatusButton {
                            Layout.fillWidth: true
                            title: "АКБ"
                            value: cabinet.batteryOk ? "Норма" : "Неисправность"
                            accent: cabinet.batteryOk ? "#0bbf63" : "#d93636"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 18

                        StatusButton {
                            Layout.fillWidth: true
                            title: "КН"
                            value: cabinet.voltageControlOk ? "Норма" : "Авария"
                            accent: cabinet.voltageControlOk ? "#0bbf63" : "#d93636"
                        }

                        StatusButton {
                            Layout.fillWidth: true
                            title: "Заряд"
                            value: cabinet.batteryPercent >= 0 ? cabinet.batteryPercent + "%" : "-"
                            accent: cabinet.batteryPercent > 30 ? "#0bbf63" : "#d99a00"
                        }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 250
                Layout.fillHeight: true
                color: panelBg
                border.color: border
                radius: 6

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 18

                    Text {
                        text: "Измерения"
                        color: muted
                        font.pixelSize: 22
                    }

                    MetricRow {
                        label: "Вход"
                        value: "-"
                        unit: "В"
                    }

                    MetricRow {
                        label: "Мощность"
                        value: "-"
                        unit: "Вт"
                    }

                    MetricRow {
                        label: "Утечка"
                        value: "-"
                        unit: "мА"
                    }

                    MetricRow {
                        label: "Темп."
                        value: "-"
                        unit: "°C"
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 74
            color: "#dce9da"
            radius: 6

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                NavButton { text: "Линии" }
                NavButton { text: "АКБ" }
                NavButton { text: "Журнал" }
                NavButton { text: "Настройки" }
                NavButton { text: "Расписание" }
            }
        }
    }

    component StatusButton: Rectangle {
        property string title
        property string value
        property color accent

        Layout.preferredHeight: 82
        color: "#ffffff"
        border.color: border
        radius: 6

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            Rectangle {
                width: 28
                height: 28
                radius: 14
                color: accent
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: title
                    color: muted
                    font.pixelSize: 18
                    elide: Text.ElideRight
                }

                Text {
                    text: value
                    color: textMain
                    font.pixelSize: 28
                    font.bold: true
                    elide: Text.ElideRight
                }
            }
        }
    }

    component MetricRow: RowLayout {
        property string label
        property string value
        property string unit

        Layout.fillWidth: true
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: label
            color: muted
            font.pixelSize: 21
            elide: Text.ElideRight
        }

        Text {
            text: value
            color: textMain
            font.pixelSize: 30
            font.bold: true
        }

        Text {
            text: unit
            color: muted
            font.pixelSize: 19
        }
    }

    component NavButton: Button {
        Layout.fillWidth: true
        Layout.fillHeight: true
        font.pixelSize: 22
        font.bold: true

        background: Rectangle {
            color: parent.down ? "#b8cbb6" : "#f8faf6"
            border.color: border
            radius: 6
        }

        contentItem: Text {
            text: parent.text
            color: textMain
            font: parent.font
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
