import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TinyBloom

ApplicationWindow {
    id: root
    property bool compactMode: mobilePlatform || mobilePreview || width < 720
    width: mobilePlatform || mobilePreview ? 390 : 1260
    height: mobilePlatform || mobilePreview ? 844 : 760
    minimumWidth: mobilePlatform || mobilePreview ? 320 : 900
    minimumHeight: mobilePlatform || mobilePreview ? 560 : 580
    visible: true
    title: (mobilePlatform || mobilePreview ? "TinyBloom Mobile — " : "TinyBloom Desktop — ")
        + qsTr("Small Steps, Real Progress.")
    color: theme.window
    property int currentPage: 0
    readonly property QtObject appTheme: theme

    Theme { id: theme }

    Shortcut {
        enabled: !root.compactMode
        sequence: "Ctrl+N"
        onActivated: currentPage === 2 && longTermLoader.item ? longTermLoader.item.createNew()
            : taskDialog.openNew(currentPage === 0 ? Qt.formatDate(new Date(), "yyyy-MM-dd") : "")
    }
    Shortcut { enabled: !root.compactMode; sequence: "Ctrl+F"; onActivated: { currentPage = 1; tasksPage.focusSearch() } }

    Item {
        anchors.fill: parent
        Sidebar {
            id: sidebar
            visible: !root.compactMode
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 230
            theme: theme; currentIndex: root.currentPage
            onPageRequested: index => root.currentPage = index
        }
        StackLayout {
            anchors.left: root.compactMode ? parent.left : sidebar.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: root.compactMode ? mobileNavigation.top : parent.bottom
            currentIndex: root.currentPage
            TodayPage {
                theme: theme
                onCreateRequested: date => taskDialog.openNew(date)
                onEditRequested: id => taskDialog.openEdit(id)
                onDeleteRequested: id => { confirmDialog.targetId = id; confirmDialog.open() }
                onGardenRequested: root.currentPage = 3
            }
            TasksPage {
                id: tasksPage; theme: theme
                onCreateRequested: date => taskDialog.openNew(date)
                onEditRequested: id => taskDialog.openEdit(id)
                onDeleteRequested: id => { confirmDialog.targetId = id; confirmDialog.open() }
            }
            Loader {
                id: longTermLoader
                sourceComponent: root.compactMode ? mobileLongTermComponent : desktopLongTermComponent
            }
            GardenPage {
                id: gardenPage; theme: theme
                onSeedPlanted: flowerName => toast.show(qsTr("%1 seed planted. Let your next small step help it grow.").arg(flowerName))
                onFocusSessionCompleted: toast.show(qsTr("Focus session complete. Take a breath, then mark the progress you made."))
            }
            SettingsPage { theme: theme }
        }
        MobileNavigationBar {
            id: mobileNavigation
            visible: root.compactMode
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 70
            theme: theme
            currentIndex: root.currentPage
            onPageRequested: index => root.currentPage = index
        }
    }

    Component { id: desktopLongTermComponent; LongTermPage { theme: root.appTheme } }
    Component { id: mobileLongTermComponent; MobileLongTermPage { theme: root.appTheme } }

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
        x: root.width - width - (root.compactMode ? 12 : 24)
        y: root.height - height - (root.compactMode ? mobileNavigation.height + 12 : 24)
        width: Math.min(460, root.width - (root.compactMode ? 24 : 48))
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
        else if (screenshotScenario === "tasks-demo") {
            if (taskManager.totalCount === 0) {
                taskManager.createTask("Review tomorrow's lesson", "", Qt.formatDate(new Date(Date.now() + 86400000), "yyyy-MM-dd"), 2, 30, "Study")
                taskManager.createTask("Prepare the project notes", "", Qt.formatDate(new Date(Date.now() + 604800000), "yyyy-MM-dd"), 1, 25, "Project")
                taskManager.createTask("Read ten pages", "", "", 1, 20, "Reading")
            }
            currentPage = 1
        }
        else if (screenshotScenario === "long-term-demo") {
            if (taskManager.longTermTasks.count === 0) {
                taskManager.createLongTermTask("完成个人作品集", "把最好的作品整理成一个让我愿意自豪分享的故事。", Qt.formatDate(new Date(Date.now() + 7776000000), "yyyy-MM-dd"))
                const goalId = taskManager.longTermTasks.get(0).taskId
                taskManager.createSubtask(goalId, "规划内容故事", 0)
                let goal = taskManager.getTask(goalId)
                const planningId = goal.subtasks[0].id
                taskManager.createSubtask(goalId, "选出三个代表项目", planningId)
                taskManager.createSubtask(goalId, "写好个人介绍", planningId)
                taskManager.createSubtask(goalId, "制作与打磨", 0)
                goal = taskManager.getTask(goalId)
                const buildId = goal.subtasks.filter(node => node.title === "制作与打磨")[0].id
                taskManager.createSubtask(goalId, "完成第一个页面", buildId)
                taskManager.createSubtask(goalId, "请朋友体验并反馈", buildId)
                taskManager.createSubtask(goalId, "正式发布", 0)
                goal = taskManager.getTask(goalId)
                const firstActionId = goal.subtasks.filter(node => Number(node.parentId) === Number(planningId))[0].id
                taskManager.toggleSubtask(goalId, firstActionId)
            }
            currentPage = 2
        }
        else if (screenshotScenario === "garden") currentPage = 3
        else if (screenshotScenario === "garden-showcase") {
            if (taskManager.totalCount === 0 && growthManager.totalXp === 0) {
                growthManager.plantSeed(0, "sunflower")
                growthManager.plantSeed(1, "rose")
                for (let i = 0; i < 26; ++i) {
                    taskManager.createTask("Garden visual test " + i, "", "", 1, 25, "")
                    taskManager.toggleTask(i + 1)
                }
            }
            currentPage = 3
        }
        else if (screenshotScenario === "garden-seeds") {
            currentPage = 3
            Qt.callLater(() => gardenPage.openSeeds(0))
        }
        else if (screenshotScenario === "settings-midnight") {
            settingsManager.theme = "midnight"
            currentPage = 4
        } else if (screenshotScenario === "task-dialog") {
            Qt.callLater(() => taskDialog.openNew(Qt.formatDate(new Date(), "yyyy-MM-dd")))
        } else if (screenshotScenario === "task-dialog-advanced") {
            Qt.callLater(() => taskDialog.openScreenshotPreview(false))
        } else if (screenshotScenario === "date-picker") {
            Qt.callLater(() => taskDialog.openScreenshotPreview(true))
        }
    }
}
