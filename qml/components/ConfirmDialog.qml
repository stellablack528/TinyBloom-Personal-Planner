import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme
    property var targetId: -1
    signal confirmed(var id)
    modal: true; anchors.centerIn: parent; width: 390
    padding: 22; closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 16; color: theme.card; border.color: theme.border }
    contentItem: ColumnLayout {
        Accessible.name: qsTr("Confirm task deletion")
        spacing: 14
        Text { text: qsTr("Delete this task?"); color: theme.text; font.pixelSize: 20; font.weight: Font.DemiBold }
        Text { text: qsTr("This task and its small steps will be removed."); color: theme.muted; font.pixelSize: 14; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        RowLayout {
            Layout.fillWidth: true; Layout.topMargin: 10
            Item { Layout.fillWidth: true }
            AppButton { theme: dialog.theme; text: qsTr("Cancel"); primary: false; onClicked: dialog.close() }
            AppButton { theme: dialog.theme; text: qsTr("Delete"); primary: false; danger: true; onClicked: { dialog.confirmed(dialog.targetId); dialog.close() } }
        }
    }
}
