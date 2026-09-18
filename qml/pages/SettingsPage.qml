import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme

    FileDialog {
        id: exportDialog; title: "Export TinyBloom data"; fileMode: FileDialog.SaveFile
        nameFilters: ["TinyBloom JSON (*.json)"]
        currentFile: "tinybloom-export.json"
        onAccepted: dataService.exportData(selectedFile)
    }
    FileDialog {
        id: importDialog; title: "Import TinyBloom data"; fileMode: FileDialog.OpenFile
        nameFilters: ["TinyBloom JSON (*.json)"]
        onAccepted: dataService.importData(selectedFile)
    }

    Flickable {
        anchors.fill: parent; contentHeight: content.implicitHeight + 80; clip: true
        ColumnLayout {
            id: content; x: 40; y: 40; width: Math.min(parent.width - 80, 780); spacing: 20
            Text { text: "Settings"; color: theme.text; font.pixelSize: 30; font.weight: Font.Bold }
            Text { text: "Make TinyBloom feel comfortable for you."; color: theme.muted; font.pixelSize: 14; Layout.bottomMargin: 6 }
            Repeater {
                model: [
                    {name:"Mint Garden", value:"mint", swatch:"#3D8B5D", desc:"Light, fresh and calm"},
                    {name:"Midnight", value:"midnight", swatch:"#73C995", desc:"Soft contrast for darker rooms"}
                ]
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true; height: 72; radius: theme.radius; color: theme.card
                    border.width: settingsManager.theme === modelData.value ? 2 : 1
                    border.color: settingsManager.theme === modelData.value ? theme.primary : theme.border
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 14
                        Rectangle { width: 36; height: 36; radius: 11; color: modelData.swatch }
                        ColumnLayout { Layout.fillWidth: true; spacing: 3
                            Text { text: modelData.name; color: theme.text; font.pixelSize: 15; font.weight: Font.DemiBold }
                            Text { text: modelData.desc; color: theme.muted; font.pixelSize: 12 }
                        }
                        Text { text: settingsManager.theme === modelData.value ? "✓" : ""; color: theme.primary; font.pixelSize: 18; font.weight: Font.Bold }
                    }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: settingsManager.theme = modelData.value }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: preferences.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: preferences; anchors.fill: parent; anchors.margins: 16; spacing: 14
                    Text { text: "Preferences"; color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout { Layout.fillWidth: true; spacing: 2
                            Text { text: "Reduce animations"; color: theme.text; font.pixelSize: 14 }
                            Text { text: "Use fewer motion effects throughout the app"; color: theme.muted; font.pixelSize: 12 }
                        }
                        Switch { checked: settingsManager.reduceAnimations; onToggled: settingsManager.reduceAnimations = checked }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout { Layout.fillWidth: true; spacing: 2
                            Text { text: "Default task duration"; color: theme.text; font.pixelSize: 14 }
                            Text { text: "Used for new tasks"; color: theme.muted; font.pixelSize: 12 }
                        }
                        SpinBox { from: 0; to: 480; editable: true; value: settingsManager.defaultTaskDuration; onValueModified: settingsManager.defaultTaskDuration = value }
                        Text { text: "minutes"; color: theme.muted; font.pixelSize: 12 }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: dataColumn.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: dataColumn; anchors.fill: parent; anchors.margins: 16; spacing: 12
                    Text { text: "Data"; color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    Text { text: "Your data stays on this device unless you export it."; color: theme.muted; font.pixelSize: 12 }
                    RowLayout {
                        AppButton { theme: page.theme; text: "Export Data"; primary: false; onClicked: exportDialog.open() }
                        AppButton { theme: page.theme; text: "Import Data"; primary: false; onClicked: importDialog.open() }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: aboutColumn.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: aboutColumn; anchors.fill: parent; anchors.margins: 16; spacing: 5
                    Text { text: "TinyBloom"; color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    Text { text: "Version 0.1.0"; color: theme.muted; font.pixelSize: 12 }
                    Text { text: "Small Steps, Real Progress."; color: theme.primary; font.pixelSize: 13; font.weight: Font.DemiBold; Layout.topMargin: 4 }
                    Text { text: "Free & Open Source · MIT License"; color: theme.muted; font.pixelSize: 12 }
                }
            }
        }
    }
}

