import QtQuick
import QtQuick.Layouts

ColumnLayout {
    property QtObject theme
    property string title
    property string description
    signal actionRequested()
    spacing: 10
    Layout.alignment: Qt.AlignHCenter

    Text { text: "🌱"; font.pixelSize: 36; Layout.alignment: Qt.AlignHCenter }
    Text {
        text: parent.title; color: theme.text; font.pixelSize: 19; font.weight: Font.DemiBold
        Layout.alignment: Qt.AlignHCenter
    }
    Text {
        text: parent.description; color: theme.muted; font.pixelSize: 14
        horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; Layout.maximumWidth: 340
    }
    AppButton {
        theme: parent.theme; text: qsTr("+ Create Task"); Layout.alignment: Qt.AlignHCenter; Layout.topMargin: 8
        onClicked: parent.actionRequested()
    }
}
