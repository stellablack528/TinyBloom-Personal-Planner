import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme

    Flickable {
        anchors.fill: parent; contentHeight: content.implicitHeight + 80; clip: true
        ColumnLayout {
            id: content; x: 40; y: 40; width: parent.width - 80; spacing: 20
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    spacing: 4
                    Text { text: qsTr("Your Garden"); color: theme.text; font.pixelSize: 30; font.weight: Font.Bold }
                    Text { text: qsTr("What you care for grows — and what you leave alone will wilt."); color: theme.muted; font.pixelSize: 14 }
                }
                Item { Layout.fillWidth: true }
                Rectangle {
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
                Layout.fillWidth: true; implicitHeight: page.width < 760 ? 720 : 430; radius: 24
                color: theme.card; border.color: theme.border
                GridLayout {
                    anchors.fill: parent; anchors.margins: 24; columnSpacing: 18; rowSpacing: 18
                    columns: page.width < 760 ? 1 : 2
                    Item {
                        Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumWidth: page.width < 760 ? 0 : 390
                        GardenPlant {
                            anchors.centerIn: parent; theme: page.theme
                            stage: growthManager.gardenStage; vitalityState: growthManager.vitalityState
                        }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 330; Layout.fillHeight: true; spacing: 12
                        Item { Layout.fillHeight: true }
                        Text { text: growthManager.gardenStageName; color: theme.text; font.pixelSize: 25; font.weight: Font.Bold }
                        Text { Layout.fillWidth: true; text: growthManager.gardenMessage; color: theme.muted; font.pixelSize: 14; wrapMode: Text.WordWrap; lineHeight: 1.25 }
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: theme.border; Layout.topMargin: 6; Layout.bottomMargin: 4 }
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: qsTr("Garden vitality"); color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold }
                            Item { Layout.fillWidth: true }
                            Text { text: growthManager.vitality + "%"; color: growthManager.vitalityState === 0 ? theme.primary : growthManager.vitalityState === 1 ? theme.warning : "#A47A61"; font.pixelSize: 14; font.weight: Font.Bold }
                        }
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 12; radius: 6; color: theme.input
                            Rectangle {
                                width: parent.width * growthManager.vitality / 100; height: parent.height; radius: 6
                                color: growthManager.vitalityState === 0 ? theme.primary : growthManager.vitalityState === 1 ? theme.warning : "#A47A61"
                                Behavior on width { NumberAnimation { duration: theme.animationDuration + 160; easing.type: Easing.OutCubic } }
                            }
                        }
                        Text { Layout.fillWidth: true; text: growthManager.vitalityMessage; color: theme.muted; font.pixelSize: 13; wrapMode: Text.WordWrap }
                        Text {
                            visible: growthManager.gardenStage < 5
                            text: qsTr("Next growth at %1 XP · %2 XP to go").arg(growthManager.nextStageXp).arg(Math.max(0, growthManager.nextStageXp - growthManager.totalXp))
                            color: theme.primary; font.pixelSize: 12; font.weight: Font.DemiBold; Layout.topMargin: 4
                        }
                        Item { Layout.fillHeight: true }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true; spacing: 16
                Repeater {
                    model: [
                        {value: qsTr("+20 XP"), title: qsTr("Complete a task"), detail: qsTr("Restores 28 vitality")},
                        {value: qsTr("+5 XP"), title: qsTr("Complete a small step"), detail: qsTr("Restores 10 vitality")},
                        {value: growthManager.progressDays, title: qsTr("Days with progress"), detail: qsTr("A quiet day costs 15 vitality")}
                    ]
                    delegate: Rectangle {
                        required property var modelData
                        Layout.fillWidth: true; implicitHeight: 108; radius: theme.radius; color: theme.card; border.color: theme.border
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 16; spacing: 4
                            Text { text: modelData.value; color: theme.primary; font.pixelSize: 20; font.weight: Font.Bold }
                            Text { text: modelData.title; color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                            Text { text: modelData.detail; color: theme.muted; font.pixelSize: 11 }
                        }
                    }
                }
            }
        }
    }
}
