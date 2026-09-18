import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card
    property QtObject theme
    signal gardenRequested()
    implicitWidth: 270; implicitHeight: 122
    radius: theme.radius; color: theme.card; border.color: theme.border
    activeFocusOnTab: true
    Accessible.role: Accessible.Button
    Accessible.name: qsTr("Open your garden")
    border.width: activeFocus ? 2 : 1

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 18; spacing: 8
        RowLayout {
            Layout.fillWidth: true
            Text { text: qsTr("Level %1").arg(growthManager.level); color: theme.text; font.pixelSize: 16; font.weight: Font.DemiBold }
            Item { Layout.fillWidth: true }
            Text { text: qsTr("+%1 XP today").arg(growthManager.todayXp); color: theme.primary; font.pixelSize: 12; font.weight: Font.DemiBold }
        }
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 8; radius: 4; color: theme.primarySoft
            Rectangle {
                width: parent.width * growthManager.levelProgress; height: parent.height; radius: 4; color: theme.primary
                Behavior on width { NumberAnimation { duration: theme.animationDuration + 120; easing.type: Easing.OutCubic } }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Text { text: qsTr("%1 / %2 XP").arg(growthManager.levelXp).arg(growthManager.xpForNextLevel); color: theme.muted; font.pixelSize: 12 }
            Item { Layout.fillWidth: true }
            Text { text: qsTr("View garden  ›"); color: theme.primary; font.pixelSize: 12; font.weight: Font.DemiBold }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 6
            Text { text: qsTr("Vitality"); color: theme.muted; font.pixelSize: 11 }
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 5; radius: 3; color: theme.input
                Rectangle {
                    width: parent.width * growthManager.vitality / 100; height: parent.height; radius: 3
                    color: growthManager.vitalityState === 0 ? theme.primary : growthManager.vitalityState === 1 ? theme.warning : "#A47A61"
                }
            }
            Text { text: growthManager.vitality; color: theme.muted; font.pixelSize: 11 }
        }
    }
    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { card.forceActiveFocus(); card.gardenRequested() } }
    Keys.onReturnPressed: event => { card.gardenRequested(); event.accepted = true }
    Keys.onSpacePressed: event => { card.gardenRequested(); event.accepted = true }
}
