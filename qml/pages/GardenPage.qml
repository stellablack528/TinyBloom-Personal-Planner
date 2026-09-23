import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme
    readonly property bool compact: width < 600
    property int focusSeconds: 25 * 60
    property bool focusRunning: false
    signal focusSessionCompleted()
    signal seedPlanted(string flowerName)

    function focusTimeText() {
        let minutes = Math.floor(focusSeconds / 60)
        let seconds = focusSeconds % 60
        return (minutes < 10 ? "0" : "") + minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }
    function resetFocus() {
        focusRunning = false
        focusSeconds = 25 * 60
    }

    function speciesName(species) {
        if (species === "sunflower") return qsTr("Sunflower")
        if (species === "tulip") return qsTr("Tulip")
        if (species === "rose") return qsTr("Rose")
        return qsTr("Pink blossom")
    }
    function openSeeds(slot) {
        seedPicker.slot = slot
        seedPicker.replanting = slot === 0 ? growthManager.firstPlantSpecies.length > 0
            : growthManager.secondPlantSpecies.length > 0
        seedPicker.open()
    }

    Flickable {
        anchors.fill: parent; contentWidth: width; contentHeight: content.implicitHeight + 80; clip: true
        ScrollBar.vertical: ScrollBar {}
        ColumnLayout {
            id: content
            x: page.compact ? 14 : 40
            y: page.compact ? 16 : 36
            width: parent.width - (page.compact ? 28 : 80)
            spacing: page.compact ? 14 : 18
            GridLayout {
                Layout.fillWidth: true
                columns: page.compact ? 1 : 2
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text { text: qsTr("Bloom Study Room"); color: theme.text; font.pixelSize: page.compact ? 25 : 30; font.weight: Font.Bold }
                    Text { Layout.fillWidth: true; text: qsTr("A quiet room where focused effort becomes something alive."); color: theme.muted; font.pixelSize: page.compact ? 12 : 14; wrapMode: Text.WordWrap }
                }
                Rectangle {
                    Layout.alignment: page.compact ? Qt.AlignLeft : Qt.AlignRight
                    implicitWidth: levelRow.implicitWidth + 28; implicitHeight: 44; radius: 12; color: theme.primarySoft
                    RowLayout {
                        id: levelRow; anchors.centerIn: parent; spacing: 8
                        Text { text: qsTr("Level %1").arg(growthManager.level); color: theme.primary; font.pixelSize: 14; font.weight: Font.Bold }
                        Text { text: "·"; color: theme.muted }
                        Text { text: qsTr("%1 XP").arg(growthManager.totalXp); color: theme.text; font.pixelSize: 13 }
                    }
                }

            }

            Rectangle {
                id: panorama
                Layout.fillWidth: true; Layout.preferredHeight: page.width < 760 ? 760 : 510
                radius: page.compact ? 18 : 24; clip: true; border.color: theme.border
                gradient: Gradient {
                    GradientStop { position: 0.0; color: theme.dark ? "#172A28" : "#DFF2ED" }
                    GradientStop { position: 0.58; color: theme.dark ? "#21332C" : "#F5EEDC" }
                    GradientStop { position: 1.0; color: theme.dark ? "#1A211C" : "#D9C2A3" }
                }

                Rectangle {
                    x: 26; y: 28; width: parent.width - 52; height: page.width < 760 ? 188 : 162
                    radius: 18; color: theme.dark ? "#183D45" : "#C8E8F2"; border.color: Qt.alpha(theme.text, 0.1)
                    Rectangle { x: parent.width / 2 - 3; width: 6; height: parent.height; color: Qt.alpha(theme.text, 0.1) }
                    Rectangle { y: parent.height / 2 - 3; width: parent.width; height: 6; color: Qt.alpha(theme.text, 0.1) }
                    Rectangle { x: parent.width - 76; y: 24; width: 34; height: 34; radius: 17; color: theme.dark ? "#F1D58A" : "#FFF1A8" }
                    Repeater {
                        model: 3
                        Rectangle {
                            required property int index
                            x: 42 + index * 128; y: 42 + (index % 2) * 46
                            width: 76 + index * 9; height: 18; radius: 9; color: Qt.alpha("white", theme.dark ? 0.14 : 0.62)
                            SequentialAnimation on x {
                                loops: Animation.Infinite; running: theme.animationDuration > 0
                                NumberAnimation { from: 42 + index * 128; to: 72 + index * 128; duration: 5000 + index * 700; easing.type: Easing.InOutSine }
                                NumberAnimation { from: 72 + index * 128; to: 42 + index * 128; duration: 5000 + index * 700; easing.type: Easing.InOutSine }
                            }
                        }
                    }
                }

                Repeater {
                    model: 10
                    Rectangle {
                        required property int index
                        x: 40 + (index * 83) % Math.max(120, panorama.width - 80)
                        y: 105 + (index % 4) * 62
                        width: 4 + index % 3; height: width; radius: width / 2
                        color: Qt.alpha(theme.dark ? "#F4DA91" : "#FFFFFF", 0.55)
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite; running: theme.animationDuration > 0
                            NumberAnimation { from: 0.18; to: 0.78; duration: 1200 + index * 110 }
                            NumberAnimation { from: 0.78; to: 0.18; duration: 1200 + index * 110 }
                        }
                    }
                }

                Rectangle {
                    id: focusPanel
                    anchors.horizontalCenter: parent.horizontalCenter; y: 20; z: 10
                    width: Math.min(272, parent.width - 24); height: 104; radius: 17
                    color: Qt.alpha(theme.card, theme.dark ? 0.84 : 0.9)
                    border.color: Qt.alpha(theme.primary, 0.24)
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 14; spacing: 12
                        Rectangle {
                            Layout.preferredWidth: 12; Layout.preferredHeight: 12; radius: 6
                            color: page.focusRunning ? theme.primary : theme.border
                            SequentialAnimation on scale {
                                loops: Animation.Infinite
                                running: page.focusRunning && theme.animationDuration > 0
                                NumberAnimation { from: 0.75; to: 1.25; duration: 720; easing.type: Easing.InOutSine }
                                NumberAnimation { from: 1.25; to: 0.75; duration: 720; easing.type: Easing.InOutSine }
                            }
                        }
                        ColumnLayout {
                            spacing: 1
                            Text { text: qsTr("Focus session"); color: theme.muted; font.pixelSize: 11; font.weight: Font.DemiBold }
                            Text { text: page.focusTimeText(); color: theme.text; font.pixelSize: 27; font.weight: Font.Bold }
                        }
                        Item { Layout.fillWidth: true }
                        ColumnLayout {
                            spacing: 4
                            AppButton {
                                theme: page.theme; implicitWidth: 76; implicitHeight: 34
                                text: page.focusRunning ? qsTr("Pause") : qsTr("Start")
                                onClicked: page.focusRunning = !page.focusRunning
                            }
                            AppButton {
                                theme: page.theme; implicitWidth: 76; implicitHeight: 28
                                text: qsTr("Reset"); primary: false
                                onClicked: page.resetFocus()
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                    height: page.width < 760 ? 520 : 286; color: theme.dark ? "#202820" : "#CBB08C"
                }
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 34
                    width: parent.width - 74; height: page.width < 760 ? 500 : 232; radius: 18
                    color: theme.dark ? "#26342B" : "#E8D4B7"; border.color: Qt.alpha(theme.text, 0.1)
                }

                GridLayout {
                    anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                    anchors.leftMargin: 50; anchors.rightMargin: 50; anchors.bottomMargin: 44
                    height: page.width < 760 ? 488 : 338
                    columns: page.width < 760 ? 1 : 2; columnSpacing: 20; rowSpacing: 12
                    Repeater {
                        model: [
                            {slot: 0, species: growthManager.firstPlantSpecies},
                            {slot: 1, species: growthManager.secondPlantSpecies}
                        ]
                        delegate: Rectangle {
                            id: plantRoom
                            required property var modelData
                            Layout.fillWidth: true; Layout.fillHeight: true; radius: 18
                            color: Qt.alpha(theme.card, theme.dark ? 0.68 : 0.78)
                            border.color: Qt.alpha(theme.text, 0.1)
                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 0
                                RowLayout {
                                    Layout.fillWidth: true
                                    Text { text: qsTr("Flower pot %1").arg(plantRoom.modelData.slot + 1); color: theme.muted; font.pixelSize: 11; font.weight: Font.DemiBold }
                                    Item { Layout.fillWidth: true }
                                    Text {
                                        visible: plantRoom.modelData.species.length > 0
                                        text: page.speciesName(plantRoom.modelData.species); color: theme.primary; font.pixelSize: 12; font.weight: Font.DemiBold
                                    }
                                    AppButton {
                                        visible: plantRoom.modelData.species.length > 0
                                        theme: page.theme; text: qsTr("Replant"); primary: false
                                        implicitWidth: 72; implicitHeight: 29
                                        onClicked: page.openSeeds(plantRoom.modelData.slot)
                                    }
                                }
                                Item {
                                    Layout.fillWidth: true; Layout.fillHeight: true
                                    GardenPlant {
                                        anchors.centerIn: parent; theme: page.theme
                                        species: plantRoom.modelData.species
                                        stage: {
                                            let currentXp = growthManager.totalXp
                                            return growthManager.plantStage(plantRoom.modelData.slot)
                                        }
                                        vitalityState: growthManager.vitalityState
                                        scale: page.width < 760 ? 0.72 : 0.8
                                    }
                                    AppButton {
                                        anchors.centerIn: parent; anchors.verticalCenterOffset: 42
                                        visible: plantRoom.modelData.species.length === 0
                                        theme: page.theme; text: qsTr("Choose a seed and plant")
                                        onClicked: page.openSeeds(plantRoom.modelData.slot)
                                    }
                                }
                                RowLayout {
                                    Layout.fillWidth: true; visible: plantRoom.modelData.species.length > 0
                                    Text {
                                        text: {
                                            let currentXp = growthManager.totalXp
                                            return growthManager.plantStageName(plantRoom.modelData.slot)
                                        }
                                        color: theme.text; font.pixelSize: 12; font.weight: Font.DemiBold
                                    }
                                    Item { Layout.fillWidth: true }
                                    Text {
                                        text: {
                                            let currentXp = growthManager.totalXp
                                            return qsTr("%1 XP since planting").arg(growthManager.plantEarnedXp(plantRoom.modelData.slot))
                                        }
                                        color: theme.muted; font.pixelSize: 11
                                    }
                                }
                            }
                        }
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true; columns: page.compact ? 1 : 3; columnSpacing: 14; rowSpacing: 10
                Repeater {
                    model: [
                        {value: growthManager.vitality + "%", title: qsTr("Room vitality"), detail: growthManager.vitalityMessage},
                        {value: qsTr("+20 XP"), title: qsTr("Complete a task"), detail: qsTr("Restores 28 vitality")},
                        {value: growthManager.progressDays, title: qsTr("Days with progress"), detail: qsTr("A quiet day costs 15 vitality")}
                    ]
                    delegate: Rectangle {
                        required property var modelData
                        Layout.fillWidth: true; implicitHeight: 106; radius: theme.radius; color: theme.card; border.color: theme.border
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 15; spacing: 4
                            Text { text: modelData.value; color: theme.primary; font.pixelSize: 19; font.weight: Font.Bold }
                            Text { text: modelData.title; color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                            Text { Layout.fillWidth: true; text: modelData.detail; color: theme.muted; font.pixelSize: 11; elide: Text.ElideRight }
                        }
                    }
                }
            }
        }
    }

    SeedPickerDialog {
        id: seedPicker; theme: page.theme
        onSeedSelected: (slot, species) => {
            if (growthManager.plantSeed(slot, species)) page.seedPlanted(page.speciesName(species))
        }
    }

    Timer {
        interval: 1000; repeat: true; running: page.focusRunning
        onTriggered: {
            if (page.focusSeconds > 1) {
                --page.focusSeconds
            } else {
                page.focusSeconds = 0
                page.focusRunning = false
                page.focusSessionCompleted()
            }
        }
    }
}
