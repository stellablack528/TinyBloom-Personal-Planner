import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: popup
    property QtObject theme
    property int amount: 0
    property string reason: ""
    parent: Overlay.overlay
    x: parent.width - width - 28
    y: 28
    width: Math.min(360, parent.width - 56)
    height: 82; padding: 0
    closePolicy: Popup.NoAutoClose
    scale: opened ? 1 : 0.92
    opacity: opened ? 1 : 0
    Behavior on scale { NumberAnimation { duration: theme.animationDuration + 80; easing.type: Easing.OutBack } }
    Behavior on opacity { NumberAnimation { duration: theme.animationDuration } }
    background: Rectangle {
        radius: 16; color: theme.card; border.width: 2; border.color: theme.primary
    }
    contentItem: RowLayout {
        anchors.fill: parent; anchors.margins: 16; spacing: 12
        Rectangle {
            width: 46; height: 46; radius: 15; color: theme.primarySoft
            Text { anchors.centerIn: parent; text: "+" + popup.amount; color: theme.primary; font.pixelSize: 16; font.weight: Font.Bold }
        }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 3
            Text { text: qsTr("Your garden grew"); color: theme.text; font.pixelSize: 15; font.weight: Font.DemiBold }
            Text { Layout.fillWidth: true; text: popup.reason; color: theme.muted; font.pixelSize: 12; elide: Text.ElideRight }
        }
        Text { text: "✦"; color: theme.warning; font.pixelSize: 21 }
    }
    Timer { id: closeTimer; interval: 3200; onTriggered: popup.close() }
    function showReward(xp, rewardReason) {
        amount = xp; reason = rewardReason; open(); closeTimer.restart()
    }
}
