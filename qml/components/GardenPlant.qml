import QtQuick

Item {
    id: plant
    property QtObject theme
    property int stage: 0
    property int vitalityState: 0
    readonly property color leafColor: vitalityState === 0 ? theme.primary
        : vitalityState === 1 ? theme.warning : "#92745A"
    readonly property color leafLight: vitalityState === 0 ? theme.primaryHover
        : vitalityState === 1 ? "#C6A861" : "#AA8D72"
    readonly property real droop: vitalityState === 0 ? 0 : vitalityState === 1 ? 18 : 38

    implicitWidth: 390
    implicitHeight: 340
    width: implicitWidth
    height: implicitHeight

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 26
        width: 300; height: 46; radius: 23
        color: Qt.alpha(plant.theme.primarySoft, 0.7)
    }
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 40
        width: 112; height: 74; radius: 18
        color: plant.vitalityState === 2 ? "#94705B" : "#9A6B52"
        Rectangle { anchors.top: parent.top; width: parent.width; height: 16; radius: 8; color: "#754B3B" }
    }
    Rectangle {
        id: stem
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 104
        width: 12; height: plant.stage === 0 ? 18 : 105 + Math.min(plant.stage, 4) * 18
        radius: 6; color: plant.leafColor
        rotation: plant.vitalityState === 2 ? 7 : plant.vitalityState === 1 ? 2 : 0
        transformOrigin: Item.Bottom
        Behavior on height { NumberAnimation { duration: plant.theme.animationDuration + 260; easing.type: Easing.OutCubic } }
        Behavior on rotation { NumberAnimation { duration: plant.theme.animationDuration + 180 } }
    }
    Rectangle {
        visible: plant.stage >= 1
        x: parent.width / 2 - 70; y: stem.y + stem.height * 0.56
        width: 72; height: 38; radius: 28
        color: plant.leafColor
        rotation: -30 + plant.droop
        transformOrigin: Item.Right
        Behavior on rotation { NumberAnimation { duration: plant.theme.animationDuration + 180 } }
    }
    Rectangle {
        visible: plant.stage >= 2
        x: parent.width / 2 - 2; y: stem.y + stem.height * 0.38
        width: 80; height: 40; radius: 30
        color: plant.leafLight
        rotation: 28 + plant.droop
        transformOrigin: Item.Left
        Behavior on rotation { NumberAnimation { duration: plant.theme.animationDuration + 180 } }
    }
    Rectangle {
        visible: plant.stage >= 3
        anchors.horizontalCenter: parent.horizontalCenter
        y: stem.y - 16; width: plant.stage >= 4 ? 34 : 42; height: plant.stage >= 4 ? 34 : 46
        radius: width / 2; color: plant.vitalityState === 2 ? "#B69A7D" : theme.warning
    }
    Item {
        visible: plant.stage >= 4
        anchors.horizontalCenter: parent.horizontalCenter
        y: stem.y - 54; width: 106; height: 106
        rotation: plant.vitalityState === 2 ? 10 : 0
        Repeater {
            model: 6
            Rectangle {
                required property int index
                x: 39 + Math.cos(index * Math.PI / 3) * 32
                y: 39 + Math.sin(index * Math.PI / 3) * 32
                width: 28; height: 40; radius: 18
                color: plant.vitalityState === 2 ? "#C9AC91" : index % 2 ? "#F4CFCB" : "#F7DDD6"
                rotation: index * 60 + 90
                transformOrigin: Item.Center
            }
        }
        Rectangle { anchors.centerIn: parent; width: 34; height: 34; radius: 17; color: plant.theme.warning }
    }
    Repeater {
        model: plant.stage >= 5 ? 2 : 0
        Item {
            required property int index
            width: 66; height: 66
            x: index === 0 ? plant.width / 2 - 105 : plant.width / 2 + 40
            y: stem.y + 62 + index * 14
            Repeater {
                model: 5
                Rectangle {
                    required property int index
                    x: 22 + Math.cos(index * Math.PI * 2 / 5) * 20
                    y: 22 + Math.sin(index * Math.PI * 2 / 5) * 20
                    width: 20; height: 28; radius: 13
                    color: plant.vitalityState === 2 ? "#BEA48B" : "#EFC7CF"
                    rotation: index * 72 + 90
                }
            }
            Rectangle { anchors.centerIn: parent; width: 22; height: 22; radius: 11; color: plant.theme.warning }
        }
    }
    Text {
        visible: plant.stage === 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: stem.y - 32; text: "✦"; color: theme.warning; font.pixelSize: 22
    }
}
