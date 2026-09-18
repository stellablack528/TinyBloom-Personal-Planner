import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme
    property url sourceFile
    signal confirmed(url file)

    modal: true
    anchors.centerIn: parent
    width: Math.min(470, parent ? parent.width - 40 : 470)
    padding: 22
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 16; color: theme.card; border.color: theme.border }

    contentItem: ColumnLayout {
        Accessible.name: qsTr("Confirm data import")
        spacing: 14
        Text {
            text: qsTr("Replace current TinyBloom data?")
            color: theme.text; font.pixelSize: 20; font.weight: Font.DemiBold
        }
        Text {
            Layout.fillWidth: true
            text: qsTr("Importing will replace all current tasks and settings. TinyBloom will create a safety backup first.")
            color: theme.muted; font.pixelSize: 14; wrapMode: Text.WordWrap
        }
        RowLayout {
            Layout.fillWidth: true; Layout.topMargin: 10
            Item { Layout.fillWidth: true }
            AppButton { theme: dialog.theme; text: qsTr("Cancel"); primary: false; onClicked: dialog.close() }
            AppButton {
                theme: dialog.theme; text: qsTr("Back Up & Import"); danger: true
                onClicked: {
                    dialog.confirmed(dialog.sourceFile)
                    dialog.close()
                }
            }
        }
    }
}
