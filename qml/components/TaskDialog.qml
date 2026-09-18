import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme
    property bool editing: false
    property var taskId: -1
    property var taskData: ({})
    signal deleteRequested(var id)

    Connections {
        target: taskManager
        function onTasksChanged() {
            if (dialog.editing && dialog.opened)
                dialog.taskData = taskManager.getTask(dialog.taskId)
        }
    }

    modal: true
    anchors.centerIn: parent
    width: Math.min(620, parent ? parent.width - 40 : 620)
    height: Math.min(720, parent ? parent.height - 40 : 720)
    padding: 0
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 18; color: theme.card; border.color: theme.border }

    function openNew(defaultDate) {
        editing = false; taskId = -1; taskData = ({})
        titleField.text = ""; descriptionField.text = ""; dueField.text = defaultDate || ""
        priorityBox.currentIndex = 1; durationField.value = settingsManager.defaultTaskDuration
        categoryField.text = ""; moreOptions.checked = false; open(); titleField.forceActiveFocus()
    }
    function openEdit(id) {
        let task = taskManager.getTask(id)
        if (!task || task.id === undefined) return
        editing = true; taskId = id; taskData = task
        titleField.text = task.title; descriptionField.text = task.description; dueField.text = task.dueDate
        priorityBox.currentIndex = task.priority; durationField.value = task.estimatedMinutes
        categoryField.text = task.category; moreOptions.checked = true; open(); titleField.forceActiveFocus()
    }
    function submit() {
        let success = editing
            ? taskManager.updateTask(taskId, titleField.text, descriptionField.text, dueField.text,
                                     priorityBox.currentIndex, durationField.value, categoryField.text)
            : taskManager.createTask(titleField.text, descriptionField.text, dueField.text,
                                     priorityBox.currentIndex, durationField.value, categoryField.text)
        if (success) close()
    }

    contentItem: ColumnLayout {
        anchors.fill: parent; anchors.margins: 26; spacing: 16
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 3
                Text { text: dialog.editing ? qsTr("Edit task") : qsTr("Create a small step"); color: theme.text; font.pixelSize: 23; font.weight: Font.Bold }
                Text { text: dialog.editing ? qsTr("Adjust what matters, and keep moving.") : qsTr("Keep it simple — you can add details later."); color: theme.muted; font.pixelSize: 13 }
            }
            Item { Layout.fillWidth: true }
            RoundButton {
                text: "×"; font.pixelSize: 22; flat: true
                contentItem: Text { text: parent.text; color: theme.muted; font.pixelSize: 22; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                onClicked: dialog.close()
            }
        }
        ScrollView {
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            ColumnLayout {
                width: parent.width; spacing: 12
                Text { text: qsTr("Task name"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                AppTextField {
                    id: titleField; theme: dialog.theme; Layout.fillWidth: true; placeholderText: qsTr("e.g. Read 10 pages")
                    maximumLength: 160; onAccepted: dialog.submit()
                }
                CheckBox {
                    id: moreOptions; text: qsTr("More options"); checked: dialog.editing
                    contentItem: Text { text: parent.text; color: theme.primary; font.pixelSize: 13; font.weight: Font.DemiBold; leftPadding: parent.indicator.width + parent.spacing }
                    indicator: Rectangle {
                        implicitWidth: 20; implicitHeight: 20; radius: 6; border.color: theme.border; color: moreOptions.checked ? theme.primary : "transparent"
                        Text { anchors.centerIn: parent; text: moreOptions.checked ? "−" : "+"; color: moreOptions.checked ? "white" : theme.primary; font.weight: Font.Bold }
                    }
                }
                ColumnLayout {
                    visible: moreOptions.checked; Layout.fillWidth: true; spacing: 10
                    Text { text: qsTr("Description"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                    ScrollView {
                        Layout.fillWidth: true; Layout.preferredHeight: 92
                        TextArea {
                            id: descriptionField; color: theme.text; placeholderText: qsTr("Notes or context (optional)"); placeholderTextColor: theme.muted
                            wrapMode: TextEdit.Wrap; padding: 12
                            background: Rectangle { radius: 10; color: theme.input; border.color: descriptionField.activeFocus ? theme.primary : theme.border }
                        }
                    }
                    GridLayout {
                        Layout.fillWidth: true; columns: 2; columnSpacing: 12; rowSpacing: 8
                        Text { text: qsTr("Due date"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                        Text { text: qsTr("Priority"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                        AppTextField { id: dueField; theme: dialog.theme; Layout.fillWidth: true; placeholderText: qsTr("YYYY-MM-DD"); maximumLength: 10 }
                        ComboBox {
                            id: priorityBox; Layout.fillWidth: true; model: [qsTr("Low"), qsTr("Medium"), qsTr("High")]
                            contentItem: Text { leftPadding: 12; text: priorityBox.displayText; color: theme.text; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { radius: 10; color: theme.input; border.color: theme.border }
                        }
                        Text { text: qsTr("Estimate (minutes)"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                        Text { text: qsTr("Category"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                        SpinBox {
                            id: durationField; Layout.fillWidth: true; from: 0; to: 10080; editable: true
                            contentItem: TextInput { text: durationField.textFromValue(durationField.value, durationField.locale); color: theme.text; horizontalAlignment: Qt.AlignHCenter; verticalAlignment: Qt.AlignVCenter; inputMethodHints: Qt.ImhDigitsOnly }
                            background: Rectangle { radius: 10; color: theme.input; border.color: theme.border }
                        }
                        AppTextField { id: categoryField; theme: dialog.theme; Layout.fillWidth: true; placeholderText: qsTr("Personal"); maximumLength: 80 }
                    }
                    ColumnLayout {
                        visible: dialog.editing; Layout.fillWidth: true; spacing: 8; Layout.topMargin: 4
                        Text { text: qsTr("Small steps"); color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold }
                        Repeater {
                            model: dialog.editing && dialog.taskData.subtasks ? dialog.taskData.subtasks : []
                            delegate: RowLayout {
                                required property var modelData
                                Layout.fillWidth: true
                                CheckBox {
                                    checked: modelData.completed
                                    onClicked: taskManager.toggleSubtask(dialog.taskId, modelData.id)
                                }
                                Text { Layout.fillWidth: true; text: modelData.title; color: modelData.completed ? theme.muted : theme.text; font.strikeout: modelData.completed; font.pixelSize: 13 }
                                ToolButton {
                                    text: "×"; onClicked: taskManager.deleteSubtask(dialog.taskId, modelData.id)
                                    contentItem: Text { text: parent.text; color: theme.muted; font.pixelSize: 18; horizontalAlignment: Text.AlignHCenter }
                                }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            AppTextField { id: subtaskField; theme: dialog.theme; Layout.fillWidth: true; placeholderText: qsTr("Add a small step"); maximumLength: 160; onAccepted: addStepButton.clicked() }
                            AppButton {
                                id: addStepButton; theme: dialog.theme; text: qsTr("Add"); primary: false
                                onClicked: if (taskManager.createSubtask(dialog.taskId, subtaskField.text)) subtaskField.text = ""
                            }
                        }
                    }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            AppButton {
                visible: dialog.editing; theme: dialog.theme; text: qsTr("Delete"); primary: false; danger: true
                onClicked: { dialog.close(); dialog.deleteRequested(dialog.taskId) }
            }
            Item { Layout.fillWidth: true }
            AppButton { theme: dialog.theme; text: qsTr("Cancel"); primary: false; onClicked: dialog.close() }
            AppButton { theme: dialog.theme; text: dialog.editing ? qsTr("Save changes") : qsTr("Create"); onClicked: dialog.submit() }
        }
    }
}
