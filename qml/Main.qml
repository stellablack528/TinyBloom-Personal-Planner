import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TinyBloom

ApplicationWindow {
    id: root
    width: 1120; height: 760; minimumWidth: 780; minimumHeight: 580
    visible: true; title: qsTr("TinyBloom — Small Steps, Real Progress.")
    color: theme.window
    property int currentPage: 0

    Theme { id: theme }

    Shortcut { sequence: "Ctrl+N"; onActivated: taskDialog.openNew(currentPage === 0 ? Qt.formatDate(new Date(), "yyyy-MM-dd") : "") }
    Shortcut { sequence: "Ctrl+F"; onActivated: { currentPage = 1; tasksPage.focusSearch() } }

    RowLayout {
        anchors.fill: parent; spacing: 0
        Sidebar {
            theme: theme; currentIndex: root.currentPage; Layout.fillHeight: true; Layout.preferredWidth: 230
            onPageRequested: index => root.currentPage = index
        }
        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: root.currentPage
            TodayPage {
                theme: theme
                onCreateRequested: date => taskDialog.openNew(date)
                onEditRequested: id => taskDialog.openEdit(id)
                onGardenRequested: root.currentPage = 2
            }
            TasksPage {
                id: tasksPage; theme: theme
                onCreateRequested: date => taskDialog.openNew(date)
                onEditRequested: id => taskDialog.openEdit(id)
            }
            GardenPage { theme: theme }
            SettingsPage { theme: theme }
        }
    }

    TaskDialog {
        id: taskDialog; theme: theme
        onDeleteRequested: id => { confirmDialog.targetId = id; confirmDialog.open() }
    }
    ConfirmDialog {
        id: confirmDialog; theme: theme
        onConfirmed: id => taskManager.deleteTask(id)
    }
    XpToast { id: xpToast; theme: theme }
    Popup {
        id: toast; property string message
        x: root.width - width - 24; y: root.height - height - 24
        width: Math.min(460, root.width - 48)
        height: Math.max(52, toastText.implicitHeight + 24); padding: 0
        closePolicy: Popup.NoAutoClose
        background: Rectangle { radius: 12; color: theme.text }
        contentItem: Text {
            id: toastText; width: toast.width - 32; text: toast.message; color: theme.window; font.pixelSize: 13
            wrapMode: Text.Wrap; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        }
        Timer { id: toastTimer; interval: 4800; onTriggered: toast.close() }
        function show(text) { message = text; open(); toastTimer.restart() }
    }
    Connections { target: taskManager; function onErrorOccurred(message) { toast.show(message) } }
    Connections { target: settingsManager; function onErrorOccurred(message) { toast.show(message) } }
    Connections {
        target: growthManager
        function onExperienceAwarded(amount, reason) { xpToast.showReward(amount, reason) }
        function onErrorOccurred(message) { toast.show(message) }
    }
    Connections {
        target: dataService
        function onOperationSucceeded(message) { toast.show(message) }
        function onOperationFailed(message) { toast.show(message) }
    }
    Component.onCompleted: {
        if (!databaseReady) toast.show(qsTr("Local storage could not be opened. Changes may not be saved."))
        if (screenshotScenario === "tasks") currentPage = 1
        else if (screenshotScenario === "garden") currentPage = 2
        else if (screenshotScenario === "settings-midnight") {
            settingsManager.theme = "midnight"
            currentPage = 3
        } else if (screenshotScenario === "task-dialog") {
            Qt.callLater(() => taskDialog.openNew(Qt.formatDate(new Date(), "yyyy-MM-dd")))
        } else if (screenshotScenario === "task-dialog-advanced") {
            Qt.callLater(() => taskDialog.openScreenshotPreview(false))
        } else if (screenshotScenario === "date-picker") {
            Qt.callLater(() => taskDialog.openScreenshotPreview(true))
        }
    }
}
