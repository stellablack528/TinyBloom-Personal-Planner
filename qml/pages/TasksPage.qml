import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme
    property int selectedView: 0
    readonly property bool compact: width < 600
    signal createRequested(string date)
    signal editRequested(var id)
    signal deleteRequested(var id)
    function focusSearch() { searchField.forceActiveFocus(); searchField.selectAll() }
    function applyView(index) {
        selectedView = index
        taskManager.filterTasks(index === 1 ? 1 : index === 5 ? 2 : 0)
        taskManager.setTaskScope(index === 2 ? 1 : index === 3 ? 2 : index === 4 ? 3 : 0)
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: page.compact ? 16 : 40; spacing: page.compact ? 14 : 20
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 4
                Text { text: qsTr("Tasks"); color: theme.text; font.pixelSize: page.compact ? 25 : 30; font.weight: Font.Bold }
                Text { text: qsTr("All your small steps, in one calm place."); color: theme.muted; font.pixelSize: page.compact ? 12 : 14 }
            }
            Item { Layout.fillWidth: true }
            AppButton { theme: page.theme; text: qsTr("+ Add Task"); onClicked: page.createRequested("") }
        }
        AppTextField {
            id: searchField; theme: page.theme; Layout.fillWidth: true; placeholderText: qsTr("Search tasks...")
            Accessible.name: qsTr("Search tasks")
            onTextChanged: taskManager.searchTasks(text)
        }
        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: 42
            contentWidth: viewChrome.width
            contentHeight: height
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            Rectangle {
                id: viewChrome
                width: viewRow.implicitWidth + 8; height: 42; radius: 11
                color: theme.input; border.color: theme.border
                RowLayout {
                    id: viewRow; anchors.centerIn: parent; spacing: 2
                    Repeater {
                        model: [qsTr("All"), qsTr("Active"), qsTr("Today"), qsTr("Tomorrow"), qsTr("Later"), qsTr("Completed")]
                        delegate: Button {
                            required property string modelData; required property int index
                            text: modelData; flat: true; checked: page.selectedView === index
                            implicitWidth: Math.max(58, viewLabel.implicitWidth + 20); implicitHeight: 34
                            Accessible.name: qsTr("Show %1 tasks").arg(modelData)
                            contentItem: Text {
                                id: viewLabel; text: parent.text
                                color: parent.checked ? theme.primary : theme.muted
                                font.pixelSize: 13; font.weight: parent.checked ? Font.DemiBold : Font.Normal
                                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                radius: 8; color: parent.checked ? theme.primarySoft : "transparent"
                                border.width: parent.activeFocus && !parent.checked ? 2 : 0; border.color: theme.primary
                            }
                            onClicked: page.applyView(index)
                        }
                    }
                }
            }
        }
        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: taskManager.allTasks.count === 0 ? 0 : 1
            Item {
                EmptyState {
                    anchors.centerIn: parent; theme: page.theme; title: searchField.text.length > 0 ? qsTr("No matching tasks.") : qsTr("No tasks yet.")
                    description: searchField.text.length > 0 ? qsTr("Try a different search.") : qsTr("Your first small step can start here.")
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
                    onDeleteRequested: id => page.deleteRequested(id)
                }
            }
        }
    }
}
