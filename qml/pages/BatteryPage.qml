import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648
    color: "white"

    readonly property var battery: panel.battery
    property int displayedCellCount: 0
    property int displayedTemperatureCount: 0
    property int displayedFaultCount: 1
    property int displayedWarningCount: 1
    property bool ready: false

    ListModel { id: batteryRows; objectName: "batteryRows" }
    Component.onCompleted: { ready = true; updateRows() }
    onBatteryChanged: if (ready) updateRows()

    function updateRows() {
        var b = root.battery || {}
        displayedCellCount = Math.max(displayedCellCount, b.cellCount || 0, (b.cellVoltages || []).length)
        displayedTemperatureCount = Math.max(displayedTemperatureCount, (b.temperatures || []).length)
        displayedFaultCount = Math.max(displayedFaultCount, (b.faults || []).length)
        displayedWarningCount = Math.max(displayedWarningCount, (b.warnings || []).length)
        var rows = buildRows()
        for (var i = 0; i < rows.length; ++i) {
            if (i < batteryRows.count)
                batteryRows.set(i, rows[i])
            else
                batteryRows.append(rows[i])
        }
        if (batteryRows.count > rows.length)
            batteryRows.remove(rows.length, batteryRows.count - rows.length)
    }

    function numberText(value, digits, suffix) {
        if (value === undefined || value === null || isNaN(value))
            return "-"
        return Number(value).toFixed(digits) + suffix
    }

    function intText(value, suffix) {
        if (value === undefined || value === null || value < 0 || isNaN(value))
            return "-"
        return Number(value).toFixed(0) + suffix
    }

    function boolText(value) {
        if (value === undefined || value === null)
            return "—"
        return value ? "ДА" : "НЕТ"
    }

    function hexText(value) {
        if (value === undefined || value === null || isNaN(value))
            return "-"
        var hex = Number(value).toString(16).toUpperCase()
        while (hex.length < 4)
            hex = "0" + hex
        return "0x" + hex
    }

    function addRow(list, name, value, bad) {
        list.push({ "name": name, "value": value, "bad": bad === true, "section": false })
    }

    function addSection(list, title) {
        list.push({ "section": true, "name": title, "value": "", "bad": false })
    }

    function buildRows() {
        var b = root.battery || {}
        var result = []

        addSection(result, "BMS")
        addRow(result, "Связь с батареей", boolText(b.connected), b.connected === false)
        addRow(result, "Обмен с BMS", boolText(b.communicationOk), b.communicationOk === false)
        if (!b.communicationOk) {
            b = {} // Keep the page structure, but never show stale measurements.
        }
        addRow(result, "Состояние", b.stateText || "Нет данных", !panel.batteryOk)
        addRow(result, "Код состояния", b.stateCode || "-")
        addRow(result, "Заряд SOC", intText(b.socPercent, "%"))
        addRow(result, "Напряжение батареи", numberText(b.voltage, 2, " В"))
        addRow(result, "Ток батареи", numberText(b.current, 2, " А"))
        addRow(result, "Остаточная емкость", numberText(b.remainingCapacityAh, 2, " Ач"))
        addRow(result, "Номинальная емкость", numberText(b.nominalCapacityAh, 2, " Ач"))
        addRow(result, "Полная емкость", numberText(b.fullChargeCapacityAh, 2, " Ач"))
        addRow(result, "Количество циклов", intText(b.cycleCount, ""))
        addRow(result, "Количество ячеек", intText(b.cellCount, ""), !b.cellCount)

        addSection(result, "Разрешения и режимы")
        addRow(result, "Заряд разрешен", boolText(b.chargeAllowed), b.chargeAllowed === false)
        addRow(result, "Разряд разрешен", boolText(b.dischargeAllowed), b.dischargeAllowed === false)
        addRow(result, "Балансировка активна", boolText(b.balancingActive))
        addRow(result, "Подогрев активен", boolText(b.heatingActive))
        addRow(result, "Protection raw", hexText(b.protectionStatusRaw), b.protectionStatusRaw > 0)
        addRow(result, "Alarm raw", hexText(b.alarmStatusRaw), b.alarmStatusRaw > 0)

        addSection(result, "Ячейки")
        addRow(result, "Минимальное напряжение ячейки", numberText(b.minCellVoltage, 3, " В"))
        addRow(result, "Максимальное напряжение ячейки", numberText(b.maxCellVoltage, 3, " В"))
        addRow(result, "Разброс ячеек", numberText(b.cellVoltageDelta, 3, " В"))

        var cells = b.cellVoltages || []
        if (displayedCellCount === 0) {
            addRow(result, "Напряжения ячеек", "-")
        } else {
            for (var i = 0; i < displayedCellCount; ++i)
                addRow(result, "Ячейка " + (i + 1), numberText(cells[i], 3, " В"))
        }

        addSection(result, "Температуры")
        addRow(result, "Минимальная температура", numberText(b.minTemperature, 1, " °C"))
        addRow(result, "Максимальная температура", numberText(b.maxTemperature, 1, " °C"))

        var temperatures = b.temperatures || []
        if (displayedTemperatureCount === 0) {
            addRow(result, "Датчики температуры", "-")
        } else {
            for (var t = 0; t < displayedTemperatureCount; ++t)
                addRow(result, "Температура " + (t + 1), numberText(temperatures[t], 1, " °C"))
        }

        addSection(result, "Ошибки")
        var faults = b.faults || []
        if (faults.length === 0) {
            addRow(result, "Ошибки BMS", b.communicationOk ? "нет" : "Нет данных")
        } else {
            for (var f = 0; f < faults.length; ++f)
                addRow(result, "Ошибка " + (f + 1), faults[f], true)
        }

        for (var emptyFault = Math.max(1, faults.length); emptyFault < displayedFaultCount; ++emptyFault)
            addRow(result, "Ошибка " + (emptyFault + 1), "—")

        addSection(result, "Предупреждения")
        var warnings = b.warnings || []
        if (warnings.length === 0) {
            addRow(result, "Предупреждения BMS", b.communicationOk ? "нет" : "Нет данных")
        } else {
            for (var w = 0; w < warnings.length; ++w)
                addRow(result, "Предупреждение " + (w + 1), warnings[w], true)
        }

        for (var emptyWarning = Math.max(1, warnings.length); emptyWarning < displayedWarningCount; ++emptyWarning)
            addRow(result, "Предупреждение " + (emptyWarning + 1), "—")

        return result
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

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
                text: "Батарея"
                color: "#111111"
                font.pixelSize: 40
                font.family: "Arial"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            Rectangle {
                Layout.preferredWidth: 150
                Layout.preferredHeight: 68
                radius: 6
                color: panel.batteryOk ? "#11bf5d" : "#d84236"

                Text {
                    anchors.centerIn: parent
                    text: panel.batteryOk ? "НОРМ" : "АВАР"
                    color: "#ffffff"
                    font.pixelSize: 28
                    font.family: "Arial"
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ffffff"
            border.color: "#d0d0d0"
            border.width: 2
            radius: 6

            ListView {
                id: listView
                objectName: "batteryList"

                anchors.fill: parent
                anchors.margins: 8
                clip: true
                spacing: 2
                model: batteryRows

                delegate: Rectangle {
                    width: listView.width - 34
                    height: model.section ? 42 : 38
                    color: model.section ? "#e7e7e7" : (index % 2 === 0 ? "#ffffff" : "#f7f7f7")

                    Text {
                        visible: model.section === true
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        text: model.name
                        color: "#111111"
                        font.pixelSize: 24
                        font.family: "Arial"
                        verticalAlignment: Text.AlignVCenter
                    }

                    RowLayout {
                        visible: model.section !== true
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 16

                        Text {
                            Layout.preferredWidth: 390
                            text: model.name
                            color: "#333333"
                            font.pixelSize: 22
                            font.family: "Arial"
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: model.value
                            color: model.bad ? "#d84236" : "#111111"
                            font.pixelSize: 22
                            font.family: "Arial"
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    width: 28
                }
            }
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
