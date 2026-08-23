import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal accepted()
    signal rejected()
    signal canceled()
    signal forgotPasswordRequested()

    property string passwordBuffer: ""
    property int forgotPasswordTapCount: 0
    color: "white"

    Component.onDestruction: forgotPasswordTapCount = 0

    Column {
        anchors.centerIn: parent
        spacing: 20

        Item {
            width: 260
            height: 54

            Text {
                id: titleLabel
                anchors.fill: parent
                text: "Введите пароль"
                color: "#111111"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 28
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    root.forgotPasswordTapCount += 1
                    if (root.forgotPasswordTapCount >= 15) {
                        root.forgotPasswordTapCount = 0
                        root.passwordBuffer = ""
                        root.forgotPasswordRequested()
                    }
                }
            }
        }

        Text {
            id: passwordDisplay
            text: "*".repeat(root.passwordBuffer.length)
            font.pixelSize: 28
            horizontalAlignment: Text.AlignHCenter
            width: 260
        }

        GridLayout {
            id: keypad
            columns: 3
            rowSpacing: 10
            columnSpacing: 10

            Repeater {
                model: 9

                delegate: Button {
                    text: index + 1
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 80
                    font.pixelSize: 30
                    contentItem: Text {
                        text: parent.text
                        color: "#111111"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 30
                    }
                    background: Rectangle {
                        radius: 4
                        color: parent.pressed ? "#d6d6d6" : "#f3f3f3"
                        border.color: "#9a9a9a"
                        border.width: 1
                    }
                    onClicked: root.passwordBuffer += text
                }
            }

            Button {
                text: "Clear"
                Layout.preferredWidth: 80
                Layout.preferredHeight: 80
                font.pixelSize: 23
                contentItem: Text {
                    text: parent.text
                    color: "#111111"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 23
                }
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#d6d6d6" : "#f3f3f3"
                    border.color: "#9a9a9a"
                    border.width: 1
                }
                onClicked: root.passwordBuffer = ""
            }

            Button {
                text: "0"
                Layout.preferredWidth: 80
                Layout.preferredHeight: 80
                font.pixelSize: 30
                contentItem: Text {
                    text: parent.text
                    color: "#111111"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 30
                }
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#d6d6d6" : "#f3f3f3"
                    border.color: "#9a9a9a"
                    border.width: 1
                }
                onClicked: root.passwordBuffer += "0"
            }

            Button {
                text: "⌫"
                Layout.preferredWidth: 80
                Layout.preferredHeight: 80
                font.pixelSize: 30
                contentItem: Text {
                    text: parent.text
                    color: "#111111"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 30
                }
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#d6d6d6" : "#f3f3f3"
                    border.color: "#9a9a9a"
                    border.width: 1
                }
                onClicked: {
                    if (root.passwordBuffer.length > 0)
                        root.passwordBuffer = root.passwordBuffer.slice(0, -1)
                }
            }
        }

        Row {
            spacing: 10

            Button {
                text: "ОТМЕНА"
                width: 150
                height: 60
                font.pixelSize: 26
                contentItem: Text {
                    text: parent.text
                    color: "#111111"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 26
                }
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#d6d6d6" : "#f3f3f3"
                    border.color: "#9a9a9a"
                    border.width: 1
                }

                onClicked: {
                    root.passwordBuffer = ""
                    root.canceled()
                }
            }

            Button {
                text: "OK"
                width: 150
                height: 60
                font.pixelSize: 28
                contentItem: Text {
                    text: parent.text
                    color: "#111111"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 28
                }
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#d6d6d6" : "#f3f3f3"
                    border.color: "#9a9a9a"
                    border.width: 1
                }

                onClicked: {
                    if (panel.checkPassword(root.passwordBuffer)) {
                        root.passwordBuffer = ""
                        root.accepted()
                    } else {
                        root.passwordBuffer = ""
                        root.rejected()
                    }
                }
            }
        }
    }
}
