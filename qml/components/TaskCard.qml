import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card
    required property var taskId
    required property string title
    required property bool completed
    required property int priority
    required property string priorityName
    required property var dueDate
    required property int estimatedMinutes
    required property string category
    required property int subtaskCount
    required property int completedSubtaskCount
    property QtObject theme
    signal toggleRequested(var id)
    signal editRequested(var id)

    width: ListView.view ? ListView.view.width : 500
    height: details.visible ? 96 : 78
    radius: theme.radius
    color: mouse.containsMouse ? theme.cardHover : theme.card
    border.width: activeFocus ? 2 : 1
    border.color: activeFocus ? theme.primary : theme.border
    activeFocusOnTab: true
    Accessible.role: Accessible.Button
    Accessible.name: title
    Accessible.description: completed ? qsTr("Completed task. Open details.") : qsTr("Active task. Open details.")
    Accessible.focusable: true
    Behavior on color { ColorAnimation { duration: theme.animationDuration } }

    MouseArea {
        id: mouse; anchors.fill: parent; hoverEnabled: true
        onClicked: { card.forceActiveFocus(); card.editRequested(card.taskId) }
    }
    Keys.onReturnPressed: event => { card.editRequested(card.taskId); event.accepted = true }
    Keys.onSpacePressed: event => { card.editRequested(card.taskId); event.accepted = true }
    RowLayout {
        anchors.fill: parent; anchors.margins: 16; spacing: 14
        Rectangle {
            id: completionControl
            Layout.preferredWidth: 24; Layout.preferredHeight: 24; radius: 8
            color: completed ? theme.primary : "transparent"
            border.width: activeFocus || !completed ? 2 : 0; border.color: theme.primary
            activeFocusOnTab: true
            Accessible.role: Accessible.CheckBox
            Accessible.name: completed ? qsTr("Mark %1 active").arg(card.title) : qsTr("Mark %1 complete").arg(card.title)
            Accessible.checked: completed
            Accessible.focusable: true
            Text { anchors.centerIn: parent; text: "✓"; visible: completed; color: "white"; font.pixelSize: 15; font.weight: Font.Bold }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { completionControl.forceActiveFocus(); card.toggleRequested(card.taskId) } }
            Keys.onReturnPressed: event => { card.toggleRequested(card.taskId); event.accepted = true }
            Keys.onSpacePressed: event => { card.toggleRequested(card.taskId); event.accepted = true }
            Behavior on color { ColorAnimation { duration: theme.animationDuration } }
        }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 7
            Text {
                Layout.fillWidth: true; text: card.title; color: completed ? theme.muted : theme.text
                font.pixelSize: 15; font.weight: Font.DemiBold; elide: Text.ElideRight
                font.strikeout: completed
            }
            RowLayout {
                id: details; visible: category.length > 0 || estimatedMinutes > 0 || dueDate.toString().length > 0 || subtaskCount > 0
                spacing: 12
                Text { visible: category.length > 0; text: category; color: theme.primary; font.pixelSize: 12; font.weight: Font.DemiBold }
                Text { visible: estimatedMinutes > 0; text: qsTr("◷ %1 min").arg(estimatedMinutes); color: theme.muted; font.pixelSize: 12 }
                Text { visible: dueDate.toString().length > 0; text: "◫ " + Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US").toString(dueDate, "MMM d"); color: theme.muted; font.pixelSize: 12 }
                Text { visible: subtaskCount > 0; text: qsTr("%1 / %2 steps").arg(completedSubtaskCount).arg(subtaskCount); color: theme.muted; font.pixelSize: 12 }
                Item { Layout.fillWidth: true }
            }
        }
        Rectangle {
            visible: priority !== 0
            Layout.preferredWidth: priorityLabel.implicitWidth + 16; Layout.preferredHeight: 26; radius: 8
            color: priority === 2 ? Qt.alpha(theme.warning, 0.12) : theme.primarySoft
            Text {
                id: priorityLabel; anchors.centerIn: parent; text: priorityName
                color: priority === 2 ? theme.warning : theme.primary; font.pixelSize: 11; font.weight: Font.DemiBold
            }
        }
        Text { text: "›"; color: theme.muted; font.pixelSize: 22 }
    }
}
