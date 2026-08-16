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
                    onClicked: root.passwordBuffer += text
                }
            }

            Button {
                text: "Clear"
                Layout.preferredWidth: 80
                Layout.preferredHeight: 80
                onClicked: root.passwordBuffer = ""
            }

            Button {
                text: "0"
                Layout.preferredWidth: 80
                Layout.preferredHeight: 80
                onClicked: root.passwordBuffer += "0"
            }

            Button {
                text: "⌫"
                Layout.preferredWidth: 80
                Layout.preferredHeight: 80
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
                width: 125
                height: 60

                onClicked: {
                    root.passwordBuffer = ""
                    root.canceled()
                }
            }

            Button {
                text: "OK"
                width: 125
                height: 60

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
