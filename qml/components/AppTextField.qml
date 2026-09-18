import QtQuick
import QtQuick.Controls

TextField {
    id: control
    property QtObject theme
    implicitHeight: 44
    color: theme.text
    placeholderTextColor: theme.muted
    selectionColor: theme.primary
    selectedTextColor: "white"
    font.pixelSize: 14
    leftPadding: 14; rightPadding: 14
    background: Rectangle {
        radius: 10
        color: theme.input
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? theme.primary : theme.border
    }
}

