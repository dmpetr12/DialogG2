import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()

    property string newPasswordBuffer: ""
    property string confirmPasswordBuffer: ""
    property string stage: "new"
    property string errorText: ""
    property int maxPasswordLength: 8

    color: "white"

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: stage === "new" ? "Введите новый пароль" : "Подтвердите новый пароль"
            font.pixelSize: 24
            horizontalAlignment: Text.AlignHCenter
            width: 320
        }

        Text {
            text: (stage === "new" ? newPasswordBuffer : confirmPasswordBuffer).length > 0
                  ? "*".repeat((stage === "new" ? newPasswordBuffer : confirmPasswordBuffer).length)
                  : ""
            font.pixelSize: 28
            horizontalAlignment: Text.AlignHCenter
            width: 200
        }

        Text {
            text: errorText
            color: "red"
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            width: 320
            visible: errorText.length > 0
        }

        GridLayout {
            columns: 3
            rowSpacing: 10
            columnSpacing: 10

            Repeater {
                model: 9
                delegate: Button {
                    text: index + 1
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 80
                    onClicked: appendDigit(text)
                }
            }

            Button { text: "Clear"; Layout.preferredWidth: 80; Layout.preferredHeight: 80; onClicked: clearCurrent() }
            Button { text: "0"; Layout.preferredWidth: 80; Layout.preferredHeight: 80; onClicked: appendDigit("0") }
            Button { text: "⌫"; Layout.preferredWidth: 80; Layout.preferredHeight: 80; onClicked: backspace() }
        }

        Row {
            spacing: 20

            Button { text: "Cancel"; width: 120; onClicked: root.backRequested() }
            Button {
                text: "OK"
                width: 120
                onClicked: acceptStage()
            }
        }
    }

    function currentBuffer() {
        return stage === "new" ? newPasswordBuffer : confirmPasswordBuffer
    }

    function appendDigit(digit) {
        errorText = ""
        if (stage === "new") {
            if (newPasswordBuffer.length < maxPasswordLength)
                newPasswordBuffer += digit
        } else if (confirmPasswordBuffer.length < maxPasswordLength) {
            confirmPasswordBuffer += digit
        }
    }

    function clearCurrent() {
        errorText = ""
        if (stage === "new")
            newPasswordBuffer = ""
        else
            confirmPasswordBuffer = ""
    }

    function backspace() {
        errorText = ""
        if (stage === "new" && newPasswordBuffer.length > 0)
            newPasswordBuffer = newPasswordBuffer.slice(0, -1)
        else if (stage === "confirm" && confirmPasswordBuffer.length > 0)
            confirmPasswordBuffer = confirmPasswordBuffer.slice(0, -1)
    }

    function acceptStage() {
        errorText = ""
        if (stage === "new") {
            if (newPasswordBuffer.length < 4) {
                errorText = "Пароль слишком короткий"
            } else {
                stage = "confirm"
            }
            return
        }

        if (confirmPasswordBuffer !== newPasswordBuffer) {
            errorText = "Пароли не совпадают"
            return
        }

        if (panel.changePassword(newPasswordBuffer))
            root.backRequested()
        else
            errorText = "Не удалось изменить пароль"
    }
}
