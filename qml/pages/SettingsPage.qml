import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme

    FileDialog {
        id: exportDialog; title: qsTr("Export TinyBloom data"); fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("TinyBloom JSON (*.json)")]
        currentFile: "tinybloom-export.json"
        onAccepted: dataService.exportData(selectedFile)
    }
    FileDialog {
        id: importDialog; title: qsTr("Import TinyBloom data"); fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("TinyBloom JSON (*.json)")]
        onAccepted: {
            importConfirm.sourceFile = selectedFile
            importConfirm.open()
        }
    }
    ImportConfirmDialog {
        id: importConfirm; theme: page.theme
        onConfirmed: file => dataService.importData(file)
    }

    Flickable {
        anchors.fill: parent; contentHeight: content.implicitHeight + 80; clip: true
        ColumnLayout {
            id: content; x: 40; y: 40; width: Math.min(parent.width - 80, 780); spacing: 20
            Text { text: qsTr("Settings"); color: theme.text; font.pixelSize: 30; font.weight: Font.Bold }
            Text { text: qsTr("Make TinyBloom feel comfortable for you."); color: theme.muted; font.pixelSize: 14; Layout.bottomMargin: 6 }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: languageRow.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                RowLayout {
                    id: languageRow; anchors.fill: parent; anchors.margins: 16; spacing: 16
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 3
                        Text { text: qsTr("Language"); color: theme.text; font.pixelSize: 15; font.weight: Font.DemiBold }
                        Text { text: qsTr("Choose the language used throughout TinyBloom"); color: theme.muted; font.pixelSize: 12 }
                    }
                    ComboBox {
                        id: languageBox; model: [qsTr("简体中文"), qsTr("English")]
                        Accessible.name: qsTr("Language")
                        currentIndex: settingsManager.language === "zh_CN" ? 0 : 1
                        onActivated: settingsManager.language = currentIndex === 0 ? "zh_CN" : "en"
                        contentItem: Text { leftPadding: 12; text: languageBox.displayText; color: theme.text; verticalAlignment: Text.AlignVCenter }
                        background: Rectangle { implicitWidth: 150; radius: 10; color: theme.input; border.color: theme.border }
                    }
                }
            }
            Repeater {
                model: [
                    {name:qsTr("Mint Garden"), value:"mint", swatch:"#3D8B5D", desc:qsTr("Light, fresh and calm")},
                    {name:qsTr("Midnight"), value:"midnight", swatch:"#73C995", desc:qsTr("Soft contrast for darker rooms")}
                ]
                delegate: Rectangle {
                    id: themeChoice
                    required property var modelData
                    Layout.fillWidth: true; height: 72; radius: theme.radius; color: theme.card
                    border.width: settingsManager.theme === modelData.value ? 2 : 1
                    border.color: activeFocus || settingsManager.theme === modelData.value ? theme.primary : theme.border
                    activeFocusOnTab: true
                    Accessible.role: Accessible.RadioButton
                    Accessible.name: modelData.name
                    Accessible.description: modelData.desc
                    Accessible.checked: settingsManager.theme === modelData.value
                    Accessible.focusable: true
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 14
                        Rectangle { width: 36; height: 36; radius: 11; color: modelData.swatch }
                        ColumnLayout { Layout.fillWidth: true; spacing: 3
                            Text { text: modelData.name; color: theme.text; font.pixelSize: 15; font.weight: Font.DemiBold }
                            Text { text: modelData.desc; color: theme.muted; font.pixelSize: 12 }
                        }
                        Text { text: settingsManager.theme === modelData.value ? "✓" : ""; color: theme.primary; font.pixelSize: 18; font.weight: Font.Bold }
                    }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { themeChoice.forceActiveFocus(); settingsManager.theme = modelData.value } }
                    Keys.onReturnPressed: event => { settingsManager.theme = modelData.value; event.accepted = true }
                    Keys.onSpacePressed: event => { settingsManager.theme = modelData.value; event.accepted = true }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: preferences.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: preferences; anchors.fill: parent; anchors.margins: 16; spacing: 14
                    Text { text: qsTr("Preferences"); color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout { Layout.fillWidth: true; spacing: 2
                            Text { text: qsTr("Reduce animations"); color: theme.text; font.pixelSize: 14 }
                            Text { text: qsTr("Use fewer motion effects throughout the app"); color: theme.muted; font.pixelSize: 12 }
                        }
                        Switch { Accessible.name: qsTr("Reduce animations"); checked: settingsManager.reduceAnimations; onToggled: settingsManager.reduceAnimations = checked }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout { Layout.fillWidth: true; spacing: 2
                            Text { text: qsTr("Default task duration"); color: theme.text; font.pixelSize: 14 }
                            Text { text: qsTr("Used for new tasks"); color: theme.muted; font.pixelSize: 12 }
                        }
                        SpinBox { Accessible.name: qsTr("Default task duration in minutes"); from: 0; to: 480; editable: true; value: settingsManager.defaultTaskDuration; onValueModified: settingsManager.defaultTaskDuration = value }
                        Text { text: qsTr("minutes"); color: theme.muted; font.pixelSize: 12 }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: dataColumn.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: dataColumn; anchors.fill: parent; anchors.margins: 16; spacing: 12
                    Text { text: qsTr("Data"); color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    Text { text: qsTr("Your data stays on this device unless you export it."); color: theme.muted; font.pixelSize: 12 }
                    RowLayout {
                        AppButton { theme: page.theme; text: qsTr("Export Data"); primary: false; onClicked: exportDialog.open() }
                        AppButton { theme: page.theme; text: qsTr("Import Data"); primary: false; onClicked: importDialog.open() }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: aboutColumn.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: aboutColumn; anchors.fill: parent; anchors.margins: 16; spacing: 5
                    Text { text: "TinyBloom Desktop"; color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    Text { text: qsTr("Windows desktop · Version 0.2.0"); color: theme.muted; font.pixelSize: 12 }
                    Text { text: qsTr("Small Steps, Real Progress."); color: theme.primary; font.pixelSize: 13; font.weight: Font.DemiBold; Layout.topMargin: 4 }
                    Text { text: qsTr("Free & Open Source · MIT License"); color: theme.muted; font.pixelSize: 12 }
                }
            }
        }
    }
}
