pragma Translator: "LongTermPage"

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme
    property var selectedTaskId: -1
    property var taskData: ({})

    function createNew() { taskDialog.openNew() }

    function refreshSelection() {
        if (selectedTaskId > 0) {
            const current = taskManager.getTask(selectedTaskId)
            if (current && current.id !== undefined && current.longTerm) {
                taskData = current
                return
            }
        }
        if (taskManager.longTermTasks.count > 0) {
            const first = taskManager.longTermTasks.get(0)
            selectedTaskId = first.taskId
            taskData = taskManager.getTask(selectedTaskId)
        } else {
            selectedTaskId = -1
            taskData = ({})
        }
    }

    function selectTask(id) {
        selectedTaskId = id
        taskData = taskManager.getTask(id)
    }

    function branches() {
        const nodes = taskData && taskData.subtasks ? taskData.subtasks : []
        return nodes.filter(node => Number(node.parentId) === 0)
    }

    function childrenOf(branchId) {
        const nodes = taskData && taskData.subtasks ? taskData.subtasks : []
        return nodes.filter(node => Number(node.parentId) === Number(branchId))
    }

    function leafStats() {
        const nodes = taskData && taskData.subtasks ? taskData.subtasks : []
        const parentIds = ({})
        for (let i = 0; i < nodes.length; ++i)
            if (Number(nodes[i].parentId) > 0) parentIds[nodes[i].parentId] = true
        let total = 0
        let done = 0
        for (let j = 0; j < nodes.length; ++j) {
            if (parentIds[nodes[j].id]) continue
            ++total
            if (nodes[j].completed) ++done
        }
        return { total: total, done: done, progress: total > 0 ? done / total : 0 }
    }

    Connections {
        target: taskManager
        function onTasksChanged() { Qt.callLater(page.refreshSelection) }
    }
    Component.onCompleted: refreshSelection()

    LongTermTaskDialog { id: taskDialog; theme: page.theme }
    LongTermNodeDialog { id: nodeDialog; theme: page.theme }

    Dialog {
        id: deleteDialog
        property bool deletingTask: false
        property var nodeId: -1
        modal: true
        anchors.centerIn: parent
        width: Math.min(410, parent ? parent.width - 24 : 410)
        padding: 20
        closePolicy: Popup.CloseOnEscape
        background: Rectangle { radius: 16; color: theme.card; border.color: theme.border }
        contentItem: ColumnLayout {
            spacing: 14
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: deleteDialog.deletingTask ? qsTr("Delete this long-term task?") : qsTr("Delete this branch?")
                color: theme.text; font.pixelSize: 19; font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted; font.pixelSize: 13
                text: deleteDialog.deletingTask
                    ? qsTr("The goal map and every step inside it will be removed.")
                    : qsTr("Any action steps under this branch will also be removed.")
            }
            RowLayout {
                Layout.fillWidth: true; Layout.topMargin: 6
                Item { Layout.fillWidth: true }
                AppButton { theme: page.theme; text: qsTr("Cancel"); primary: false; onClicked: deleteDialog.close() }
                AppButton {
                    theme: page.theme; text: qsTr("Delete"); primary: false; danger: true
                    onClicked: {
                        if (deleteDialog.deletingTask) taskManager.deleteTask(page.selectedTaskId)
                        else taskManager.deleteSubtask(page.selectedTaskId, deleteDialog.nodeId)
                        deleteDialog.close()
                    }
                }
            }
        }
    }

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: body.implicitHeight + 36
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        ColumnLayout {
            id: body
            x: 14; y: 16
            width: parent.width - 28
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3
                    Text { text: qsTr("Long-term Tasks"); color: theme.text; font.pixelSize: 25; font.weight: Font.Bold }
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("See the whole path, then take the next small step.")
                        color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap
                    }
                }
                AppButton { theme: page.theme; text: qsTr("+ New Big Task"); onClicked: taskDialog.openNew() }
            }

            RowLayout {
                Layout.fillWidth: true
                Text { text: qsTr("My big tasks"); color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold }
                Rectangle {
                    width: 28; height: 24; radius: 8; color: theme.primarySoft
                    Text { anchors.centerIn: parent; text: taskManager.longTermTasks.count; color: theme.primary; font.pixelSize: 12; font.weight: Font.Bold }
                }
                Item { Layout.fillWidth: true }
            }

            ListView {
                id: bigTaskList
                Layout.fillWidth: true
                Layout.preferredHeight: taskManager.longTermTasks.count > 0 ? 108 : 0
                visible: taskManager.longTermTasks.count > 0
                orientation: ListView.Horizontal
                spacing: 10
                clip: true
                model: taskManager.longTermTasks
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
                delegate: Rectangle {
                    id: taskCard
                    required property int taskId
                    required property string title
                    required property bool completed
                    required property int leafCount
                    required property int completedLeafCount
                    width: Math.min(250, bigTaskList.width * 0.78)
                    height: 98
                    radius: 14
                    color: page.selectedTaskId === taskId ? theme.primarySoft : theme.card
                    border.width: page.selectedTaskId === taskId ? 2 : 1
                    border.color: page.selectedTaskId === taskId ? theme.primary : theme.border

                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 7
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                Layout.fillWidth: true; text: taskCard.title; elide: Text.ElideRight
                                color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold
                            }
                            Text { text: taskCard.completed ? "✓" : ""; color: theme.primary; font.pixelSize: 15; font.weight: Font.Bold }
                        }
                        ProgressBar {
                            Layout.fillWidth: true; from: 0; to: Math.max(1, taskCard.leafCount); value: taskCard.completedLeafCount
                            background: Rectangle { implicitHeight: 5; radius: 3; color: theme.border }
                            contentItem: Item {
                                implicitHeight: 5
                                Rectangle { width: parent.width * parent.parent.visualPosition; height: parent.height; radius: 3; color: theme.primary }
                            }
                        }
                        Text {
                            text: taskCard.leafCount > 0
                                ? qsTr("%1 / %2 steps").arg(taskCard.completedLeafCount).arg(taskCard.leafCount)
                                : qsTr("Waiting to be broken down")
                            color: theme.muted; font.pixelSize: 11
                        }
                    }
                    TapHandler { onTapped: page.selectTask(taskCard.taskId) }
                }
            }

            Item {
                visible: page.selectedTaskId < 0
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                EmptyState {
                    anchors.centerIn: parent; theme: page.theme
                    title: qsTr("No long-term tasks yet.")
                    description: qsTr("Create a big task, then turn it into a clear goal map.")
                    actionText: qsTr("Create a big task")
                    onActionRequested: taskDialog.openNew()
                }
            }

            ColumnLayout {
                visible: page.selectedTaskId > 0
                Layout.fillWidth: true
                spacing: 14

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: goalSummary.implicitHeight + 28
                    radius: 16; color: theme.card; border.color: theme.border
                    ColumnLayout {
                        id: goalSummary
                        anchors.fill: parent; anchors.margins: 14; spacing: 10
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                Layout.fillWidth: true
                                text: page.taskData.title || ""
                                color: theme.text; font.pixelSize: 20; font.weight: Font.Bold
                                wrapMode: Text.WordWrap
                            }
                            ToolButton {
                                text: "✎"; implicitWidth: 42; implicitHeight: 42
                                Accessible.name: qsTr("Edit goal")
                                onClicked: taskDialog.openEdit(page.selectedTaskId)
                            }
                            ToolButton {
                                text: "×"; implicitWidth: 42; implicitHeight: 42
                                Accessible.name: qsTr("Delete")
                                onClicked: { deleteDialog.deletingTask = true; deleteDialog.nodeId = -1; deleteDialog.open() }
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            text: page.taskData.description || qsTr("Add a short vision to remind yourself why this matters.")
                            color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap
                        }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 10
                            ProgressBar {
                                Layout.fillWidth: true; from: 0; to: 1; value: page.leafStats().progress
                                background: Rectangle { implicitHeight: 8; radius: 4; color: theme.input }
                                contentItem: Item {
                                    implicitHeight: 8
                                    Rectangle { width: parent.width * parent.parent.visualPosition; height: parent.height; radius: 4; color: theme.primary }
                                }
                            }
                            Text {
                                readonly property var stats: page.leafStats()
                                text: stats.total > 0 ? qsTr("%1% complete").arg(Math.round(stats.progress * 100)) : qsTr("Start by adding a stage")
                                color: theme.primary; font.pixelSize: 11; font.weight: Font.DemiBold
                            }
                        }
                        Text {
                            visible: (page.taskData.dueDate || "").length > 0
                            text: qsTr("Target %1").arg(page.taskData.dueDate || "")
                            color: theme.muted; font.pixelSize: 11
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Goal map"); color: theme.text; font.pixelSize: 16; font.weight: Font.DemiBold }
                    Item { Layout.fillWidth: true }
                    AppButton {
                        theme: page.theme; text: qsTr("+ Add Stage"); primary: false
                        onClicked: nodeDialog.openAddBranch(page.selectedTaskId)
                    }
                }

                Repeater {
                    model: page.branches()
                    delegate: Rectangle {
                        id: stageCard
                        required property var modelData
                        readonly property var childNodes: page.childrenOf(modelData.id)
                        Layout.fillWidth: true
                        implicitHeight: stageContent.implicitHeight + 24
                        radius: 14; color: theme.card; border.color: theme.border

                        ColumnLayout {
                            id: stageContent
                            anchors.fill: parent; anchors.margins: 12; spacing: 9
                            RowLayout {
                                Layout.fillWidth: true
                                CheckBox {
                                    visible: stageCard.childNodes.length === 0
                                    checked: stageCard.modelData.completed
                                    Accessible.name: qsTr("Complete %1").arg(stageCard.modelData.title)
                                    onClicked: taskManager.toggleSubtask(page.selectedTaskId, stageCard.modelData.id)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 2
                                    Text {
                                        Layout.fillWidth: true; text: stageCard.modelData.title
                                        color: stageCard.modelData.completed ? theme.muted : theme.text
                                        font.pixelSize: 15; font.weight: Font.DemiBold; wrapMode: Text.WordWrap
                                        font.strikeout: stageCard.modelData.completed
                                    }
                                    Text {
                                        visible: stageCard.childNodes.length > 0
                                        readonly property int done: stageCard.childNodes.filter(node => node.completed).length
                                        text: qsTr("%1 / %2 actions").arg(done).arg(stageCard.childNodes.length)
                                        color: theme.muted; font.pixelSize: 11
                                    }
                                }
                                ToolButton {
                                    text: "✎"; implicitWidth: 40; implicitHeight: 40
                                    Accessible.name: qsTr("Edit stage")
                                    onClicked: nodeDialog.openEdit(page.selectedTaskId, stageCard.modelData.id, stageCard.modelData.title, true)
                                }
                                ToolButton {
                                    text: "×"; implicitWidth: 40; implicitHeight: 40
                                    Accessible.name: qsTr("Delete stage")
                                    onClicked: { deleteDialog.deletingTask = false; deleteDialog.nodeId = stageCard.modelData.id; deleteDialog.open() }
                                }
                            }

                            Repeater {
                                model: stageCard.childNodes
                                delegate: RowLayout {
                                    required property var modelData
                                    Layout.fillWidth: true; spacing: 6
                                    CheckBox {
                                        checked: modelData.completed
                                        Accessible.name: qsTr("Complete %1").arg(modelData.title)
                                        onClicked: taskManager.toggleSubtask(page.selectedTaskId, modelData.id)
                                    }
                                    Text {
                                        Layout.fillWidth: true; text: modelData.title
                                        color: modelData.completed ? theme.muted : theme.text
                                        font.pixelSize: 13; wrapMode: Text.WordWrap
                                        font.strikeout: modelData.completed
                                    }
                                    ToolButton {
                                        text: "✎"; implicitWidth: 38; implicitHeight: 38
                                        Accessible.name: qsTr("Edit action step")
                                        onClicked: nodeDialog.openEdit(page.selectedTaskId, modelData.id, modelData.title, false)
                                    }
                                    ToolButton {
                                        text: "×"; implicitWidth: 38; implicitHeight: 38
                                        Accessible.name: qsTr("Delete action step")
                                        onClicked: { deleteDialog.deletingTask = false; deleteDialog.nodeId = modelData.id; deleteDialog.open() }
                                    }
                                }
                            }

                            AppButton {
                                theme: page.theme; text: qsTr("Add action step"); primary: false
                                onClicked: nodeDialog.openAddStep(page.selectedTaskId, stageCard.modelData.id)
                            }
                        }
                    }
                }

                Rectangle {
                    visible: page.branches().length === 0
                    Layout.fillWidth: true
                    implicitHeight: emptyStages.implicitHeight + 28
                    radius: 14; color: theme.card; border.color: theme.border
                    ColumnLayout {
                        id: emptyStages
                        anchors.centerIn: parent; width: parent.width - 28; spacing: 8
                        Text { Layout.fillWidth: true; text: qsTr("What are the main stages?"); color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter }
                        Text { Layout.fillWidth: true; text: qsTr("Add 3–5 stages to sketch the route."); color: theme.muted; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap }
                        AppButton { Layout.alignment: Qt.AlignHCenter; theme: page.theme; text: qsTr("+ Add Stage"); onClicked: nodeDialog.openAddBranch(page.selectedTaskId) }
                    }
                }
            }
        }
    }
}
