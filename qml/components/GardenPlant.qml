import QtQuick

Item {
    id: plant
    property QtObject theme
    property string species: ""
    property int stage: 0
    property int vitalityState: 0
    readonly property bool planted: species.length > 0
    readonly property color leafColor: vitalityState === 0 ? theme.primary
        : vitalityState === 1 ? theme.warning : "#92745A"
    readonly property color leafLight: vitalityState === 0 ? theme.primaryHover
        : vitalityState === 1 ? "#C6A861" : "#AA8D72"
    readonly property color petalColor: vitalityState === 2 ? "#B69B86"
        : species === "sunflower" ? "#F7C948"
        : species === "tulip" ? "#F07B82"
        : species === "rose" ? "#D95D6A" : "#F4CFCB"
    readonly property color petalLight: vitalityState === 2 ? "#C8AE98"
        : species === "sunflower" ? "#FFE185"
        : species === "tulip" ? "#F6A0A5"
        : species === "rose" ? "#ED8791" : "#F8E1DC"
    readonly property color flowerCenter: species === "sunflower" ? "#6F4A2F"
        : species === "rose" ? "#A63F50" : theme.warning
    readonly property real droop: vitalityState === 0 ? 0 : vitalityState === 1 ? 15 : 34

    implicitWidth: 270
    implicitHeight: 290
    width: implicitWidth
    height: implicitHeight

    onSpeciesChanged: {
        if (planted) {
            opacity = 0.25
            revealAnimation.restart()
        }
    }
    NumberAnimation { id: revealAnimation; target: plant; property: "opacity"; to: 1; duration: theme.animationDuration + 420; easing.type: Easing.OutCubic }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 18
        width: 224; height: 30; radius: 15
        color: Qt.alpha(plant.theme.primary, plant.theme.dark ? 0.12 : 0.09)
    }
    Rectangle {
        id: pot
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 30
        width: 92; height: 66; radius: 16
        color: plant.vitalityState === 2 ? "#94705B" : "#AD7658"
        Rectangle { anchors.top: parent.top; width: parent.width; height: 15; radius: 8; color: "#754B3B" }
        Rectangle { anchors.horizontalCenter: parent.horizontalCenter; anchors.top: parent.top; anchors.topMargin: 7; width: 68; height: 13; radius: 7; color: "#4F392E" }
    }

    Item {
        id: growth
        anchors.fill: parent
        visible: plant.planted
        transformOrigin: Item.Bottom
        SequentialAnimation on rotation {
            loops: Animation.Infinite
            running: plant.planted && plant.vitalityState < 2 && plant.theme.animationDuration > 0
            NumberAnimation { from: -1.2; to: 1.2; duration: 1900; easing.type: Easing.InOutSine }
            NumberAnimation { from: 1.2; to: -1.2; duration: 1900; easing.type: Easing.InOutSine }
        }

        Rectangle {
            id: stem
            visible: plant.stage >= 1
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom; anchors.bottomMargin: 91
            width: 10; height: 78 + Math.min(plant.stage, 4) * 19
            radius: 5; color: plant.leafColor
            rotation: plant.vitalityState === 2 ? 7 : plant.vitalityState === 1 ? 2 : 0
            transformOrigin: Item.Bottom
            Behavior on height { NumberAnimation { duration: plant.theme.animationDuration + 300; easing.type: Easing.OutCubic } }
            Behavior on rotation { NumberAnimation { duration: plant.theme.animationDuration + 180 } }
        }
        Rectangle {
            visible: plant.stage >= 1
            x: parent.width / 2 - 57; y: stem.y + stem.height * 0.58
            width: 58; height: 29; radius: 22
            color: plant.leafColor; rotation: -29 + plant.droop; transformOrigin: Item.Right
            Behavior on rotation { NumberAnimation { duration: plant.theme.animationDuration + 180 } }
        }
        Rectangle {
            visible: plant.stage >= 2
            x: parent.width / 2 - 1; y: stem.y + stem.height * 0.38
            width: 64; height: 31; radius: 24
            color: plant.leafLight; rotation: 27 + plant.droop; transformOrigin: Item.Left
            Behavior on rotation { NumberAnimation { duration: plant.theme.animationDuration + 180 } }
        }
        Rectangle {
            visible: plant.stage === 3
            anchors.horizontalCenter: parent.horizontalCenter
            y: stem.y - 15; width: 36; height: 42; radius: 18
            color: plant.petalColor; rotation: plant.vitalityState === 2 ? 9 : 0
        }

        Item {
            id: flowerHead
            visible: plant.stage >= 4
            anchors.horizontalCenter: parent.horizontalCenter
            y: stem.y - 52; width: 96; height: 96
            rotation: plant.vitalityState === 2 ? 10 : 0

            Repeater {
                model: plant.species === "sunflower" ? 12 : plant.species === "tulip" ? 3 : plant.species === "rose" ? 10 : 6
                Rectangle {
                    required property int index
                    readonly property int count: plant.species === "sunflower" ? 12 : plant.species === "tulip" ? 3 : plant.species === "rose" ? 10 : 6
                    readonly property real angle: index * Math.PI * 2 / count
                    x: plant.species === "tulip" ? 29 + index * 12 : 35 + Math.cos(angle) * (plant.species === "rose" ? 20 : 29)
                    y: plant.species === "tulip" ? 31 - (index % 2) * 9 : 35 + Math.sin(angle) * (plant.species === "rose" ? 20 : 29)
                    width: plant.species === "sunflower" ? 25 : plant.species === "tulip" ? 30 : plant.species === "rose" ? 31 : 27
                    height: plant.species === "sunflower" ? 36 : plant.species === "tulip" ? 46 : plant.species === "rose" ? 34 : 39
                    radius: width / 2
                    color: index % 2 ? plant.petalColor : plant.petalLight
                    rotation: plant.species === "tulip" ? (index - 1) * 17 : index * 360 / count + 90
                    transformOrigin: Item.Center
                }
            }
            Rectangle {
                visible: plant.species !== "tulip"
                anchors.centerIn: parent
                width: plant.species === "sunflower" ? 38 : plant.species === "rose" ? 28 : 31
                height: width; radius: width / 2; color: plant.flowerCenter
                border.width: plant.species === "sunflower" ? 4 : 0; border.color: Qt.alpha("white", 0.16)
            }
        }

        Repeater {
            model: plant.stage >= 5 ? 2 : 0
            Rectangle {
                required property int index
                width: 30; height: 36; radius: 16
                x: index === 0 ? plant.width / 2 - 74 : plant.width / 2 + 43
                y: stem.y + 55 + index * 12
                color: index ? plant.petalColor : plant.petalLight
                rotation: index === 0 ? -24 : 24
            }
        }
        Rectangle {
            id: seed
            visible: plant.stage === 0
            anchors.horizontalCenter: parent.horizontalCenter
            y: pot.y - 8; width: 17; height: 11; radius: 6
            color: "#71513B"
            SequentialAnimation on scale {
                loops: Animation.Infinite
                running: seed.visible && plant.theme.animationDuration > 0
                NumberAnimation { from: 0.92; to: 1.08; duration: 850; easing.type: Easing.InOutSine }
                NumberAnimation { from: 1.08; to: 0.92; duration: 850; easing.type: Easing.InOutSine }
            }
        }
    }

    Text {
        visible: !plant.planted
        anchors.horizontalCenter: parent.horizontalCenter
        y: pot.y - 35; text: "+"; color: theme.muted; font.pixelSize: 25; font.weight: Font.Light
    }
}
