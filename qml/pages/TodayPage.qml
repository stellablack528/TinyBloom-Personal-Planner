import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property QtObject theme
    signal createRequested(string date)
    signal editRequested(var id)
    signal deleteRequested(var id)
    signal gardenRequested()

    function greeting() {
        let hour = new Date().getHours()
        return hour < 12 ? qsTr("Good morning 🌱") : hour < 18 ? qsTr("Good afternoon 🌱") : qsTr("Good evening 🌱")
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 40; spacing: 22
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 5
                Text { text: page.greeting(); color: theme.text; font.pixelSize: 30; font.weight: Font.Bold }
                Text { text: Qt.locale(settingsManager.language === "zh_CN" ? "zh_CN" : "en_US").toString(new Date(), "dddd, MMMM d"); color: theme.muted; font.pixelSize: 14 }
            }
            Item { Layout.fillWidth: true }
            AppButton { theme: page.theme; text: qsTr("+ Add Task"); onClicked: page.createRequested(Qt.formatDate(new Date(), "yyyy-MM-dd")) }
        }
        GridLayout {
            id: statsGrid
            Layout.fillWidth: true; columns: width < 700 ? 1 : 2; columnSpacing: 14; rowSpacing: 12
            ProgressCard {
                theme: page.theme; completed: taskManager.todayCompletedCount; total: taskManager.todayCount
                Layout.fillWidth: true; Layout.preferredWidth: 430
                Layout.minimumWidth: statsGrid.columns === 1 ? 0 : 400
            }
            GrowthSummaryCard {
                theme: page.theme; Layout.fillWidth: true; Layout.preferredWidth: 290
                Layout.minimumWidth: statsGrid.columns === 1 ? 0 : 260
                onGardenRequested: page.gardenRequested()
            }
        }
        Text { text: qsTr("Today's Tasks"); color: theme.text; font.pixelSize: 19; font.weight: Font.DemiBold }
        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: taskManager.todayCount === 0 ? 0 : 1
            Item {
                EmptyState {
                    anchors.centerIn: parent; theme: page.theme; title: qsTr("Your day is clear. 🌱")
                    description: qsTr("Create a small task to get started.")
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
                    onDeleteRequested: id => page.deleteRequested(id)
                }
            }
        }
    }
}
