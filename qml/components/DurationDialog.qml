import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme
    property bool forFocus: true
    property bool saveFailed: false
    readonly property int minimumMinutes: forFocus ? settingsManager.minimumFocusMinutes : 0
    readonly property int maximumMinutes: forFocus ? settingsManager.maximumFocusMinutes : 10080
    readonly property real enteredMinutes: Number(minutesField.text)
    readonly property bool validMinutes: minutesField.acceptableInput
        && Number.isInteger(enteredMinutes) && enteredMinutes >= minimumMinutes && enteredMinutes <= maximumMinutes

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(360, parent ? parent.width - 32 : 360)
    modal: true
    focus: true
    padding: width < 320 ? 16 : 22
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 20; color: theme.card; border.color: theme.border }
    Overlay.modal: Rectangle { color: Qt.alpha("#13251A", theme.dark ? 0.65 : 0.34) }

    function edit(focus) {
        forFocus = focus
        setMinutes(focus ? settingsManager.focusDurationMinutes : settingsManager.defaultTaskDuration)
        open()
    }

    function setMinutes(minutes) {
        saveFailed = false
        minutesField.text = String(minutes)
    }

    function save() {
        if (!validMinutes) return
        const saved = forFocus ? settingsManager.setFocusDurationMinutes(enteredMinutes)
                               : settingsManager.setDefaultTaskDuration(enteredMinutes)
        if (saved) close()
        else saveFailed = true
    }

    component StepButton: AppButton {
        id: step
        theme: dialog.theme
        primary: false
        implicitWidth: 42
        implicitHeight: 44
        leftPadding: 0
        rightPadding: 0
        opacity: enabled ? 1 : 0.42
        contentItem: Text {
            text: step.text
            color: dialog.theme.text
            font.pixelSize: 22
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    contentItem: ColumnLayout {
        spacing: 14
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                text: dialog.forFocus ? qsTr("Focus duration") : qsTr("Default task estimate")
                color: theme.text; font.pixelSize: 22; font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                text: dialog.forFocus
                    ? (focusManager.sessionInProgress ? qsTr("Applies to the next session. This session continues as planned.")
                                                      : qsTr("Saved as your default for future focus sessions."))
                    : qsTr("Applies to new tasks. Enter 0 to leave the estimate empty.")
                color: theme.muted; font.pixelSize: 13
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: durationEditor.implicitHeight + 24
            radius: 16
            color: theme.input
            border.color: theme.border

            ColumnLayout {
                id: durationEditor
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: dialog.width < 320 ? 14 : 20
                    StepButton {
                        Layout.alignment: Qt.AlignVCenter
                        text: "−"
                        Accessible.name: qsTr("Decrease by one minute")
                        enabled: dialog.validMinutes && dialog.enteredMinutes > dialog.minimumMinutes
                        onClicked: dialog.setMinutes(dialog.enteredMinutes - 1)
                    }
                    Item {
                        Layout.preferredWidth: 106
                        Layout.preferredHeight: 76
                        AppTextField {
                            id: minutesField
                            theme: dialog.theme
                            anchors.fill: parent
                            bottomPadding: 23
                            leftPadding: 2; rightPadding: 2
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 32; font.weight: Font.Bold
                            maximumLength: dialog.forFocus ? 3 : 5
                            selectByMouse: true
                            Accessible.name: qsTr("Duration in minutes")
                            inputMethodHints: Qt.ImhDigitsOnly
                            validator: IntValidator { bottom: dialog.minimumMinutes; top: dialog.maximumMinutes }
                            background: Rectangle {
                                radius: 10
                                color: theme.card
                                border.width: minutesField.activeFocus ? 2 : 1
                                border.color: minutesField.activeFocus ? theme.primary : theme.border
                            }
                            onTextEdited: dialog.saveFailed = false
                            onAccepted: dialog.save()
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 10
                            text: qsTr("min")
                            color: theme.muted
                            font.pixelSize: 13
                        }
                    }
                    StepButton {
                        Layout.alignment: Qt.AlignVCenter
                        text: "+"
                        Accessible.name: qsTr("Increase by one minute")
                        enabled: dialog.validMinutes && dialog.enteredMinutes < dialog.maximumMinutes
                        onClicked: dialog.setMinutes(dialog.enteredMinutes + 1)
                    }
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("%1–%2 minutes").arg(dialog.minimumMinutes).arg(dialog.maximumMinutes)
                    color: dialog.validMinutes ? theme.muted : theme.danger
                    font.pixelSize: 12
                }
            }
        }

        RowLayout {
            visible: dialog.forFocus
            Layout.fillWidth: true
            spacing: 6
            Repeater {
                model: [5, 15, 25, 45]
                AppButton {
                    id: preset
                    required property int modelData
                    readonly property bool selected: dialog.validMinutes && dialog.enteredMinutes === modelData
                    theme: dialog.theme
                    primary: false
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    implicitHeight: 42
                    leftPadding: 4; rightPadding: 4
                    text: qsTr("%1 min").arg(modelData)
                    Accessible.checkable: true
                    Accessible.checked: selected
                    contentItem: Text {
                        text: preset.text
                        color: preset.selected ? theme.text : theme.muted
                        font.pixelSize: 13
                        font.weight: preset.selected ? Font.DemiBold : Font.Normal
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 10
                        color: preset.selected ? theme.primarySoft : preset.hovered ? theme.cardHover : theme.input
                        border.width: preset.activeFocus ? 2 : 0
                        border.color: theme.primary
                    }
                    onClicked: dialog.setMinutes(modelData)
                }
            }
        }
        Text {
            visible: dialog.saveFailed; Layout.fillWidth: true; wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Couldn't save the duration. Please try again.")
            color: theme.danger; font.pixelSize: 12
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: theme.border
        }

        RowLayout {
            Layout.fillWidth: true; spacing: 10
            AppButton {
                theme: dialog.theme; text: qsTr("Cancel"); primary: false
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                onClicked: dialog.close()
            }
            AppButton {
                id: saveButton
                theme: dialog.theme; text: qsTr("Save")
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                contentItem: Text {
                    text: saveButton.text
                    color: theme.dark ? theme.window : "white"
                    font.pixelSize: 14; font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                enabled: dialog.validMinutes; opacity: enabled ? 1 : 0.5
                onClicked: dialog.save()
            }
        }
    }
}
