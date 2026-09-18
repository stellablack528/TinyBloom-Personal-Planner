import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme
    property var targetId: -1
    signal confirmed(var id)
    modal: true; anchors.centerIn: parent; width: 390
    padding: 0; closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 16; color: theme.card; border.color: theme.border }
    contentItem: ColumnLayout {
        spacing: 14
        Text { text: "Delete this task?"; color: theme.text; font.pixelSize: 20; font.weight: Font.DemiBold }
        Text { text: "This task and its small steps will be removed."; color: theme.muted; font.pixelSize: 14; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        RowLayout {
            Layout.fillWidth: true; Layout.topMargin: 10
            Item { Layout.fillWidth: true }
            AppButton { theme: dialog.theme; text: "Cancel"; primary: false; onClicked: dialog.close() }
            AppButton { theme: dialog.theme; text: "Delete"; primary: false; danger: true; onClicked: { dialog.confirmed(dialog.targetId); dialog.close() } }
        }
    }
}
