import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme
    signal createRequested(string date)
    signal editRequested(var id)

    function greeting() {
        let hour = new Date().getHours()
        return hour < 12 ? "Good morning 🌱" : hour < 18 ? "Good afternoon 🌱" : "Good evening 🌱"
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 40; spacing: 22
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 5
                Text { text: page.greeting(); color: theme.text; font.pixelSize: 30; font.weight: Font.Bold }
                Text { text: Qt.formatDate(new Date(), "dddd, MMMM d"); color: theme.muted; font.pixelSize: 14 }
            }
            Item { Layout.fillWidth: true }
            AppButton { theme: page.theme; text: "+ Add Task"; onClicked: page.createRequested(Qt.formatDate(new Date(), "yyyy-MM-dd")) }
        }
        ProgressCard { theme: page.theme; completed: taskManager.todayCompletedCount; total: taskManager.todayCount }
        Text { text: "Today's Tasks"; color: theme.text; font.pixelSize: 19; font.weight: Font.DemiBold }
        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: taskManager.todayCount === 0 ? 0 : 1
            Item {
                EmptyState {
                    anchors.centerIn: parent; theme: page.theme; title: "Your day is clear. 🌱"
                    description: "Create a small task to get started."
                    onActionRequested: page.createRequested(Qt.formatDate(new Date(), "yyyy-MM-dd"))
                }
            }
            ListView {
                id: list; clip: true; spacing: 10; model: taskManager.todayTasks
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

