import QtQuick
import QtQuick.Controls

Button {
    id: control
    property QtObject theme
    property bool primary: true
    property bool danger: false
    implicitHeight: 42
    implicitWidth: Math.max(104, contentItem.implicitWidth + 32)
    leftPadding: 16; rightPadding: 16
    hoverEnabled: true

    contentItem: Text {
        text: control.text
        color: control.danger ? theme.danger : control.primary ? "white" : theme.text
        font.pixelSize: 14
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
        radius: 10
        color: control.danger ? (control.hovered ? Qt.alpha(theme.danger, 0.14) : "transparent")
            : control.primary ? (control.hovered ? theme.primaryHover : theme.primary)
            : (control.hovered ? theme.cardHover : "transparent")
        border.color: control.primary ? "transparent" : (control.danger ? Qt.alpha(theme.danger, 0.4) : theme.border)
        Behavior on color { ColorAnimation { duration: theme.animationDuration } }
    }
}

