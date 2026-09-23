import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    property QtObject theme
    property var taskId: -1
    property var parentId: 0
    property var nodeId: -1
    property bool editing: false
    property bool branch: true

    modal: true
    anchors.centerIn: parent
    width: Math.min(430, parent ? parent.width - (mobilePlatform || mobilePreview ? 16 : 40) : 430)
    height: 245
    padding: 0
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: 16; color: theme.card; border.color: theme.border }

    function openAddBranch(ownerId) {
        taskId = ownerId; parentId = 0; nodeId = -1; editing = false; branch = true
        nameField.text = ""; open(); nameField.forceActiveFocus()
    }
    function openAddStep(ownerId, ownerBranchId) {
        taskId = ownerId; parentId = ownerBranchId; nodeId = -1; editing = false; branch = false
        nameField.text = ""; open(); nameField.forceActiveFocus()
    }
    function openEdit(ownerId, id, title, isBranch) {
        taskId = ownerId; nodeId = id; editing = true; branch = isBranch
        nameField.text = title; open(); nameField.forceActiveFocus(); nameField.selectAll()
    }
    function submit() {
        const success = editing
            ? taskManager.updateSubtask(taskId, nodeId, nameField.text)
            : taskManager.createSubtask(taskId, nameField.text, parentId)
        if (success) close()
    }

    contentItem: ColumnLayout {
        anchors.fill: parent; anchors.margins: dialog.width < 400 ? 16 : 24; spacing: 13
        Text {
            text: dialog.editing
                ? (dialog.branch ? qsTr("Edit stage") : qsTr("Edit action step"))
                : (dialog.branch ? qsTr("Add a stage") : qsTr("Add an action step"))
            color: theme.text; font.pixelSize: 21; font.weight: Font.Bold
        }
        Text {
            text: dialog.branch
                ? qsTr("Stages are the main branches of your goal map.")
                : qsTr("Make it small enough to finish in one or two sessions.")
            color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true
        }
        AppTextField {
            id: nameField; theme: dialog.theme; Layout.fillWidth: true
            placeholderText: dialog.branch ? qsTr("e.g. Research and planning") : qsTr("e.g. List three reference projects")
            maximumLength: 160; onAccepted: if (text.trim().length > 0) dialog.submit()
        }
        Item { Layout.fillHeight: true }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { theme: dialog.theme; text: qsTr("Cancel"); primary: false; onClicked: dialog.close() }
            AppButton {
                theme: dialog.theme; text: dialog.editing ? qsTr("Save") : qsTr("Add")
                enabled: nameField.text.trim().length > 0; opacity: enabled ? 1 : 0.48
                onClicked: dialog.submit()
            }
        }
    }
}
