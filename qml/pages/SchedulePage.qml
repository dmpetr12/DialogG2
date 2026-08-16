import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()
    signal editRequested(int index)

    property int currentRow: -1
    property var scheduleModel: []

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648
    color: "white"

    function refresh() {
        scheduleModel = panel.getAllTests()
        if (currentRow >= scheduleModel.length)
            currentRow = -1
    }

    function weekDaysToText(weekDays) {
        if (!weekDays)
            return "-"

        var names = {
            "Mon": "Пн",
            "Tue": "Вт",
            "Wed": "Ср",
            "Thu": "Чт",
            "Fri": "Пт",
            "Sat": "Сб",
            "Sun": "Вс"
        }

        var arr = []
        for (var i = 0; i < weekDays.length; ++i)
            arr.push(names[weekDays[i]] || weekDays[i])

        return arr.length > 0 ? arr.join(", ") : "-"
    }

    Component.onCompleted: refresh()
    onVisibleChanged: if (visible) refresh()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

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
                text: "Расписание тестов"
                font.pixelSize: 36
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            ActionButton {
                text: "Добавить"
                enabled: true
                onClicked: {
                    var now = new Date()
                    var dateText = now.getFullYear() + "-" + ("0" + (now.getMonth() + 1)).slice(-2) + "-" + ("0" + now.getDate()).slice(-2)
                    panel.addTest({
                        "enabled": true,
                        "period": "один раз",
                        "startDate": dateText,
                        "startTime": "00:00",
                        "testType": "Функциональный",
                        "weekDays": []
                    })
                    root.refresh()
                }
            }

            ActionButton {
                text: "Удалить"
                enabled: root.currentRow >= 0
                onClicked: {
                    if (root.currentRow >= 0) {
                        panel.removeTest(root.currentRow)
                        root.currentRow = -1
                        root.refresh()
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            color: "#f2f2f2"
            border.color: "#d0d0d0"
            radius: 6

            RowLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 6

                HeaderCell { text: "Период"; Layout.preferredWidth: 170 }
                HeaderCell { text: "Дата"; Layout.preferredWidth: 150 }
                HeaderCell { text: "Время"; Layout.preferredWidth: 100 }
                HeaderCell { text: "Дни"; Layout.preferredWidth: 210 }
                HeaderCell { text: "Описание теста"; Layout.fillWidth: true }
                HeaderCell { text: ""; Layout.preferredWidth: 120 }
            }
        }

        ListView {
            id: list

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: root.scheduleModel
            currentIndex: root.currentRow

            delegate: Rectangle {
                width: list.width
                height: 60
                radius: 6
                color: index === root.currentRow ? "#eaf4ff" : "#ffffff"
                border.color: index === root.currentRow ? "#80b0ff" : "#cccccc"

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.currentRow = index
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    spacing: 6

                    ValueCell { text: modelData.period || "-"; Layout.preferredWidth: 170 }
                    ValueCell { text: modelData.startDate || "-"; Layout.preferredWidth: 150 }
                    ValueCell { text: modelData.startTime || "-"; Layout.preferredWidth: 100 }
                    ValueCell {
                        text: modelData.period === "дни недели" ? root.weekDaysToText(modelData.weekDays) : "-"
                        Layout.preferredWidth: 210
                    }
                    ValueCell { text: modelData.testType || "-"; Layout.fillWidth: true }

                    Rectangle {
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 54
                        radius: 6
                        color: "orange"

                        Text {
                            anchors.centerIn: parent
                            text: "настр."
                            color: "black"
                            font.pixelSize: 25
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.editRequested(index)
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

    component HeaderCell: Text {
        font.pixelSize: 24
        color: "black"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    component ValueCell: Text {
        font.pixelSize: 24
        color: "black"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    component ActionButton: Rectangle {
        signal clicked()
        property alias text: label.text

        Layout.preferredWidth: 170
        Layout.preferredHeight: 72
        radius: 10
        color: enabled ? "orange" : "#9e9e9e"

        Text {
            id: label
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 26
        }

        MouseArea {
            anchors.fill: parent
            enabled: parent.enabled
            onClicked: parent.clicked()
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
