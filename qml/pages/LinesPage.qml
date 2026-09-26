import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648
    color: "white"

    readonly property int tableFontSize: 21
    readonly property int typeFontSize: 17
    readonly property int wName: 206
    readonly property int wType: 140
    readonly property int wState: 124
    readonly property int wNominalPower: 98
    readonly property int wMeasuredPower: 104
    readonly property int wVoltage: 78
    readonly property int wCurrent: 72
    readonly property int wLeakage: 108
    readonly property var panelLines: panel.lines

    ListModel { id: lineRows }
    Component.onCompleted: updateLines()
    onPanelLinesChanged: updateLines()

    function updateLines() {
        var rows = panelLines.filter(function(line) { return line.mode !== 2 })
        for (var i = 0; i < rows.length; ++i) {
            var row = {}
            for (var key in rows[i]) {
                if (key !== "index") {
                    // ListModel cannot create a role from a null value. Keep
                    // measurement roles present; availability flags decide
                    // whether the value is displayed as a dash.
                    var value = rows[i][key]
                    row[key] = (value === null || value === undefined)
                            && (key === "power" || key === "voltage"
                                || key === "current" || key === "leakage")
                        ? 0
                        : value
                }
            }
            row.lineNumber = rows[i].index
            if (i < lineRows.count)
                lineRows.set(i, row)
            else
                lineRows.append(row)
        }
        if (lineRows.count > rows.length)
            lineRows.remove(rows.length, lineRows.count - rows.length)
    }

    function numberText(value, available, digits) {
        if (available !== undefined && !available)
            return "-"
        if (value === undefined || value === null || !isFinite(value))
            return "-"
        return Number(value).toFixed(digits)
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
                text: "Линии"
                color: "#111111"
                font.pixelSize: 40
                font.family: "Arial"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            Item {
                Layout.preferredWidth: 82
                Layout.preferredHeight: 82
            }
        }

        Row {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            spacing: 0

            HeaderCell { width: root.wName; text: "Название" }
            HeaderCell { width: root.wType; text: "Тип" }
            HeaderCell { width: root.wState; text: "Состояние" }
            PowerHeaderCell { width: root.wNominalPower; subText: "ном" }
            PowerHeaderCell { width: root.wMeasuredPower; subText: "изм" }
            HeaderCell { width: root.wVoltage; text: "U, В" }
            HeaderCell { width: root.wCurrent; text: "I, А" }
            HeaderCell { width: root.wLeakage; text: "Iутеч, мА" }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ffffff"

            Text {
                anchors.centerIn: parent
                visible: lineRows.count === 0
                text: "Включенные линии не настроены"
                color: "#666666"
                font.pixelSize: 34
                font.family: "Arial"
            }

            ListView {
                id: lineList

                anchors.fill: parent
                clip: true
                spacing: 8
                model: lineRows
                visible: lineRows.count > 0

                delegate: Rectangle {
                    width: lineList.width - 34
                    height: 62
                    radius: 6
                    color: "#f7f7f7"
                    border.color: "#d0d0d0"
                    border.width: 2

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 0

                        NameCell {
                            width: root.wName - 8
                            text: model.description || ("Линия " + (model.lineNumber || (index + 1)))
                            ok: model.displayStateOk
                        }

                        TextCell {
                            width: root.wType
                            text: model.displayModeText
                            font.pixelSize: root.typeFontSize
                        }

                        StateCell {
                            width: root.wState
                            text: model.displayStateText
                            ok: model.displayStateOk
                        }

                        TextCell {
                            width: root.wNominalPower
                            text: root.numberText(model.mpower, true, 0)
                        }

                        TextCell {
                            width: root.wMeasuredPower
                            text: root.numberText(model.power, model.powerAvailable, 0)
                        }

                        TextCell {
                            width: root.wVoltage
                            text: root.numberText(model.voltage, model.voltageAvailable, 0)
                        }

                        TextCell {
                            width: root.wCurrent
                            text: root.numberText(model.current, model.currentAvailable, 1)
                        }

                        TextCell {
                            width: root.wLeakage
                            text: root.numberText(model.leakage, model.leakageAvailable, 1)
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

    component HeaderCell: Rectangle {
        property string text: ""

        height: 44
        color: "#e7e7e7"
        border.color: "#cfcfcf"
        border.width: 1

        Text {
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            text: parent.text
            color: "#333333"
            font.pixelSize: root.tableFontSize
            font.bold: false
            font.family: "Arial"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component PowerHeaderCell: Rectangle {
        property string subText: ""

        height: 44
        color: "#e7e7e7"
        border.color: "#cfcfcf"
        border.width: 1

        Text {
            anchors.fill: parent
            text: "P<span style='font-size:14px'>" + parent.subText + "</span>, Вт"
            textFormat: Text.RichText
            color: "#333333"
            font.pixelSize: root.tableFontSize
            font.family: "Arial"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    component NameCell: Item {
        property string text: ""
        property bool ok: true

        height: 62

        RowLayout {
            anchors.fill: parent
            spacing: 6

            Rectangle {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                radius: 15
                color: ok ? "#11bf5d" : "#d84236"
            }

            Text {
                Layout.fillWidth: true
                text: parent.parent.text
                color: "#111111"
                font.pixelSize: root.tableFontSize
                font.bold: false
                font.family: "Arial"
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    }

    component TextCell: Text {
        property color textColor: "#111111"

        height: 62
        color: textColor
        font.pixelSize: root.tableFontSize
        font.bold: false
        font.family: "Arial"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    component StateCell: Rectangle {
        property string text: ""
        property bool ok: true

        height: 62
        color: "transparent"

        Rectangle {
            anchors.centerIn: parent
            width: parent.width - 18
            height: 36
            radius: 4
            color: parent.text === "ВКЛ"
                   ? "#f3d64e"
                   : (parent.text === "ВЫКЛ" || parent.text === "ОТКЛ" ? "#d5d5d5" : "#d84236")

            Text {
                anchors.fill: parent
                text: parent.parent.text
                color: parent.parent.text === "ВКЛ" || parent.parent.text === "ВЫКЛ" || parent.parent.text === "ОТКЛ"
                       ? "#111111"
                       : "#ffffff"
                font.pixelSize: root.tableFontSize
                font.bold: false
                font.family: "Arial"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
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
