import QtQuick
import QtQuick.Layouts

Rectangle {
    property QtObject theme
    property int completed: 0
    property int total: 0
    Layout.fillWidth: true
    implicitHeight: 122
    radius: theme.radius
    color: theme.card
    border.color: theme.border

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 22; spacing: 12
        RowLayout {
            Layout.fillWidth: true
            Text { text: qsTr("Today's Progress"); color: theme.text; font.pixelSize: 16; font.weight: Font.DemiBold }
            Item { Layout.fillWidth: true }
            Text { text: qsTr("%1 / %2 completed").arg(completed).arg(total); color: theme.muted; font.pixelSize: 13 }
        }
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 10; radius: 5; color: theme.primarySoft
            Rectangle {
                width: parent.width * (total > 0 ? completed / total : 0); height: parent.height
                radius: 5; color: theme.primary
                Behavior on width { NumberAnimation { duration: theme.animationDuration + 100; easing.type: Easing.OutCubic } }
            }
        }
        Text {
            text: total === 0 ? qsTr("A clear day is a lovely place to begin.") : completed === total ? qsTr("Beautiful work — today's steps are complete.") : qsTr("One small step at a time.")
            color: theme.muted; font.pixelSize: 13
        }
    }
}
