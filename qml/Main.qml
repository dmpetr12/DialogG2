import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import "pages"

ApplicationWindow {
    id: window

    readonly property bool debugWindow: panel.logLevel === "DEBUG"

    width: debugWindow ? 1024 : Screen.width
    height: debugWindow ? 768 : Screen.height
    minimumWidth: debugWindow ? 1024 : 0
    minimumHeight: debugWindow ? 768 : 0
    maximumWidth: debugWindow ? 1024 : Screen.width
    maximumHeight: debugWindow ? 768 : Screen.height
    visible: true
    visibility: debugWindow ? Window.Windowed : Window.FullScreen
    flags: debugWindow ? Qt.Window : (Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
    title: "Щит аварийного освещения Dialog G2"
    color: "#ffffff"

    property date now: new Date()
    property bool unlocked: false
    property int selectedLineIndex: 0
    property int selectedScheduleIndex: -1
    property string lastMaintenancePopupDate: ""
    property bool startupMaintenanceChecked: false
    readonly property var maintenance: panel.maintenance
    readonly property int idleTimeoutMs: 10 * 60 * 1000

    Timer {
        interval: 1000
        repeat: true
        running: true
        onTriggered: window.now = new Date()
    }

    Timer {
        id: accessIdleTimer
        interval: window.idleTimeoutMs
        repeat: false
        running: window.unlocked

        onTriggered: {
            window.unlocked = false
            pageStack.replace(startPageComponent)
        }
    }

    Timer {
        id: startupMaintenanceTimer
        interval: 60 * 1000
        repeat: true
        running: !window.startupMaintenanceChecked
        onTriggered: window.startupMaintenanceChecked = window.showMaintenancePopup(false)
    }

    Timer {
        id: dailyMaintenanceTimer
        interval: 30 * 1000
        repeat: true
        running: true
        onTriggered: {
            var now = new Date()
            var today = Qt.formatDate(now, "yyyy-MM-dd")
            if (now.getHours() === 8 && now.getMinutes() === 0 &&
                    window.lastMaintenancePopupDate !== today)
                window.showMaintenancePopup(true)
        }
    }

    onUnlockedChanged: {
        if (unlocked)
            accessIdleTimer.restart()
        else
            accessIdleTimer.stop()
    }

    function maintenanceDateText(value) {
        if (value === undefined || value === null || value === "")
            return "не проводился"

        var dt = new Date(value)
        if (isNaN(dt.getTime()))
            return "не проводился"

        return Qt.formatDate(dt, "dd.MM.yyyy")
    }

    function maintenanceDetailsText() {
        var m = window.maintenance || {}
        var parts = []

        if (m.longTestOverdue)
            parts.push("Тест длительности:\n" + maintenanceDateText(m.lastLongTestAt))

        if (m.summary) {
            var summaryParts = m.summary.split("; ").filter(function(part) {
                return part.indexOf("Тест длительности") !== 0
            })
            if (summaryParts.length > 0)
                parts.push(summaryParts.join("; "))
        }

        var lines = m.lines || []
        var overdue = []
        for (var i = 0; i < lines.length; ++i) {
            var line = lines[i]
            if (line && line.overdue) {
                var name = line.lineName || ("Линия " + line.lineIndex)
                overdue.push(name + ": " + maintenanceDateText(line.lastTestAt))
            }
        }

        if (overdue.length > 0)
            parts.push("Функциональные проверки:\n" + overdue.join("\n"))

        return parts.length > 0 ? parts.join("\n\n") : "Есть просроченные проверки."
    }

    function showMaintenancePopup(markDaily) {
        var m = window.maintenance || {}
        if (m.ok === undefined)
            return false

        if (m.ok !== false)
            return true

        window.lastMaintenancePopupDate = Qt.formatDate(new Date(), "yyyy-MM-dd")

        maintenanceText.text = maintenanceDetailsText()
        maintenancePopup.open()
        return true
    }

    Rectangle {
        id: headerBand

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 120
        color: "#c5dfc0"

        Image {
            anchors.left: parent.left
            anchors.leftMargin: -40
            anchors.top: parent.top
            anchors.topMargin: -5
            width: 400
            height: 100
            source: "assets/light-tech-logo.png"
            fillMode: Image.PreserveAspectFit
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 26
            anchors.top: parent.top
            anchors.topMargin: 20
            spacing: 34

            Text {
                text: panel.temperature.toFixed(0) + "°C"
                color: "#111111"
                font.pixelSize: 41
                font.family: "Arial"
            }

            Text {
                text: Qt.formatDateTime(window.now, "hh:mm")
                color: "#111111"
                font.pixelSize: 41
                font.family: "Arial"
            }

            Text {
                text: Qt.formatDateTime(window.now, "dd.MM.yyyy")
                color: "#111111"
                font.pixelSize: 41
                font.family: "Arial"
            }
        }

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2
            text: "ЩИТ АВАРИЙНОГО ОСВЕЩЕНИЯ \"ДИАЛОГ G2\""
            color: "#111111"
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 35
            font.family: "Arial"
        }
    }

    StackView {
        id: pageStack

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: headerBand.bottom
        anchors.bottom: parent.bottom

        initialItem: startPageComponent

        pushEnter: null
        pushExit: null
        popEnter: null
        popExit: null
        replaceEnter: null
        replaceExit: null
    }

    Rectangle {
        id: backendWarning

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: headerBand.bottom
        height: 64
        z: 900
        visible: !panel.connected
        color: "#b00020"

        Text {
            anchors.centerIn: parent
            text: "НЕТ СВЯЗИ С BACKEND. ДАННЫЕ НА ЭКРАНЕ МОГУТ БЫТЬ УСТАРЕВШИМИ"
            color: "#ffffff"
            font.pixelSize: 25
            font.family: "Arial"
            font.bold: true
        }
    }

    MouseArea {
        anchors.fill: parent
        z: 1000
        hoverEnabled: true
        propagateComposedEvents: true

        onPressed: function(mouse) {
            if (window.unlocked)
                accessIdleTimer.restart()
            mouse.accepted = false
        }

        onReleased: function(mouse) {
            if (window.unlocked)
                accessIdleTimer.restart()
            mouse.accepted = false
        }

        onClicked: function(mouse) {
            if (window.unlocked)
                accessIdleTimer.restart()
            mouse.accepted = false
        }
    }

    Popup {
        id: digitalPopup

        parent: Overlay.overlay
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        x: Math.round((window.width - width) / 2)
        y: Math.round((window.height - height) / 2)
        width: 360
        height: 450
        margins: 20

        property var targetObject: null
        property string targetProperty: ""
        property string nameP: ""
        property int maxV: 1000

        function openFor(nameText, target, propName, initialValue, maxValue) {
            nameP = nameText
            targetObject = target
            targetProperty = propName
            powerField.text = ""
            maxV = maxValue
            open()
        }

        Rectangle {
            anchors.fill: parent
            color: "#333333"
            radius: 10

            Column {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Text {
                    text: digitalPopup.nameP
                    color: "white"
                    font.pixelSize: 30
                }

                TextField {
                    id: powerField
                    text: ""
                    readOnly: true
                    font.pixelSize: 40
                    inputMethodHints: Qt.ImhPreferNumbers
                }

                GridLayout {
                    columns: 3
                    rowSpacing: 4
                    columnSpacing: 4

                    Repeater {
                        model: ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0", ",", "←"]

                        delegate: Button {
                            text: modelData
                            font.pixelSize: 34

                            onClicked: {
                                if (text === "←")
                                    powerField.text = powerField.text.slice(0, -1)
                                else
                                    powerField.text += text
                            }
                        }
                    }
                }

                Row {
                    spacing: 12

                    Button {
                        text: "Отмена"
                        font.pixelSize: 40
                        onClicked: digitalPopup.close()
                    }

                    Button {
                        text: " OK "
                        font.pixelSize: 40

                        onClicked: {
                            var cont = true
                            var v = parseFloat(powerField.text.replace(",", "."))

                            if (!isNaN(v) && v < 1 &&
                                    digitalPopup.targetProperty !== "minute" &&
                                    digitalPopup.targetProperty !== "hour" &&
                                    digitalPopup.targetProperty !== "mpower")
                                cont = false

                            if (!isNaN(v) && v < 2025 &&
                                    digitalPopup.targetProperty === "year")
                                cont = false

                            if (cont && !isNaN(v) &&
                                    digitalPopup.targetObject &&
                                    digitalPopup.targetProperty !== "")
                                digitalPopup.targetObject[digitalPopup.targetProperty] =
                                        digitalPopup.maxV > v ? v : digitalPopup.maxV

                            digitalPopup.close()
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: maintenancePopup

        parent: Overlay.overlay
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        x: Math.round((window.width - width) / 2)
        y: Math.round((window.height - height) / 2)
        width: 660
        height: 500
        margins: 20

        Rectangle {
            anchors.fill: parent
            radius: 10
            color: "#fff0f0"
            border.color: "#c62828"
            border.width: 3

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 16

                Text {
                    Layout.fillWidth: true
                    text: "ВНИМАНИЕ: просрочены проверки"
                    color: "#b00020"
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 34
                    font.family: "Arial"
                    font.bold: true
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    Text {
                        id: maintenanceText
                        width: maintenancePopup.width - 72
                        color: "#b00020"
                        wrapMode: Text.WordWrap
                        font.pixelSize: 24
                        font.family: "Arial"
                        font.bold: true
                    }
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter
                    width: 220
                    height: 72
                    text: "Закрыть"
                    font.pixelSize: 30
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 30
                        font.family: "Arial"
                    }
                    background: Rectangle {
                        radius: 8
                        color: "#c62828"
                    }
                    onClicked: maintenancePopup.close()
                }
            }
        }
    }

    Component {
        id: startPageComponent

        StartPage {
            unlocked: window.unlocked
            onLoginRequested: pageStack.replace(passwordPageComponent)
            onLogoutRequested: window.unlocked = false
            onTestRequested: pageStack.replace(testPageComponent)
            onSettingsRequested: pageStack.replace(settingsPageComponent)
            onScheduleRequested: pageStack.replace(schedulePageComponent)
            onJournalRequested: {
                pageStack.replace(journalPageComponent)
                window.showMaintenancePopup(false)
            }
            onSystemRequested: pageStack.replace(systemPageComponent)
            onLinesRequested: pageStack.replace(linesPageComponent)
            onBatteryRequested: pageStack.replace(batteryPageComponent)
        }
    }

    Component {
        id: passwordPageComponent

        PasswordPage {
            onAccepted: {
                window.unlocked = true
                pageStack.replace(startPageComponent)
            }
            onRejected: {
                window.unlocked = false
                pageStack.replace(startPageComponent)
            }
            onCanceled: pageStack.replace(startPageComponent)
            onForgotPasswordRequested: pageStack.replace(forgotPasswordChangePageComponent)
        }
    }

    Component {
        id: testPageComponent

        TestPage {
            id: testPage

            unlocked: window.unlocked
            onBackRequested: pageStack.replace(startPageComponent)
            onInputRequested: function(nameText, propertyName, initialValue, maxValue) {
                digitalPopup.openFor(nameText, testPage, propertyName, initialValue, maxValue)
            }
        }
    }

    Component {
        id: settingsPageComponent

        SettingsPage {
            onBackRequested: pageStack.replace(startPageComponent)
            onDateTimeRequested: pageStack.replace(dateTimePageComponent)
            onPasswordRequested: pageStack.replace(passwordChangePageComponent)
            onLineRequested: function(index) {
                window.selectedLineIndex = index
                pageStack.replace(lineSettingsPageComponent)
            }
        }
    }

    Component {
        id: lineSettingsPageComponent

        LineSettingsPage {
            id: lineSettingsPage

            lineIndex: window.selectedLineIndex
            onBackRequested: pageStack.replace(settingsPageComponent)
            onInputRequested: function(nameText, propertyName, initialValue, maxValue) {
                digitalPopup.openFor(nameText, lineSettingsPage, propertyName, initialValue, maxValue)
            }
        }
    }

    Component {
        id: passwordChangePageComponent

        PasswordChangePage {
            onBackRequested: pageStack.replace(settingsPageComponent)
        }
    }

    Component {
        id: schedulePageComponent

        SchedulePage {
            onBackRequested: pageStack.replace(startPageComponent)
            onEditRequested: function(index) {
                window.selectedScheduleIndex = index
                pageStack.replace(testSettingsPageComponent)
            }
        }
    }

    Component {
        id: journalPageComponent

        JournalPage {
            onBackRequested: pageStack.replace(startPageComponent)
        }
    }

    Component {
        id: systemPageComponent

        SystemPage {
            onBackRequested: pageStack.replace(startPageComponent)
        }
    }

    Component {
        id: linesPageComponent

        LinesPage {
            onBackRequested: pageStack.replace(startPageComponent)
        }
    }

    Component {
        id: batteryPageComponent

        BatteryPage {
            onBackRequested: pageStack.replace(startPageComponent)
        }
    }

    Component {
        id: testSettingsPageComponent

        TestSettingsPage {
            id: testSettingsPage

            currentIndex: window.selectedScheduleIndex
            onBackRequested: pageStack.replace(schedulePageComponent)
            onInputRequested: function(nameText, propertyName, initialValue, maxValue) {
                digitalPopup.openFor(nameText, testSettingsPage, propertyName, initialValue, maxValue)
            }
        }
    }

    Component {
        id: forgotPasswordChangePageComponent

        PasswordChangePage {
            onBackRequested: pageStack.replace(startPageComponent)
        }
    }

    Component {
        id: dateTimePageComponent

        DateTimePage {
            id: dateTimePage

            onBackRequested: pageStack.replace(settingsPageComponent)
            onInputRequested: function(nameText, propertyName, initialValue, maxValue) {
                digitalPopup.openFor(nameText, dateTimePage, propertyName, initialValue, maxValue)
            }
        }
    }
}
