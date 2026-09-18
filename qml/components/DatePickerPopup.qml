import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: popup
    property QtObject theme
    property date selectedDate: new Date()
    property date visibleMonth: new Date(selectedDate.getFullYear(), selectedDate.getMonth(), 1)
    signal dateSelected(date value)

    function openFor(value) {
        let parsed = new Date(value + "T00:00:00")
        if (!value || isNaN(parsed.getTime())) parsed = new Date()
        selectedDate = parsed
        visibleMonth = new Date(parsed.getFullYear(), parsed.getMonth(), 1)
        open()
    }

    parent: Overlay.overlay
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: 340
    height: calendarColumn.implicitHeight + 28
    padding: 14
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    background: Rectangle {
        radius: 16; color: theme.card; border.color: theme.border
    }

    contentItem: ColumnLayout {
        id: calendarColumn
        Accessible.name: qsTr("Choose a due date")
        spacing: 10
        RowLayout {
            Layout.fillWidth: true
            ToolButton {
                id: previousMonthButton
                text: "‹"; Accessible.name: qsTr("Previous month")
                contentItem: Text { text: previousMonthButton.text; color: popup.theme.text; font.pixelSize: 24; horizontalAlignment: Text.AlignHCenter }
                onClicked: popup.visibleMonth = new Date(popup.visibleMonth.getFullYear(), popup.visibleMonth.getMonth() - 1, 1)
            }
            Text {
                Layout.fillWidth: true
                text: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US").toString(popup.visibleMonth, "MMMM yyyy")
                color: theme.text; font.pixelSize: 16; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter
            }
            ToolButton {
                id: nextMonthButton
                text: "›"; Accessible.name: qsTr("Next month")
                contentItem: Text { text: nextMonthButton.text; color: popup.theme.text; font.pixelSize: 24; horizontalAlignment: Text.AlignHCenter }
                onClicked: popup.visibleMonth = new Date(popup.visibleMonth.getFullYear(), popup.visibleMonth.getMonth() + 1, 1)
            }
        }
        DayOfWeekRow {
            Layout.fillWidth: true
            locale: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US")
            delegate: Text {
                required property var model
                text: model.shortName
                color: theme.muted; font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
        }
        MonthGrid {
            id: monthGrid
            Layout.fillWidth: true
            Layout.preferredHeight: 238
            month: popup.visibleMonth.getMonth()
            year: popup.visibleMonth.getFullYear()
            locale: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US")
            delegate: Rectangle {
                id: dayCell
                required property var model
                implicitWidth: 40; implicitHeight: 34; radius: 9
                readonly property bool selected: Qt.formatDate(model.date, "yyyy-MM-dd") === Qt.formatDate(popup.selectedDate, "yyyy-MM-dd")
                color: selected ? theme.primary : model.today ? theme.primarySoft : dayMouse.containsMouse ? theme.cardHover : "transparent"
                opacity: model.month === monthGrid.month ? 1 : 0.38
                border.width: activeFocus ? 2 : 0
                border.color: theme.primary
                Accessible.role: Accessible.Button
                Accessible.name: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US").toString(model.date, "dddd, MMMM d")
                Accessible.focusable: true
                activeFocusOnTab: true
                function chooseDate() {
                    popup.selectedDate = model.date
                    popup.dateSelected(model.date)
                    popup.close()
                }
                Text {
                    anchors.centerIn: parent; text: model.day
                    color: parent.selected ? "white" : theme.text; font.pixelSize: 13
                }
                MouseArea {
                    id: dayMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: { dayCell.forceActiveFocus(); dayCell.chooseDate() }
                }
                Keys.onReturnPressed: event => { chooseDate(); event.accepted = true }
                Keys.onSpacePressed: event => { chooseDate(); event.accepted = true }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton {
                theme: popup.theme; primary: false; text: qsTr("Today")
                onClicked: {
                    popup.selectedDate = new Date()
                    popup.dateSelected(popup.selectedDate)
                    popup.close()
                }
            }
            AppButton { theme: popup.theme; primary: false; text: qsTr("Cancel"); onClicked: popup.close() }
        }
    }
}
