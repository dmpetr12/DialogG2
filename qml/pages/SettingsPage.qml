import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()
    signal dateTimeRequested()
    signal passwordRequested()
    signal lineRequested(int index)

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648
    color: "white"

    Rectangle {
        width: parent.width
        height: 550

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Text {
                text: "          Настройка линий"
                font.pixelSize: 40
                color: "black"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                height: 40
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                ColumnLayout {
                    Layout.preferredWidth: 280
                    spacing: 20
                    Layout.topMargin: 60

                    Rectangle {
                        width: 250
                        height: 90
                        radius: 12
                        color: "orange"
                        Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

                        Text {
                            anchors.centerIn: parent
                            text: "Настройка даты\nи времени"
                            font.pixelSize: 30
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.dateTimeRequested()
                        }
                    }

                    Rectangle {
                        width: 250
                        height: 90
                        radius: 12
                        color: panel.lineCount < 25 ? "orange" : "#9e9e9e"
                        Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

                        Text {
                            anchors.centerIn: parent
                            text: "Добавить\nлинию"
                            font.pixelSize: 30
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: panel.lineCount < 25
                            onClicked: panel.addLine()
                        }
                    }

                    Rectangle {
                        width: 250
                        height: 90
                        radius: 12
                        color: "orange"
                        Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

                        Text {
                            anchors.centerIn: parent
                            text: "Сменить пароль"
                            font.pixelSize: 30
                            color: "white"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.passwordRequested()
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 10

                    Text {
                        text: "настроить линию"
                        font.pixelSize: 30
                        color: "white"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    ListView {
                        id: listView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: panel.lines
                        spacing: 0
                        reuseItems: true

                        delegate: Item {
                            width: ListView.view.width
                            visible: true
                            height: 80

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10

                                Rectangle {
                                    width: 470
                                    height: 70
                                    radius: 3
                                    color: "#E7E7E7"

                                    Text {
                                        text: (index + 1) + ": " + (modelData.description || ("Линия " + (index + 1)))
                                        font.pixelSize: 30
                                        anchors.fill: parent
                                        color: "black"
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }
                                }

                                Rectangle {
                                    width: 165
                                    height: 70
                                    radius: 3
                                    color: "orange"

                                    Text {
                                        text: "  настроить"
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: 30
                                        color: "black"
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: root.lineRequested(index)
                                    }
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AlwaysOn
                            width: 30
                        }
                    }
                }
            }
        }
    }

    BackButton {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 12
        onClicked: root.backRequested()
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
