import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    signal backRequested()

    property var journalModel: []
    property int currentRow: -1

    width: parent ? parent.width : 1024
    height: parent ? parent.height : 648
    color: "white"

    function refresh() {
        var entries = panel.journal()
        var copy = []
        for (var i = 0; i < entries.length; ++i)
            copy.push(entries[i])

        copy.sort(function(a, b) {
            var ad = Date.parse(a.finishedAt || a.startedAt || "")
            var bd = Date.parse(b.finishedAt || b.startedAt || "")
            if (isNaN(ad))
                ad = 0
            if (isNaN(bd))
                bd = 0
            return bd - ad
        })

        journalModel = copy
        if (journalModel.length === 0)
            currentRow = -1
        else if (currentRow < 0 || currentRow >= journalModel.length)
            currentRow = 0
    }

    function selectedEntry() {
        if (currentRow < 0 || currentRow >= journalModel.length)
            return null
        return journalModel[currentRow]
    }

    function formatDateTime(value) {
        if (!value)
            return "-"

        var date = new Date(value)
        if (isNaN(date.getTime()))
            return "-"

        return Qt.formatDateTime(date, "dd.MM.yyyy hh:mm")
    }

    function durationText(entry) {
        if (!entry || !entry.startedAt || !entry.finishedAt)
            return "-"

        var start = new Date(entry.startedAt)
        var finish = new Date(entry.finishedAt)
        if (isNaN(start.getTime()) || isNaN(finish.getTime()))
            return "-"

        var sec = Math.max(0, Math.round((finish.getTime() - start.getTime()) / 1000))
        var min = Math.floor(sec / 60)
        var rest = sec % 60
        return min + ":" + (rest < 10 ? "0" + rest : rest)
    }

    function numberText(value, suffix) {
        var number = Number(value)
        if (isNaN(number))
            return "-"
        return number.toFixed(0) + suffix
    }

    Component.onCompleted: refresh()
    onVisibleChanged: if (visible) refresh()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

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
                text: "Журнал тестов"
                color: "#111111"
                font.pixelSize: 36
                font.family: "Arial"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
            }

            Item {
                Layout.preferredWidth: 82
                Layout.preferredHeight: 72
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            radius: 6
            color: "#f2f2f2"
            border.color: "#d0d0d0"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 8

                HeaderCell { text: "Завершен"; Layout.preferredWidth: 170 }
                HeaderCell { text: "Тест"; Layout.preferredWidth: 220 }
                HeaderCell { text: "Источник"; Layout.preferredWidth: 130 }
                HeaderCell { text: "Итог"; Layout.preferredWidth: 190 }
                HeaderCell { text: "Линий"; Layout.fillWidth: true }
            }
        }

        ListView {
            id: journalList

            Layout.fillWidth: true
            Layout.preferredHeight: 260
            clip: true
            spacing: 4
            model: root.journalModel
            currentIndex: root.currentRow

            delegate: Rectangle {
                width: journalList.width
                height: 58
                radius: 6
                color: index === root.currentRow ? "#eaf4ff" : "#ffffff"
                border.color: index === root.currentRow ? "#80b0ff" : "#cccccc"

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.currentRow = index
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    ValueCell { text: root.formatDateTime(modelData.finishedAt); Layout.preferredWidth: 170 }
                    ValueCell { text: modelData.kindText || "-"; Layout.preferredWidth: 220 }
                    ValueCell { text: modelData.sourceText || "-"; Layout.preferredWidth: 130 }
                    ValueCell {
                        text: modelData.statusText || "-"
                        color: modelData.statusCode === "passed" ? "#0d8a43" : "#c43b32"
                        Layout.preferredWidth: 190
                    }
                    ValueCell {
                        text: modelData.lines ? modelData.lines.length.toString() : "0"
                        Layout.fillWidth: true
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
                width: 28
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 6
            color: "#ffffff"
            border.color: "#d0d0d0"

            Text {
                anchors.centerIn: parent
                visible: root.selectedEntry() === null
                text: "Записей тестов пока нет"
                color: "#666666"
                font.pixelSize: 30
                font.family: "Arial"
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8
                visible: root.selectedEntry() !== null

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 34
                    spacing: 12

                    DetailText {
                        text: {
                            var entry = root.selectedEntry()
                            return entry ? ("Начало: " + root.formatDateTime(entry.startedAt)) : ""
                        }
                        Layout.preferredWidth: 245
                    }
                    DetailText {
                        text: {
                            var entry = root.selectedEntry()
                            return entry ? ("Конец: " + root.formatDateTime(entry.finishedAt)) : ""
                        }
                        Layout.preferredWidth: 245
                    }
                    DetailText {
                        text: {
                            var entry = root.selectedEntry()
                            return entry ? ("Длит.: " + root.durationText(entry)) : ""
                        }
                        Layout.preferredWidth: 150
                    }
                    DetailText {
                        text: {
                            var entry = root.selectedEntry()
                            return entry && entry.reason ? entry.reason : ""
                        }
                        Layout.fillWidth: true
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    color: "#f2f2f2"
                    radius: 5

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8

                        HeaderCell { text: "Линия"; Layout.fillWidth: true }
                        HeaderCell { text: "Измерено"; Layout.preferredWidth: 120 }
                        HeaderCell { text: "Номинал"; Layout.preferredWidth: 120 }
                        HeaderCell { text: "Допуск"; Layout.preferredWidth: 105 }
                        HeaderCell { text: "Итог"; Layout.preferredWidth: 170 }
                    }
                }

                ListView {
                    id: lineList

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 3
                    model: {
                        var entry = root.selectedEntry()
                        return entry && entry.lines ? entry.lines : []
                    }

                    delegate: Rectangle {
                        width: lineList.width
                        height: 46
                        color: "#ffffff"
                        border.color: "#e0e0e0"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8

                            ValueCell {
                                text: modelData.lineName || ("Линия " + modelData.lineIndex)
                                Layout.fillWidth: true
                            }
                            ValueCell {
                                text: root.numberText(modelData.measuredPower, " Вт")
                                Layout.preferredWidth: 120
                            }
                            ValueCell {
                                text: root.numberText(modelData.nominalPower, " Вт")
                                Layout.preferredWidth: 120
                            }
                            ValueCell {
                                text: root.numberText(modelData.tolerancePercent, "%")
                                Layout.preferredWidth: 105
                            }
                            ValueCell {
                                text: modelData.statusText || "-"
                                color: modelData.statusCode === "passed" ? "#0d8a43" : "#c43b32"
                                Layout.preferredWidth: 170
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        width: 28
                    }
                }
            }
        }
    }

    component HeaderCell: Text {
        color: "#111111"
        font.pixelSize: 22
        font.family: "Arial"
        font.bold: true
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    component ValueCell: Text {
        color: "#111111"
        font.pixelSize: 22
        font.family: "Arial"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    component DetailText: Text {
        color: "#111111"
        font.pixelSize: 21
        font.family: "Arial"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
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
