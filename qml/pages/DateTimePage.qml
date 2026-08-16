import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: timeChange

    signal backRequested()
    signal inputRequested(string nameText, string propertyName, real initialValue, int maxValue)

    property int year: new Date().getFullYear()
    property int month: new Date().getMonth() + 1
    property int day: new Date().getDate()
    property int hour: new Date().getHours()
    property int minute: new Date().getMinutes()

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 500
        anchors.margins: 12
        color: "transparent"

        Column {
            topPadding: 150
            leftPadding: 20
            spacing: 20

            Text {
                text: "Установка системного времени"
                font.pixelSize: 40
                horizontalAlignment: Text.AlignHCenter
            }

            Row {
                spacing: 30

                DateInput { value: timeChange.day; label: "день" }
                DateInput { value: timeChange.month; label: "месяц" }
                DateInput { value: timeChange.year; label: "год" }

                Column {
                    spacing: 20
                    Text {
                        font.pixelSize: 50
                        text: "   "
                    }
                }

                DateInput { value: timeChange.hour; label: "часы"; suffix: "   :" }
                DateInput { value: timeChange.minute; label: "минуты" }
            }

            Button {
                text: "Установить системное время"
                width: 400
                height: 70
                font.pixelSize: 22
                onClicked: {
                    var d = new Date(timeChange.year, timeChange.month - 1, timeChange.day, timeChange.hour, timeChange.minute, 0)
                    panel.setSystemTime(d.getTime())
                }
            }
        }
    }

    BackButton {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 12
        onClicked: timeChange.backRequested()
    }

    component DateInput: Column {
        property int value: 0
        property string label: ""
        property string suffix: ""

        spacing: 20

        Text {
            font.pixelSize: 50
            text: parent.suffix.length > 0 ? parent.value + parent.suffix : parent.value
        }

        Button {
            text: parent.label
            font.pixelSize: 50
            onClicked: {
                if (parent.label === "день")
                    timeChange.inputRequested("Введите день", "day", timeChange.day, 31)
                else if (parent.label === "месяц")
                    timeChange.inputRequested("Введите месяц", "month", timeChange.month, 12)
                else if (parent.label === "год")
                    timeChange.inputRequested("Введите полный год", "year", timeChange.year, 2100)
                else if (parent.label === "часы")
                    timeChange.inputRequested("Введите часы", "hour", timeChange.hour, 23)
                else if (parent.label === "минуты")
                    timeChange.inputRequested("Введите минуты", "minute", timeChange.minute, 59)
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
