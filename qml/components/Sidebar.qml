import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    property QtObject theme
    property int currentIndex: 0
    signal pageRequested(int index)
    color: theme.sidebar
    implicitWidth: 230

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 20; spacing: 8
        RowLayout {
            Layout.fillWidth: true; Layout.bottomMargin: 28; spacing: 10
            Rectangle {
                width: 38; height: 38; radius: 12; color: theme.primarySoft
                Text { anchors.centerIn: parent; text: "🌱"; font.pixelSize: 20 }
            }
            Text { text: "TinyBloom"; color: theme.text; font.pixelSize: 20; font.weight: Font.Bold }
        }
        Repeater {
            model: [
                {label:qsTr("Today"), icon:"◉"},
                {label:qsTr("Tasks"), icon:"☰"},
                {label:qsTr("Big Tasks"), icon:"⌘"},
                {label:qsTr("Study Room"), icon:"✿"},
                {label:qsTr("Settings"), icon:"⚙"}
            ]
            delegate: Rectangle {
                id: navigationItem
                required property var modelData
                required property int index
                Layout.fillWidth: true; height: 46; radius: 11
                color: sidebar.currentIndex === index ? theme.primarySoft : itemMouse.containsMouse ? theme.cardHover : "transparent"
                border.width: activeFocus ? 2 : 0
                border.color: theme.primary
                activeFocusOnTab: true
                Accessible.role: Accessible.Button
                Accessible.name: modelData.label
                Accessible.focusable: true
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 14; anchors.rightMargin: 14; spacing: 12
                    Text { text: modelData.icon; color: sidebar.currentIndex === index ? theme.primary : theme.muted; font.pixelSize: 16 }
                    Text { text: modelData.label; color: sidebar.currentIndex === index ? theme.text : theme.muted; font.pixelSize: 14; font.weight: sidebar.currentIndex === index ? Font.DemiBold : Font.Normal }
                    Item { Layout.fillWidth: true }
                }
                MouseArea {
                    id: itemMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: { navigationItem.forceActiveFocus(); sidebar.pageRequested(index) }
                }
                Keys.onReturnPressed: event => { sidebar.pageRequested(index); event.accepted = true }
                Keys.onSpacePressed: event => { sidebar.pageRequested(index); event.accepted = true }
            }
        }
        Item { Layout.fillHeight: true }
        Text {
            Layout.fillWidth: true; text: qsTr("Small Steps,\nReal Progress."); color: theme.muted
            font.pixelSize: 12; lineHeight: 1.35
        }
    }
}
