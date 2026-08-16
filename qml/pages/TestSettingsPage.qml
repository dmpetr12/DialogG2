import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()
    signal inputRequested(string nameText, string propertyName, int initialValue, int maxValue)

    property int currentIndex: -1
    property var entry: ({})
    property alias year: timeChange.year
    property alias month: timeChange.month
    property alias day: timeChange.day
    property alias hour: timeChange.hour
    property alias minute: timeChange.minute

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648
    color: "white"

    function loadEntry() {
        var all = panel.getAllTests()
        entry = currentIndex >= 0 && currentIndex < all.length ? all[currentIndex] : ({})
        syncUiFromEntry()
    }

    function syncUiFromEntry() {
        var now = new Date()

        periodBox.currentIndex = Math.max(0, periodBox.model.indexOf(entry.period || "один раз"))
        testTypeBox.currentIndex = Math.max(0, testTypeBox.model.indexOf(entry.testType || "Функциональный"))

        timeChange.ready = false
        weekRow.ready = false

        timeChange.year = now.getFullYear()
        timeChange.month = now.getMonth() + 1
        timeChange.day = now.getDate()
        timeChange.hour = now.getHours()
        timeChange.minute = now.getMinutes()

        if (entry.startDate && entry.startDate.length === 10) {
            var d = entry.startDate.split("-")
            if (d.length === 3) {
                timeChange.year = parseInt(d[0])
                timeChange.month = parseInt(d[1])
                timeChange.day = parseInt(d[2])
            }
        }

        if (entry.startTime && entry.startTime.length >= 5) {
            var t = entry.startTime.split(":")
            if (t.length >= 2) {
                timeChange.hour = parseInt(t[0])
                timeChange.minute = parseInt(t[1])
            }
        }

        weekRow.localDays = entry.weekDays ? entry.weekDays.slice(0) : []

        timeChange.ready = true
        weekRow.ready = true
    }

    Component.onCompleted: loadEntry()
    onVisibleChanged: if (visible) loadEntry()
    onCurrentIndexChanged: loadEntry()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 18

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
                text: "Настройка теста"
                font.pixelSize: 38
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            Item { Layout.preferredWidth: 82 }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 2
            columnSpacing: 28
            rowSpacing: 22

            FieldLabel { text: "Периодичность:" }

            ComboBox {
                id: periodBox
                Layout.preferredWidth: 600
                Layout.preferredHeight: 58
                font.pixelSize: 30
                model: ["один раз", "ежедневно", "дни недели", "раз в месяц", "раз в 3 месяца", "раз в полгода", "раз в год"]

                onActivated: {
                    if (root.currentIndex < 0)
                        return

                    var value = model[currentIndex]
                    panel.updateTestProperty(root.currentIndex, "period", value)
                    entry.period = value

                    if (value !== "дни недели") {
                        weekRow.localDays = []
                        weekRow.ready = false
                        panel.updateWeekDays(root.currentIndex, [])
                        entry.weekDays = []
                        weekRow.ready = true
                    }
                }
            }

            FieldLabel {
                text: "Дата и время\nпервого старта:"
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
            }

            ColumnLayout {
                id: timeChange
                spacing: 18
                Layout.fillWidth: true

                property int year: new Date().getFullYear()
                property int month: new Date().getMonth() + 1
                property int day: new Date().getDate()
                property int hour: new Date().getHours()
                property int minute: new Date().getMinutes()
                property bool ready: false

                function saveDate() {
                    if (!ready || root.currentIndex < 0)
                        return

                    month = Math.max(1, Math.min(month, 12))
                    day = Math.max(1, Math.min(day, 31))

                    var s = year + "-" + ("0" + month).slice(-2) + "-" + ("0" + day).slice(-2)
                    panel.updateTestProperty(root.currentIndex, "startDate", s)
                    entry.startDate = s
                }

                function saveTime() {
                    if (!ready || root.currentIndex < 0)
                        return

                    hour = Math.max(0, Math.min(hour, 23))
                    minute = Math.max(0, Math.min(minute, 59))

                    var s = ("0" + hour).slice(-2) + ":" + ("0" + minute).slice(-2)
                    panel.updateTestProperty(root.currentIndex, "startTime", s)
                    entry.startTime = s
                }

                onYearChanged: saveDate()
                onMonthChanged: saveDate()
                onDayChanged: saveDate()
                onHourChanged: saveTime()
                onMinuteChanged: saveTime()

                RowLayout {
                    spacing: 10

                    Text { text: "Дата:"; font.pixelSize: 30; color: "black" }
                    NumberBox { valueText: ("0" + timeChange.day).slice(-2); onClicked: root.inputRequested("Введите день", "day", timeChange.day, 31) }
                    NumberBox { valueText: ("0" + timeChange.month).slice(-2); onClicked: root.inputRequested("Введите месяц", "month", timeChange.month, 12) }
                    NumberBox { widthValue: 130; valueText: timeChange.year; onClicked: root.inputRequested("Введите год", "year", timeChange.year, 2100) }
                }

                RowLayout {
                    spacing: 10

                    Text { text: "Время:"; font.pixelSize: 30; color: "black" }
                    NumberBox { valueText: ("0" + timeChange.hour).slice(-2); onClicked: root.inputRequested("Введите час", "hour", timeChange.hour, 23) }
                    NumberBox { valueText: ("0" + timeChange.minute).slice(-2); onClicked: root.inputRequested("Введите минуты", "minute", timeChange.minute, 59) }
                }

                ColumnLayout {
                    spacing: 10
                    Layout.fillWidth: true
                    enabled: periodBox.currentText === "дни недели"
                    opacity: periodBox.currentText === "дни недели" ? 1.0 : 0.35

                    Text {
                        text: "Дни недели:"
                        font.pixelSize: 30
                        color: "black"
                    }

                    RowLayout {
                        id: weekRow
                        spacing: 10
                        Layout.fillWidth: true

                        property bool ready: false
                        property var localDays: []
                        property var daysList: [
                            { short: "Пн", key: "Mon" },
                            { short: "Вт", key: "Tue" },
                            { short: "Ср", key: "Wed" },
                            { short: "Чт", key: "Thu" },
                            { short: "Пт", key: "Fri" },
                            { short: "Сб", key: "Sat" },
                            { short: "Вс", key: "Sun" }
                        ]

                        function saveWeekDays(days) {
                            if (root.currentIndex < 0)
                                return

                            localDays = days
                            panel.updateWeekDays(root.currentIndex, days)
                            entry.weekDays = days
                        }

                        Repeater {
                            model: weekRow.daysList

                            CheckBox {
                                id: dayCheck
                                text: modelData.short
                                font.pixelSize: 30
                                checked: weekRow.localDays.indexOf(modelData.key) !== -1

                                indicator: Rectangle {
                                    implicitWidth: 30
                                    implicitHeight: 30
                                    radius: 6
                                    border.width: 2
                                    border.color: dayCheck.checked ? "orange" : "#808080"
                                    color: dayCheck.checked ? "orange" : "white"
                                }

                                onToggled: {
                                    if (!weekRow.ready)
                                        return

                                    var arr = weekRow.localDays.slice(0)
                                    var pos = arr.indexOf(modelData.key)

                                    if (checked && pos === -1)
                                        arr.push(modelData.key)
                                    if (!checked && pos !== -1)
                                        arr.splice(pos, 1)

                                    weekRow.saveWeekDays(arr)
                                }
                            }
                        }
                    }
                }
            }

            FieldLabel { text: "Тип теста:" }

            ComboBox {
                id: testTypeBox
                Layout.preferredWidth: 600
                Layout.preferredHeight: 58
                font.pixelSize: 32
                model: ["Функциональный", "На время"]

                onActivated: {
                    if (root.currentIndex < 0)
                        return

                    var value = model[currentIndex]
                    panel.updateTestProperty(root.currentIndex, "testType", value)
                    entry.testType = value
                }
            }
        }
    }

    component FieldLabel: Text {
        font.pixelSize: 34
        color: "black"
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
    }

    component NumberBox: Rectangle {
        signal clicked()
        property alias valueText: valueLabel.text
        property int widthValue: 88

        Layout.preferredWidth: widthValue
        Layout.preferredHeight: 70
        radius: 10
        color: "lightgrey"

        Text {
            id: valueLabel
            anchors.centerIn: parent
            font.pixelSize: 38
            color: "black"
        }

        MouseArea {
            anchors.fill: parent
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
