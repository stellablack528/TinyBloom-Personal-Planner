import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme
    property int slot: 0
    property bool replanting: false
    signal seedSelected(int slot, string species)
    modal: true; anchors.centerIn: parent
    width: Math.min(560, parent ? parent.width - 48 : 560)
    padding: 22; closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 18; color: theme.card; border.color: theme.border }

    contentItem: ColumnLayout {
        spacing: 16
        Text {
            text: dialog.replanting ? qsTr("Choose a new seed for flower pot %1").arg(dialog.slot + 1)
                : qsTr("Choose a seed for flower pot %1").arg(dialog.slot + 1)
            color: theme.text; font.pixelSize: 21; font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            text: dialog.replanting ? qsTr("Replanting resets this flower's growth progress.")
                : qsTr("Each flower grows from the progress you make after planting it.")
            color: theme.muted; font.pixelSize: 13; wrapMode: Text.WordWrap
        }
        GridLayout {
            Layout.fillWidth: true; columns: 2; columnSpacing: 12; rowSpacing: 12
            Repeater {
                model: [
                    {key: "pink", icon: "🌸", name: qsTr("Pink blossom"), detail: qsTr("Soft and quietly hopeful")},
                    {key: "sunflower", icon: "🌻", name: qsTr("Sunflower"), detail: qsTr("Bright and full of energy")},
                    {key: "tulip", icon: "🌷", name: qsTr("Tulip"), detail: qsTr("Simple and determined")},
                    {key: "rose", icon: "🌹", name: qsTr("Rose"), detail: qsTr("Warm and resilient")}
                ]
                delegate: Rectangle {
                    id: seedCard
                    required property var modelData
                    Layout.fillWidth: true; Layout.preferredHeight: 92; radius: 13
                    color: seedMouse.containsMouse ? theme.primarySoft : theme.input
                    border.width: activeFocus ? 2 : 1; border.color: activeFocus ? theme.primary : theme.border
                    activeFocusOnTab: true
                    Accessible.role: Accessible.Button
                    Accessible.name: modelData.name
                    Accessible.focusable: true
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 14; spacing: 12
                        Text { text: seedCard.modelData.icon; font.pixelSize: 30 }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 3
                            Text { text: seedCard.modelData.name; color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold }
                            Text { Layout.fillWidth: true; text: seedCard.modelData.detail; color: theme.muted; font.pixelSize: 11; wrapMode: Text.WordWrap }
                        }
                    }
                    function choose() { dialog.seedSelected(dialog.slot, modelData.key); dialog.close() }
                    MouseArea { id: seedMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: seedCard.choose() }
                    Keys.onReturnPressed: event => { seedCard.choose(); event.accepted = true }
                    Keys.onSpacePressed: event => { seedCard.choose(); event.accepted = true }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.topMargin: 4
            Item { Layout.fillWidth: true }
            AppButton { theme: dialog.theme; text: qsTr("Cancel"); primary: false; onClicked: dialog.close() }
        }
    }
}
