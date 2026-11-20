import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card
    property QtObject theme
    readonly property bool compact: width < 720
    property date visibleMonth: new Date(new Date().getFullYear(), new Date().getMonth(), 1)
    property string selectedDate: Qt.formatDate(new Date(), "yyyy-MM-dd")

    implicitHeight: compact ? 500 : 330
    radius: theme.radius
    color: theme.card
    border.color: theme.border

    function durationText(seconds) {
        const totalMinutes = Math.floor(seconds / 60)
        const hours = Math.floor(totalMinutes / 60)
        const minutes = totalMinutes % 60
        if (hours > 0 && minutes > 0) return qsTr("%1 h %2 min").arg(hours).arg(minutes)
        if (hours > 0) return qsTr("%1 h").arg(hours)
        return qsTr("%1 min").arg(totalMinutes)
    }

    function feedback(seconds) {
        if (seconds <= 0) return qsTr("No focus time yet today. Start with one small session.")
        if (seconds < 25 * 60) return qsTr("You have started. A few focused minutes still count.")
        if (seconds < 60 * 60) return qsTr("One solid session finished. Your attention is taking root.")
        if (seconds < 120 * 60) return qsTr("More than an hour of focus today. Your rhythm is becoming steady.")
        return qsTr("Deep focus today. Remember to rest your eyes and body, too.")
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: card.compact ? 16 : 20
        columns: card.compact ? 1 : 2
        columnSpacing: 28
        rowSpacing: 14

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 8
            Text {
                text: qsTr("Today's focus")
                color: theme.muted; font.pixelSize: 12; font.weight: Font.DemiBold
            }
            Text {
                text: card.durationText(focusManager.todaySeconds)
                color: theme.primary; font.pixelSize: card.compact ? 28 : 34; font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                text: card.feedback(focusManager.todaySeconds)
                color: theme.text; font.pixelSize: 13; wrapMode: Text.WordWrap
            }
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: theme.border }
            Text {
                text: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US")
                    .toString(new Date(card.selectedDate + "T00:00:00"), "MMMM d, yyyy")
                color: theme.muted; font.pixelSize: 12
            }
            Text {
                property int revision: focusManager.historyRevision
                text: {
                    const ignored = revision
                    return card.durationText(focusManager.focusSecondsForDate(card.selectedDate))
                }
                color: theme.text; font.pixelSize: 20; font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Tap a date to review that day's focused study time.")
                color: theme.muted; font.pixelSize: 11; wrapMode: Text.WordWrap
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
            spacing: 6
            RowLayout {
                Layout.fillWidth: true
                ToolButton {
                    id: previousMonth
                    text: "‹"; Accessible.name: qsTr("Previous month")
                    contentItem: Text { text: previousMonth.text; color: theme.text; font.pixelSize: 24; horizontalAlignment: Text.AlignHCenter }
                    onClicked: card.visibleMonth = new Date(card.visibleMonth.getFullYear(), card.visibleMonth.getMonth() - 1, 1)
                }
                Text {
                    Layout.fillWidth: true
                    text: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US")
                        .toString(card.visibleMonth, "MMMM yyyy")
                    color: theme.text; font.pixelSize: 15; font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                }
                ToolButton {
                    id: nextMonth
                    text: "›"; Accessible.name: qsTr("Next month")
                    contentItem: Text { text: nextMonth.text; color: theme.text; font.pixelSize: 24; horizontalAlignment: Text.AlignHCenter }
                    onClicked: card.visibleMonth = new Date(card.visibleMonth.getFullYear(), card.visibleMonth.getMonth() + 1, 1)
                }
            }
            DayOfWeekRow {
                Layout.fillWidth: true
                locale: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US")
                delegate: Text {
                    required property var model
                    text: model.shortName; color: theme.muted; font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
            }
            MonthGrid {
                id: monthGrid
                Layout.fillWidth: true
                Layout.preferredHeight: 232
                month: card.visibleMonth.getMonth()
                year: card.visibleMonth.getFullYear()
                locale: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US")
                delegate: Rectangle {
                    id: dayCell
                    required property var model
                    readonly property string isoDate: Qt.formatDate(model.date, "yyyy-MM-dd")
                    property int revision: focusManager.historyRevision
                    readonly property int focusedSeconds: {
                        const ignored = revision
                        return focusManager.focusSecondsForDate(isoDate)
                    }
                    readonly property bool selected: isoDate === card.selectedDate
                    implicitWidth: 38; implicitHeight: 38; radius: 9
                    opacity: model.month === monthGrid.month ? 1 : 0.32
                    color: selected ? theme.primary
                        : focusedSeconds >= 60 * 60 ? Qt.alpha(theme.primary, 0.62)
                        : focusedSeconds >= 25 * 60 ? Qt.alpha(theme.primary, 0.36)
                        : focusedSeconds > 0 ? theme.primarySoft
                        : model.today ? Qt.alpha(theme.primarySoft, 0.7) : "transparent"
                    border.width: activeFocus ? 2 : 0
                    border.color: theme.primary
                    Accessible.role: Accessible.Button
                    Accessible.focusable: true
                    Accessible.name: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US")
                        .toString(model.date, "dddd, MMMM d") + ", " + card.durationText(focusedSeconds)
                    activeFocusOnTab: true

                    function chooseDate() {
                        card.selectedDate = isoDate
                        card.visibleMonth = new Date(model.date.getFullYear(), model.date.getMonth(), 1)
                    }

                    Column {
                        anchors.centerIn: parent; spacing: -1
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: dayCell.model.day
                            color: dayCell.selected || dayCell.focusedSeconds >= 60 * 60 ? "white" : theme.text
                            font.pixelSize: 12; font.weight: dayCell.model.today ? Font.Bold : Font.Normal
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: dayCell.focusedSeconds > 0
                            text: Math.max(1, Math.floor(dayCell.focusedSeconds / 60))
                            color: dayCell.selected || dayCell.focusedSeconds >= 60 * 60 ? "white" : theme.primary
                            font.pixelSize: 8
                        }
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: { dayCell.forceActiveFocus(); dayCell.chooseDate() }
                    }
                    Keys.onReturnPressed: event => { chooseDate(); event.accepted = true }
                    Keys.onSpacePressed: event => { chooseDate(); event.accepted = true }
                }
            }
        }
    }
}
