import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme
    property bool editing: false
    property var taskId: -1
    readonly property bool targetDateValid: {
        const value = targetDateField.text.trim()
        if (value.length === 0) return true
        if (!/^\d{4}-\d{2}-\d{2}$/.test(value)) return false
        const parsed = new Date(value + "T00:00:00")
        return !isNaN(parsed.getTime()) && Qt.formatDate(parsed, "yyyy-MM-dd") === value
    }

    modal: true
    anchors.centerIn: parent
    width: Math.min(540, parent ? parent.width - 40 : 540)
    height: Math.min(520, parent ? parent.height - 40 : 520)
    padding: 0
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 18; color: theme.card; border.color: theme.border }

    function openNew() {
        editing = false
        taskId = -1
        titleField.text = ""
        visionField.text = ""
        targetDateField.text = ""
        open()
        titleField.forceActiveFocus()
    }

    function openEdit(id) {
        const task = taskManager.getTask(id)
        if (!task || task.id === undefined) return
        editing = true
        taskId = id
        titleField.text = task.title
        visionField.text = task.description
        targetDateField.text = task.dueDate
        open()
        titleField.forceActiveFocus()
    }

    function submit() {
        const success = editing
            ? taskManager.updateLongTermTask(taskId, titleField.text, visionField.text, targetDateField.text)
            : taskManager.createLongTermTask(titleField.text, visionField.text, targetDateField.text)
        if (success) close()
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 26
        spacing: 15

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 3
                Text {
                    text: dialog.editing ? qsTr("Edit long-term task") : qsTr("Create a long-term task")
                    color: theme.text; font.pixelSize: 23; font.weight: Font.Bold
                }
                Text {
                    text: qsTr("Turn a distant goal into a map you can act on.")
                    color: theme.muted; font.pixelSize: 13
                }
            }
            Item { Layout.fillWidth: true }
            RoundButton {
                text: "×"; flat: true; font.pixelSize: 22
                Accessible.name: qsTr("Close")
                onClicked: dialog.close()
            }
        }

        Text { text: qsTr("Goal name"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
        AppTextField {
            id: titleField; theme: dialog.theme; Layout.fillWidth: true
            placeholderText: qsTr("e.g. Finish my graduation project")
            maximumLength: 160; Accessible.name: qsTr("Goal name")
        }
        Text { text: qsTr("Why this matters"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
        ScrollView {
            Layout.fillWidth: true; Layout.preferredHeight: 110
            TextArea {
                id: visionField; padding: 12; wrapMode: TextEdit.Wrap
                color: theme.text; placeholderTextColor: theme.muted
                placeholderText: qsTr("Write down the result you want and the reason behind it.")
                background: Rectangle { radius: 10; color: theme.input; border.color: visionField.activeFocus ? theme.primary : theme.border }
            }
        }
        Text { text: qsTr("Target date (optional)"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
        AppTextField {
            id: targetDateField; theme: dialog.theme; Layout.fillWidth: true
            placeholderText: qsTr("YYYY-MM-DD"); maximumLength: 10
            Accessible.name: qsTr("Target date")
        }
        Text {
            visible: !dialog.targetDateValid
            text: qsTr("Use a valid date in YYYY-MM-DD format.")
            color: theme.danger; font.pixelSize: 11
        }
        Item { Layout.fillHeight: true }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { theme: dialog.theme; text: qsTr("Cancel"); primary: false; onClicked: dialog.close() }
            AppButton {
                theme: dialog.theme
                text: dialog.editing ? qsTr("Save changes") : qsTr("Create map")
                enabled: titleField.text.trim().length > 0 && dialog.targetDateValid
                opacity: enabled ? 1 : 0.48
                onClicked: dialog.submit()
            }
        }
    }
}
