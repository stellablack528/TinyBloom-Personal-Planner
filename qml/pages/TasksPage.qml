import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme
    signal createRequested(string date)
    signal editRequested(var id)
    function focusSearch() { searchField.forceActiveFocus(); searchField.selectAll() }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 40; spacing: 20
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 4
                Text { text: "Tasks"; color: theme.text; font.pixelSize: 30; font.weight: Font.Bold }
                Text { text: "All your small steps, in one calm place."; color: theme.muted; font.pixelSize: 14 }
            }
            Item { Layout.fillWidth: true }
            AppButton { theme: page.theme; text: "+ Add Task"; onClicked: page.createRequested("") }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 12
            AppTextField {
                id: searchField; theme: page.theme; Layout.fillWidth: true; placeholderText: "Search tasks..."
                onTextChanged: taskManager.searchTasks(text)
            }
            Rectangle {
                implicitWidth: filterRow.implicitWidth + 10; implicitHeight: 44; radius: 11; color: theme.input; border.color: theme.border
                RowLayout {
                    id: filterRow; anchors.centerIn: parent; spacing: 2
                    Repeater {
                        model: ["All", "Active", "Completed"]
                        delegate: Button {
                            required property string modelData; required property int index
                            text: modelData; flat: true; checked: filterGroup.checkedButton === this
                            ButtonGroup.group: filterGroup
                            contentItem: Text { text: parent.text; color: parent.checked ? theme.text : theme.muted; font.pixelSize: 13; font.weight: parent.checked ? Font.DemiBold : Font.Normal; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { radius: 8; color: parent.checked ? theme.card : "transparent" }
                            onClicked: taskManager.filterTasks(index)
                            Component.onCompleted: if (index === 0) checked = true
                        }
                    }
                    ButtonGroup { id: filterGroup; exclusive: true }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true; spacing: 8
            Text { text: "View"; color: theme.muted; font.pixelSize: 12; Layout.rightMargin: 4 }
            Repeater {
                model: ["All", "Today", "Tomorrow", "Later"]
                delegate: Button {
                    required property string modelData; required property int index
                    text: modelData; flat: true; checked: scopeGroup.checkedButton === this
                    ButtonGroup.group: scopeGroup
                    contentItem: Text { text: parent.text; color: parent.checked ? theme.primary : theme.muted; font.pixelSize: 13; font.weight: parent.checked ? Font.DemiBold : Font.Normal; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { radius: 8; color: parent.checked ? theme.primarySoft : "transparent" }
                    onClicked: taskManager.setTaskScope(index)
                    Component.onCompleted: if (index === 0) checked = true
                }
            }
            ButtonGroup { id: scopeGroup; exclusive: true }
            Item { Layout.fillWidth: true }
        }
        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: taskManager.allTasks.count === 0 ? 0 : 1
            Item {
                EmptyState {
                    anchors.centerIn: parent; theme: page.theme; title: searchField.text.length > 0 ? "No matching tasks." : "No tasks yet."
                    description: searchField.text.length > 0 ? "Try a different search." : "Your first small step can start here."
                    onActionRequested: page.createRequested("")
                }
            }
            ListView {
                clip: true; spacing: 10; model: taskManager.allTasks
                ScrollBar.vertical: ScrollBar {}
                delegate: TaskCard {
                    theme: page.theme
                    onToggleRequested: id => taskManager.toggleTask(id)
                    onEditRequested: id => page.editRequested(id)
                }
            }
        }
    }
}
