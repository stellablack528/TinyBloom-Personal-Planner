import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme
    readonly property bool compact: width < 600
    signal updateRequested()

    DurationDialog { id: durationDialog; theme: page.theme }

    FileDialog {
        id: focusSoundDialog
        title: qsTr("Import a focus sound")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Audio files (*.wav *.mp3)")]
        onAccepted: desktopIntegration.importFocusSound(selectedFile)
    }

    FileDialog {
        id: exportDialog; title: qsTr("Export TinyBloom data"); fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("TinyBloom JSON (*.json)")]
        onAccepted: dataService.exportDataAsync(selectedFile)
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
            id: content
            x: page.compact ? 16 : 40
            y: page.compact ? 16 : 40
            width: Math.min(parent.width - (page.compact ? 32 : 80), 780)
            spacing: page.compact ? 14 : 20
            Text { text: qsTr("Settings"); color: theme.text; font.pixelSize: page.compact ? 25 : 30; font.weight: Font.Bold }
            Text { text: qsTr("Make TinyBloom feel comfortable for you."); color: theme.muted; font.pixelSize: page.compact ? 12 : 14; Layout.bottomMargin: 6 }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: languageRow.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                GridLayout {
                    id: languageRow; anchors.fill: parent; anchors.margins: 16
                    columnSpacing: 16; rowSpacing: 10
                    columns: page.compact ? 1 : 2
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 3
                        Text { text: qsTr("Language"); color: theme.text; font.pixelSize: 15; font.weight: Font.DemiBold }
                        Text { Layout.fillWidth: true; text: qsTr("Choose the language used throughout TinyBloom"); color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap }
                    }
                    ComboBox {
                        id: languageBox; model: [qsTr("简体中文"), qsTr("English")]
                        Layout.fillWidth: page.compact
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
                            Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: qsTr("Use fewer motion effects throughout the app"); color: theme.muted; font.pixelSize: 12 }
                        }
                        Switch { Accessible.name: qsTr("Reduce animations"); checked: settingsManager.reduceAnimations; onToggled: settingsManager.reduceAnimations = checked }
                    }
                    RowLayout {
                        visible: !!desktopIntegration
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: qsTr("Focus end sound"); color: theme.text; font.pixelSize: 14 }
                            Text {
                                Layout.fillWidth: true; wrapMode: Text.WordWrap
                                text: qsTr("Play a short sound when a focus session ends")
                                color: theme.muted; font.pixelSize: 12
                            }
                        }
                        ComboBox {
                            id: soundStyle
                            model: {
                                const options = [
                                    {label: qsTr("Wind chime"), value: "wind"},
                                    {label: qsTr("Soft glass"), value: "glass"}
                                ]
                                if (desktopIntegration && desktopIntegration.customFocusSoundAvailable)
                                    options.push({label: qsTr("Custom"), value: "custom"})
                                return options
                            }
                            textRole: "label"
                            valueRole: "value"
                            currentValue: settingsManager.focusEndSoundStyle === "custom"
                                && !(desktopIntegration && desktopIntegration.customFocusSoundAvailable)
                                ? "wind" : settingsManager.focusEndSoundStyle
                            onActivated: settingsManager.focusEndSoundStyle = currentValue
                            Accessible.name: qsTr("Focus sound style")
                            Layout.preferredWidth: 142
                            implicitHeight: 34
                            palette.base: theme.card
                            palette.window: theme.card
                            palette.text: theme.text
                            palette.dark: theme.muted
                            palette.mid: theme.border
                            palette.highlight: theme.primarySoft
                            palette.highlightedText: theme.text
                            contentItem: Text {
                                text: soundStyle.displayText
                                leftPadding: 12
                                verticalAlignment: Text.AlignVCenter
                                color: theme.text
                            }
                            background: Rectangle {
                                radius: 10
                                color: theme.input
                                border.color: soundStyle.activeFocus ? theme.primary : theme.border
                            }
                        }
                        AppButton {
                            theme: page.theme
                            primary: false
                            text: qsTr("Preview")
                            Accessible.name: qsTr("Preview focus end sound")
                            implicitWidth: contentItem.implicitWidth + 24
                            leftPadding: 12; rightPadding: 12
                            implicitHeight: 34
                            onClicked: desktopIntegration.playFocusEndSound()
                        }
                        Switch {
                            Accessible.name: qsTr("Focus end sound")
                            checked: settingsManager.focusEndSound
                            onToggled: settingsManager.focusEndSound = checked
                        }
                    }
                    RowLayout {
                        visible: !!desktopIntegration
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: qsTr("Custom focus sound"); color: theme.text; font.pixelSize: 14 }
                            Text {
                                Layout.fillWidth: true; wrapMode: Text.WordWrap
                                text: qsTr("WAV or MP3 · Uses the first 5 seconds")
                                color: theme.muted; font.pixelSize: 12
                            }
                        }
                        AppButton {
                            theme: page.theme
                            primary: false
                            implicitHeight: 34
                            text: desktopIntegration && desktopIntegration.customSoundImporting
                                ? qsTr("Importing…") : qsTr("Import…")
                            enabled: !!desktopIntegration && !desktopIntegration.customSoundImporting
                            onClicked: focusSoundDialog.open()
                        }
                    }
                    RowLayout {
                        visible: !!desktopIntegration
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: qsTr("Close to system tray"); color: theme.text; font.pixelSize: 14 }
                            Text {
                                Layout.fillWidth: true; wrapMode: Text.WordWrap
                                text: qsTr("Click × to hide the window and keep focusing. Reopen from the leaf near the clock.")
                                color: theme.muted; font.pixelSize: 12
                            }
                        }
                        Switch {
                            Accessible.name: qsTr("Close to system tray")
                            checked: settingsManager.closeToTray
                            onToggled: settingsManager.closeToTray = checked
                        }
                    }
                    RowLayout {
                        visible: !(mobilePlatform || mobilePreview)
                        Layout.fillWidth: true
                        ColumnLayout { Layout.fillWidth: true; spacing: 2
                            Text { text: qsTr("Compatibility rendering"); color: theme.text; font.pixelSize: 14 }
                            Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: qsTr("Use software rendering after restart if the graphics driver is unstable"); color: theme.muted; font.pixelSize: 12 }
                        }
                        Switch {
                            Accessible.name: qsTr("Compatibility rendering")
                            checked: settingsManager.compatibilityRendering
                            onToggled: settingsManager.compatibilityRendering = checked
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout { Layout.fillWidth: true; spacing: 2
                            Text { text: qsTr("Default task estimate"); color: theme.text; font.pixelSize: 14 }
                            Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: qsTr("Estimate for new tasks"); color: theme.muted; font.pixelSize: 12 }
                        }
                        AppButton {
                            theme: page.theme; primary: false; implicitWidth: 106; implicitHeight: 36
                            text: qsTr("%1 min").arg(settingsManager.defaultTaskDuration)
                            Accessible.name: qsTr("Edit default task estimate")
                            onClicked: durationDialog.edit(false)
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout { Layout.fillWidth: true; spacing: 2
                            Text { text: qsTr("Focus duration"); color: theme.text; font.pixelSize: 14 }
                            Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: qsTr("Shared with the Study Room timer"); color: theme.muted; font.pixelSize: 12 }
                        }
                        AppButton {
                            theme: page.theme; primary: false; implicitWidth: 106; implicitHeight: 36
                            text: qsTr("%1 min").arg(settingsManager.focusDurationMinutes)
                            Accessible.name: qsTr("Edit focus duration")
                            onClicked: durationDialog.edit(true)
                        }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: updateColumn.implicitHeight + 32
                radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: updateColumn; anchors.fill: parent; anchors.margins: 16; spacing: 10
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 3
                            Text { text: qsTr("Software updates"); color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                            Text {
                                Layout.fillWidth: true
                                text: qsTr("Current version: %1").arg(updateManager.currentVersion)
                                color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap
                            }
                        }
                        Rectangle {
                            visible: updateManager.updateAvailable
                            implicitWidth: updateBadge.implicitWidth + 20; implicitHeight: 28
                            radius: 14; color: theme.primarySoft
                            Text { id: updateBadge; anchors.centerIn: parent; text: qsTr("New %1").arg(updateManager.latestVersion); color: theme.primary; font.pixelSize: 11; font.weight: Font.Bold }
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: updateManager.checking ? qsTr("Checking GitHub for updates...")
                            : updateManager.errorMessage.length > 0 ? updateManager.errorMessage
                            : updateManager.updateAvailable ? qsTr("A new version is ready. Open the update announcement to see what changed.")
                            : updateManager.checked ? qsTr("You are using the latest published version.")
                            : qsTr("TinyBloom can check GitHub Releases and choose the correct download for this device.")
                        color: updateManager.errorMessage.length > 0 ? theme.danger : theme.muted
                        font.pixelSize: 12; wrapMode: Text.WordWrap
                    }
                    RowLayout {
                        AppButton {
                            theme: page.theme; primary: updateManager.updateAvailable
                            text: updateManager.checking ? qsTr("Checking...")
                                : updateManager.updateAvailable ? qsTr("View update") : qsTr("Check for updates")
                            enabled: !updateManager.checking
                            onClicked: updateManager.updateAvailable ? page.updateRequested() : updateManager.checkForUpdates(true)
                        }
                        AppButton {
                            visible: updateManager.checked && updateManager.latestVersion.length > 0
                            theme: page.theme; primary: false; text: qsTr("Release page")
                            onClicked: updateManager.openReleasePage()
                        }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: dataColumn.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: dataColumn; anchors.fill: parent; anchors.margins: 16; spacing: 12
                    Text { text: qsTr("Data"); color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    Text { Layout.fillWidth: true; text: qsTr("Your data stays on this device unless you export it."); color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap }
                    RowLayout {
                        AppButton {
                            theme: page.theme; text: dataService.busy ? qsTr("Working...") : qsTr("Export Data")
                            primary: false; enabled: !dataService.busy; onClicked: exportDialog.open()
                        }
                        AppButton {
                            theme: page.theme; text: qsTr("Import Data"); primary: false
                            enabled: !dataService.busy; onClicked: importDialog.open()
                        }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: aboutColumn.implicitHeight + 32; radius: theme.radius; color: theme.card; border.color: theme.border
                ColumnLayout {
                    id: aboutColumn; anchors.fill: parent; anchors.margins: 16; spacing: 5
                    Text { text: mobilePlatform || mobilePreview ? "TinyBloom Mobile" : "TinyBloom Desktop"; color: theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    Text {
                        text: mobilePlatform
                            ? qsTr("%1 mobile · Version %2").arg(platformLabel).arg(Qt.application.version)
                            : (mobilePreview
                                ? qsTr("Mobile preview · Version %1").arg(Qt.application.version)
                                : qsTr("Windows desktop · Version %1").arg(Qt.application.version))
                        color: theme.muted
                        font.pixelSize: 12
                    }
                    Text { text: qsTr("Small Steps, Real Progress."); color: theme.primary; font.pixelSize: 13; font.weight: Font.DemiBold; Layout.topMargin: 4 }
                    Text { text: qsTr("Free & Open Source · MIT License"); color: theme.muted; font.pixelSize: 12 }
                }
            }
        }
    }
}
