import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme
    readonly property bool compact: width < 600
    readonly property bool focusHistoryShowcase: screenshotScenario === "focus-month-demo"
    readonly property bool landscapePreview: screenshotScenario === "garden-day-preview"
        || screenshotScenario === "garden-night-preview"
    property int localHour: new Date().getHours()
    readonly property bool nightGarden: landscapePreview ? screenshotScenario === "garden-night-preview"
        : localHour < 6 || localHour >= 18
    signal seedPlanted(string flowerName)

    onVisibleChanged: { if (visible) localHour = new Date().getHours() }
    Timer {
        running: page.visible && !page.landscapePreview
        interval: 15000
        repeat: true
        onTriggered: page.localHour = new Date().getHours()
    }
    Connections {
        target: Qt.application
        function onStateChanged() { if (Qt.application.state === Qt.ApplicationActive) page.localHour = new Date().getHours() }
    }

    function focusTimeText() {
        let minutes = Math.floor(focusManager.remainingSeconds / 60)
        let seconds = focusManager.remainingSeconds % 60
        return (minutes < 10 ? "0" : "") + minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }
    function resetFocus() {
        focusManager.reset()
    }
    function editFocusDuration() { focusDurationDialog.edit(true) }

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
    function showFocusHistory() {
        Qt.callLater(() => {
            const targetY = Math.max(0, panorama.y + panorama.height + content.spacing - 18)
            studyScroll.contentY = Math.min(targetY, Math.max(0, studyScroll.contentHeight - studyScroll.height))
        })
    }

    Flickable {
        id: studyScroll
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
                        Text {
                            text: page.focusHistoryShowcase ? qsTr("Demo data") : qsTr("Level %1").arg(growthManager.level)
                            color: theme.primary; font.pixelSize: 14; font.weight: Font.Bold
                        }
                        Text { visible: !page.focusHistoryShowcase; text: "·"; color: theme.muted }
                        Text { visible: !page.focusHistoryShowcase; text: qsTr("%1 XP").arg(growthManager.totalXp); color: theme.text; font.pixelSize: 13 }
                    }
                }

            }

            Rectangle {
                id: panorama
                visible: !page.focusHistoryShowcase
                Layout.fillWidth: true; Layout.preferredHeight: page.width < 760 ? 760 : 510
                radius: page.compact ? 18 : 24; clip: true; border.color: theme.border
                gradient: Gradient {
                    GradientStop { position: 0.0; color: page.nightGarden ? "#22394B" : "#DFEFE6" }
                    GradientStop { position: 0.62; color: page.nightGarden ? "#354D56" : "#F3EBD3" }
                    GradientStop { position: 1.0; color: page.nightGarden ? "#26323F" : "#C6A688" }
                }

                Rectangle {
                    id: gardenWindow
                    x: page.compact ? 14 : 22
                    y: 22
                    width: parent.width - 2 * x
                    height: page.compact ? 176 : 226
                    radius: 18
                    clip: true
                    border.width: 2
                    border.color: page.nightGarden ? "#627887" : "#F2F8EB"
                    gradient: Gradient {
                        GradientStop { position: 0; color: page.nightGarden ? "#14243F" : "#A4D5EC" }
                        GradientStop { position: 0.56; color: page.nightGarden ? "#365674" : "#D1EDF1" }
                        GradientStop { position: 1; color: page.nightGarden ? "#57787E" : "#FFF0C6" }
                    }

                    Repeater {
                        model: page.nightGarden ? 19 : 0
                        Rectangle {
                            required property int index
                            x: (0.05 + ((index * 37) % 85) / 100) * gardenWindow.width
                            y: 13 + (index * 17) % 82
                            width: index % 4 === 0 ? 4 : 2
                            height: width
                            radius: width / 2
                            color: index % 4 === 0 ? "#FFF3CE" : "#B8D1E6"
                            opacity: 0.48 + (index % 5) * 0.10
                        }
                    }

                    Rectangle {
                        x: gardenWindow.width - width - (page.compact ? 18 : 48)
                        y: 19
                        width: page.compact ? 40 : 60
                        height: width
                        radius: width / 2
                        color: Qt.alpha("#FFEDB5", page.nightGarden ? 0.14 : 0.35)
                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width - 12; height: width; radius: width / 2
                            color: page.nightGarden ? "#F3EDD4" : "#FFF3AD"
                        }
                    }

                    Repeater {
                        model: page.nightGarden ? 0 : (page.compact ? 1 : 3)
                        Rectangle {
                            required property int index
                            readonly property real cloudX: gardenWindow.width * (0.08 + index * 0.24)
                            x: cloudX
                            y: 38 + (index % 2) * 30
                            width: 58 + index * 10; height: 12; radius: 6
                            color: Qt.alpha("white", 0.68)
                            SequentialAnimation on x {
                                loops: Animation.Infinite; running: page.visible && theme.animationDuration > 0
                                NumberAnimation { from: cloudX; to: cloudX + 18; duration: 6400 + index * 850; easing.type: Easing.InOutSine }
                                NumberAnimation { from: cloudX + 18; to: cloudX; duration: 6400 + index * 850; easing.type: Easing.InOutSine }
                            }
                        }
                    }

                    Canvas {
                        anchors.fill: parent
                        anchors.margins: 2
                        readonly property bool night: page.nightGarden
                        onNightChanged: requestPaint()
                        onWidthChanged: requestPaint()
                        onHeightChanged: requestPaint()
                        onPaint: {
                            const brush = getContext("2d")
                            const w = width
                            const h = height
                            brush.clearRect(0, 0, w, h)

                            brush.beginPath()
                            brush.moveTo(0, h * 0.62)
                            brush.bezierCurveTo(w * 0.23, h * 0.28, w * 0.35, h * 0.68, w * 0.54, h * 0.55)
                            brush.bezierCurveTo(w * 0.72, h * 0.47, w * 0.84, h * 0.30, w, h * 0.51)
                            brush.lineTo(w, h)
                            brush.lineTo(0, h)
                            brush.closePath()
                            brush.fillStyle = night ? "#426776" : "#AECEB4"
                            brush.fill()

                            brush.beginPath()
                            brush.moveTo(0, h * 0.76)
                            brush.bezierCurveTo(w * 0.17, h * 0.59, w * 0.28, h * 0.80, w * 0.45, h * 0.73)
                            brush.bezierCurveTo(w * 0.68, h * 0.57, w * 0.84, h * 0.65, w, h * 0.82)
                            brush.lineTo(w, h)
                            brush.lineTo(0, h)
                            brush.closePath()
                            brush.fillStyle = night ? "#2E505C" : "#85B097"
                            brush.fill()
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
                        anchors.fill: parent; anchors.margins: 12; spacing: 10
                        Rectangle {
                            Layout.preferredWidth: 12; Layout.preferredHeight: 12; radius: 6
                            color: focusManager.running ? theme.primary : theme.border
                            SequentialAnimation on scale {
                                loops: Animation.Infinite
                                running: focusManager.running && theme.animationDuration > 0
                                NumberAnimation { from: 0.75; to: 1.25; duration: 720; easing.type: Easing.InOutSine }
                                NumberAnimation { from: 1.25; to: 0.75; duration: 720; easing.type: Easing.InOutSine }
                            }
                        }
                        ColumnLayout {
                            spacing: 1
                            Text { text: qsTr("Focus session"); color: theme.muted; font.pixelSize: 11; font.weight: Font.DemiBold }
                            Text { text: page.focusTimeText(); color: theme.text; font.pixelSize: 27; font.weight: Font.Bold }
                            AppButton {
                                theme: page.theme; primary: false; implicitHeight: 27
                                implicitWidth: contentItem.implicitWidth + 18; leftPadding: 9; rightPadding: 9
                                text: qsTr("Duration")
                                Accessible.name: qsTr("Edit focus duration")
                                onClicked: page.editFocusDuration()
                            }
                        }
                        Item { Layout.fillWidth: true }
                        ColumnLayout {
                            spacing: 4
                            AppButton {
                                theme: page.theme; implicitWidth: 76; implicitHeight: 34
                                text: focusManager.running ? qsTr("Pause") : (focusManager.sessionInProgress ? qsTr("Resume") : qsTr("Start"))
                                onClicked: focusManager.toggle()
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
                    height: page.width < 760 ? 520 : 258
                    radius: 18
                    color: page.nightGarden ? "#343740" : "#C8AA88"
                }
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 34
                    width: parent.width - 74; height: page.width < 760 ? 500 : 232; radius: 18
                    color: page.nightGarden ? "#4D5056" : "#E5CFB1"
                    border.color: Qt.alpha(theme.text, 0.1)
                }

                GridLayout {
                    anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                    anchors.leftMargin: 50; anchors.rightMargin: 50; anchors.bottomMargin: 44
                    height: page.width < 760 ? 488 : 300
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
                            color: Qt.alpha(theme.card, page.nightGarden ? 0.91 : 0.84)
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

            FocusCalendar {
                id: focusCalendar
                Layout.fillWidth: true
                theme: page.theme
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

    DurationDialog { id: focusDurationDialog; theme: page.theme }

    SeedPickerDialog {
        id: seedPicker; theme: page.theme
        onSeedSelected: (slot, species) => {
            if (growthManager.plantSeed(slot, species)) page.seedPlanted(page.speciesName(species))
        }
    }

}
