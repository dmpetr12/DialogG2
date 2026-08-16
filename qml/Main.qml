import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "pages"

ApplicationWindow {
    id: window

    width: 1024
    height: 768
    minimumWidth: 1024
    minimumHeight: 768
    maximumWidth: 1024
    maximumHeight: 768
    visible: true
    title: "Щит аварийного освещения Dialog G2"
    color: "#ffffff"

    property date now: new Date()
    property bool unlocked: false
    property int selectedLineIndex: 0
    property int selectedScheduleIndex: -1
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

    onUnlockedChanged: {
        if (unlocked)
            accessIdleTimer.restart()
        else
            accessIdleTimer.stop()
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

    Component {
        id: startPageComponent

        StartPage {
            unlocked: window.unlocked
            onLoginRequested: pageStack.replace(passwordPageComponent)
            onLogoutRequested: window.unlocked = false
            onTestRequested: pageStack.replace(testPageComponent)
            onSettingsRequested: pageStack.replace(settingsPageComponent)
            onScheduleRequested: pageStack.replace(schedulePageComponent)
            onJournalRequested: pageStack.replace(journalPageComponent)
            onSystemRequested: pageStack.replace(systemPageComponent)
            onLinesRequested: pageStack.replace(linesPageComponent)
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
