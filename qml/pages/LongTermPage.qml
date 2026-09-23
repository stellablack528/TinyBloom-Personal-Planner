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
        modal: true; anchors.centerIn: parent; width: 410; padding: 22
        closePolicy: Popup.CloseOnEscape
        background: Rectangle { radius: 16; color: theme.card; border.color: theme.border }
        contentItem: ColumnLayout {
            spacing: 14
            Text {
                text: deleteDialog.deletingTask ? qsTr("Delete this long-term task?") : qsTr("Delete this branch?")
                color: theme.text; font.pixelSize: 20; font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap; color: theme.muted; font.pixelSize: 13
                text: deleteDialog.deletingTask
                    ? qsTr("The goal map and every step inside it will be removed.")
                    : qsTr("Any action steps under this branch will also be removed.")
            }
            RowLayout {
                Layout.fillWidth: true; Layout.topMargin: 8
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

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 32; spacing: 18
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 3
                Text { text: qsTr("Long-term Tasks"); color: theme.text; font.pixelSize: 30; font.weight: Font.Bold }
                Text { text: qsTr("See the whole path, then take the next small step."); color: theme.muted; font.pixelSize: 14 }
            }
            Item { Layout.fillWidth: true }
            AppButton { theme: page.theme; text: qsTr("+ New Big Task"); onClicked: taskDialog.openNew() }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 16

            Rectangle {
                Layout.preferredWidth: 248; Layout.fillHeight: true
                radius: 16; color: theme.card; border.color: theme.border
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 14; spacing: 10
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTr("My big tasks"); color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 28; height: 24; radius: 8; color: theme.primarySoft
                            Text { anchors.centerIn: parent; text: taskManager.longTermTasks.count; color: theme.primary; font.pixelSize: 12; font.weight: Font.Bold }
                        }
                    }
                    ListView {
                        id: taskList
                        Layout.fillWidth: true; Layout.fillHeight: true
                        model: taskManager.longTermTasks; spacing: 8; clip: true
                        ScrollBar.vertical: ScrollBar {}
                        delegate: Rectangle {
                            id: taskItem
                            required property int taskId
                            required property string title
                            required property bool completed
                            required property int leafCount
                            required property int completedLeafCount
                            width: taskList.width; height: 92; radius: 12
                            color: page.selectedTaskId === taskId ? theme.primarySoft : taskMouse.containsMouse ? theme.cardHover : theme.input
                            border.width: page.selectedTaskId === taskId ? 1 : 0
                            border.color: theme.primary
                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 7
                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        Layout.fillWidth: true; text: taskItem.title; elide: Text.ElideRight
                                        color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold
                                    }
                                    Text { text: taskItem.completed ? "✓" : ""; color: theme.primary; font.pixelSize: 15; font.weight: Font.Bold }
                                }
                                ProgressBar {
                                    Layout.fillWidth: true; from: 0; to: Math.max(1, taskItem.leafCount); value: taskItem.completedLeafCount
                                    background: Rectangle { implicitHeight: 5; radius: 3; color: theme.border }
                                    contentItem: Item {
                                        implicitHeight: 5
                                        Rectangle { width: parent.width * parent.parent.visualPosition; height: parent.height; radius: 3; color: theme.primary }
                                    }
                                }
                                Text {
                                    text: taskItem.leafCount > 0
                                        ? qsTr("%1 / %2 steps").arg(taskItem.completedLeafCount).arg(taskItem.leafCount)
                                        : qsTr("Waiting to be broken down")
                                    color: theme.muted; font.pixelSize: 11
                                }
                            }
                            MouseArea {
                                id: taskMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onClicked: page.selectTask(taskItem.taskId)
                                onDoubleClicked: taskDialog.openEdit(taskItem.taskId)
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                radius: 16; color: theme.card; border.color: theme.border

                EmptyState {
                    visible: page.selectedTaskId < 0
                    anchors.centerIn: parent; theme: page.theme
                    title: qsTr("No long-term tasks yet.")
                    description: qsTr("Create a big task, then turn it into a clear goal map.")
                    actionText: qsTr("Create a big task")
                    onActionRequested: taskDialog.openNew()
                }

                ColumnLayout {
                    visible: page.selectedTaskId > 0
                    anchors.fill: parent; anchors.margins: 20; spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 3
                            Text {
                                Layout.fillWidth: true; text: page.taskData.title || ""; color: theme.text
                                font.pixelSize: 22; font.weight: Font.Bold; elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: page.taskData.description || qsTr("Add a short vision to remind yourself why this matters.")
                                color: theme.muted; font.pixelSize: 12; wrapMode: Text.WordWrap; maximumLineCount: 2; elide: Text.ElideRight
                            }
                        }
                        AppButton { theme: page.theme; text: qsTr("Edit goal"); primary: false; onClicked: taskDialog.openEdit(page.selectedTaskId) }
                        AppButton {
                            theme: page.theme; text: qsTr("Delete"); primary: false; danger: true
                            onClicked: { deleteDialog.deletingTask = true; deleteDialog.nodeId = -1; deleteDialog.open() }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true; spacing: 12
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
                            color: theme.primary; font.pixelSize: 12; font.weight: Font.DemiBold
                        }
                        Text {
                            visible: (page.taskData.dueDate || "").length > 0
                            text: qsTr("Target %1").arg(page.taskData.dueDate || "")
                            color: theme.muted; font.pixelSize: 12
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        radius: 14; color: theme.input; border.color: theme.border
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 14; spacing: 10
                            RowLayout {
                                Layout.fillWidth: true
                                Text { text: qsTr("Goal map"); color: theme.text; font.pixelSize: 14; font.weight: Font.DemiBold }
                                Text { text: qsTr("Click names to edit them"); color: theme.muted; font.pixelSize: 11 }
                                Item { Layout.fillWidth: true }
                                AppButton { theme: page.theme; text: qsTr("+ Add Stage"); primary: false; onClicked: nodeDialog.openAddBranch(page.selectedTaskId) }
                            }
                            ScrollView {
                                id: mapScroll
                                Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                                ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                                Item {
                                    width: Math.max(mapScroll.availableWidth, mapRow.implicitWidth + 24)
                                    height: Math.max(mapScroll.availableHeight, mapRow.implicitHeight + 24)
                                    RowLayout {
                                        id: mapRow
                                        anchors.centerIn: parent; spacing: 0
                                        Rectangle {
                                            Layout.preferredWidth: 176; Layout.preferredHeight: 112
                                            radius: 16; color: theme.primary; border.color: theme.primaryHover
                                            ColumnLayout {
                                                anchors.fill: parent; anchors.margins: 14; spacing: 5
                                                Text { text: qsTr("BIG TASK"); color: "#DDF3E5"; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 1 }
                                                Text {
                                                    Layout.fillWidth: true; Layout.fillHeight: true
                                                    text: page.taskData.title || ""; color: "white"; font.pixelSize: 16; font.weight: Font.Bold
                                                    wrapMode: Text.WordWrap; verticalAlignment: Text.AlignVCenter
                                                }
                                                Text {
                                                    readonly property var stats: page.leafStats()
                                                    text: stats.total > 0 ? qsTr("%1 of %2 done").arg(stats.done).arg(stats.total) : qsTr("Build your path →")
                                                    color: "#EAF7EF"; font.pixelSize: 11
                                                }
                                            }
                                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: taskDialog.openEdit(page.selectedTaskId) }
                                        }
                                        Rectangle { Layout.preferredWidth: 34; Layout.preferredHeight: 3; color: theme.primary }
                                        ColumnLayout {
                                            spacing: 12
                                            Repeater {
                                                model: page.branches()
                                                delegate: RowLayout {
                                                    id: branchRow
                                                    required property var modelData
                                                    spacing: 0
                                                    readonly property var childNodes: page.childrenOf(modelData.id)
                                                    Rectangle {
                                                        Layout.preferredWidth: 172; Layout.preferredHeight: Math.max(72, branchContent.implicitHeight + 20)
                                                        radius: 12; color: theme.card; border.color: theme.primary
                                                        RowLayout {
                                                            id: branchContent; anchors.fill: parent; anchors.margins: 9; spacing: 5
                                                            CheckBox {
                                                                visible: branchRow.childNodes.length === 0
                                                                checked: branchRow.modelData.completed
                                                                Accessible.name: qsTr("Complete %1").arg(branchRow.modelData.title)
                                                                onClicked: taskManager.toggleSubtask(page.selectedTaskId, branchRow.modelData.id)
                                                            }
                                                            ColumnLayout {
                                                                Layout.fillWidth: true; spacing: 2
                                                                Text {
                                                                    Layout.fillWidth: true; text: branchRow.modelData.title
                                                                    color: branchRow.modelData.completed && branchRow.childNodes.length === 0 ? theme.muted : theme.text
                                                                    font.pixelSize: 13; font.weight: Font.DemiBold; wrapMode: Text.WordWrap
                                                                    font.strikeout: branchRow.modelData.completed && branchRow.childNodes.length === 0
                                                                }
                                                                Text {
                                                                    visible: branchRow.childNodes.length > 0
                                                                    readonly property int done: branchRow.childNodes.filter(node => node.completed).length
                                                                    text: qsTr("%1 / %2 actions").arg(done).arg(branchRow.childNodes.length)
                                                                    color: theme.muted; font.pixelSize: 10
                                                                }
                                                            }
                                                            ToolButton {
                                                                text: "✎"; implicitWidth: 28
                                                                Accessible.name: qsTr("Edit stage")
                                                                onClicked: nodeDialog.openEdit(page.selectedTaskId, branchRow.modelData.id, branchRow.modelData.title, true)
                                                            }
                                                            ToolButton {
                                                                text: "×"; implicitWidth: 28
                                                                Accessible.name: qsTr("Delete stage")
                                                                onClicked: { deleteDialog.deletingTask = false; deleteDialog.nodeId = branchRow.modelData.id; deleteDialog.open() }
                                                            }
                                                        }
                                                    }
                                                    Rectangle {
                                                        visible: branchRow.childNodes.length > 0
                                                        Layout.preferredWidth: 26; Layout.preferredHeight: 2; color: theme.border
                                                    }
                                                    ColumnLayout {
                                                        visible: branchRow.childNodes.length > 0
                                                        spacing: 6
                                                        Repeater {
                                                            model: branchRow.childNodes
                                                            delegate: Rectangle {
                                                                id: childCard
                                                                required property var modelData
                                                                Layout.preferredWidth: 218; Layout.preferredHeight: 48
                                                                radius: 10; color: theme.card; border.color: modelData.completed ? theme.primary : theme.border
                                                                RowLayout {
                                                                    anchors.fill: parent; anchors.margins: 6; spacing: 5
                                                                    CheckBox {
                                                                        checked: childCard.modelData.completed
                                                                        Accessible.name: qsTr("Complete %1").arg(childCard.modelData.title)
                                                                        onClicked: taskManager.toggleSubtask(page.selectedTaskId, childCard.modelData.id)
                                                                    }
                                                                    Text {
                                                                        Layout.fillWidth: true; text: childCard.modelData.title
                                                                        color: childCard.modelData.completed ? theme.muted : theme.text
                                                                        font.pixelSize: 12; elide: Text.ElideRight; font.strikeout: childCard.modelData.completed
                                                                    }
                                                                    ToolButton {
                                                                        text: "✎"; implicitWidth: 26
                                                                        Accessible.name: qsTr("Edit action step")
                                                                        onClicked: nodeDialog.openEdit(page.selectedTaskId, childCard.modelData.id, childCard.modelData.title, false)
                                                                    }
                                                                    ToolButton {
                                                                        text: "×"; implicitWidth: 26
                                                                        Accessible.name: qsTr("Delete action step")
                                                                        onClicked: { deleteDialog.deletingTask = false; deleteDialog.nodeId = childCard.modelData.id; deleteDialog.open() }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                    ToolButton {
                                                        text: "+"; implicitWidth: 34; implicitHeight: 34
                                                        Accessible.name: qsTr("Add action step")
                                                        ToolTip.visible: hovered; ToolTip.text: qsTr("Add action step")
                                                        onClicked: nodeDialog.openAddStep(page.selectedTaskId, branchRow.modelData.id)
                                                        background: Rectangle { radius: 10; color: parent.hovered ? theme.primarySoft : theme.card; border.color: theme.border }
                                                    }
                                                }
                                            }
                                            Rectangle {
                                                visible: page.branches().length === 0
                                                Layout.preferredWidth: 270; Layout.preferredHeight: 90
                                                radius: 12; color: theme.card; border.color: theme.border; border.width: 1
                                                ColumnLayout {
                                                    anchors.centerIn: parent; spacing: 6
                                                    Text { text: qsTr("What are the main stages?"); color: theme.text; font.pixelSize: 13; font.weight: Font.DemiBold }
                                                    Text { text: qsTr("Add 3–5 stages to sketch the route."); color: theme.muted; font.pixelSize: 11 }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
