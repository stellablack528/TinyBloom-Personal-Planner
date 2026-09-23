#include "database/DatabaseManager.h"
#include "managers/SettingsManager.h"
#include "managers/TaskManager.h"
#include "managers/GrowthManager.h"
#include "services/DataService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

#include <algorithm>

class CoreTests final : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void taskCrudAndPersistence();
    void subtasksAndCascadeDelete();
    void settingsPersistence();
    void jsonRoundTripAndInvalidImportSafety();
    void dataServiceImportCreatesSafetyBackup();
    void backgroundExportUsesThreadPoolAndCallback();
    void growthRewardsAreOneTimeAndPersistent();
    void vitalityDecaysAfterInactiveDays();
    void gardenSeedsPersistAndGrowIndependently();
    void legacyDatabaseMigratesToGrowthSchema();
    void classificationSearchAndValidation();
    void incrementalTaskModelUpdates();
    void longTermTaskMapPersistsAndCompletes();
    void completeUserJourney();

private:
    std::unique_ptr<QTemporaryDir> m_temp;
    std::unique_ptr<DatabaseManager> m_database;
    std::unique_ptr<TaskManager> m_tasks;
};

void CoreTests::init()
{
    m_temp = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp->isValid());
    m_database = std::make_unique<DatabaseManager>();
    QVERIFY(m_database->initializeDatabase(m_temp->filePath(QStringLiteral("test.sqlite3"))));
    m_tasks = std::make_unique<TaskManager>(m_database.get());
    QVERIFY(m_tasks->initialize());
}

void CoreTests::cleanup()
{
    m_tasks.reset();
    m_database.reset();
    m_temp.reset();
}

void CoreTests::taskCrudAndPersistence()
{
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    QVERIFY(m_tasks->createTask(QStringLiteral(" Read 10 pages "), QStringLiteral("A chapter"), today, 2, 20, QStringLiteral("Study")));
    QCOMPARE(m_tasks->totalCount(), 1);
    QCOMPARE(m_tasks->todayCount(), 1);
    const qint64 id = m_tasks->getTask(1).value(QStringLiteral("id")).toLongLong();
    QVERIFY(id > 0);
    QCOMPARE(m_tasks->getTask(id).value(QStringLiteral("title")).toString(), QStringLiteral("Read 10 pages"));
    QVERIFY(m_tasks->updateTask(id, QStringLiteral("Read 12 pages"), QString{}, today, 1, 25, QStringLiteral("Personal")));
    QVERIFY(m_tasks->toggleTask(id));
    QCOMPARE(m_tasks->todayCompletedCount(), 1);

    TaskManager reopened(m_database.get());
    QVERIFY(reopened.initialize());
    QCOMPARE(reopened.totalCount(), 1);
    QCOMPARE(reopened.getTask(id).value(QStringLiteral("completed")).toBool(), true);
    QVERIFY(reopened.deleteTask(id));
    QCOMPARE(reopened.totalCount(), 0);
}

void CoreTests::subtasksAndCascadeDelete()
{
    QVERIFY(m_tasks->createTask(QStringLiteral("Study networks")));
    const qint64 taskId = m_database->exportObject().value(QStringLiteral("tasks")).toArray().first().toObject().value(QStringLiteral("id")).toInteger();
    QVERIFY(m_tasks->createSubtask(taskId, QStringLiteral("Review TCP")));
    QVERIFY(m_tasks->createSubtask(taskId, QStringLiteral("Practice sockets")));
    auto task = m_tasks->getTask(taskId);
    QCOMPARE(task.value(QStringLiteral("subtasks")).toList().size(), 2);
    const qint64 subtaskId = task.value(QStringLiteral("subtasks")).toList().first().toMap().value(QStringLiteral("id")).toLongLong();
    QVERIFY(m_tasks->toggleSubtask(taskId, subtaskId));
    QVERIFY(m_tasks->deleteTask(taskId));
    QCOMPARE(m_database->exportObject().value(QStringLiteral("subtasks")).toArray().size(), 0);
}

void CoreTests::settingsPersistence()
{
    SettingsManager settings(m_database.get());
    settings.load();
    QCOMPARE(settings.language(), QStringLiteral("zh_CN"));
    settings.setTheme(QStringLiteral("midnight"));
    settings.setLanguage(QStringLiteral("en"));
    settings.setReduceAnimations(true);
    settings.setCompatibilityRendering(true);
    settings.setDefaultTaskDuration(40);
    SettingsManager reopened(m_database.get());
    reopened.load();
    QCOMPARE(reopened.theme(), QStringLiteral("midnight"));
    QCOMPARE(reopened.language(), QStringLiteral("en"));
    QCOMPARE(reopened.reduceAnimations(), true);
    QCOMPARE(reopened.compatibilityRendering(), true);
    QCOMPARE(reopened.defaultTaskDuration(), 40);
}

void CoreTests::jsonRoundTripAndInvalidImportSafety()
{
    QVERIFY(m_tasks->createTask(QStringLiteral("Export me"), QString{}, QDate::currentDate().addDays(1).toString(Qt::ISODate)));
    const QJsonObject exported = m_database->exportObject();
    QVERIFY(exported.value(QStringLiteral("version")).isString());
    QCOMPARE(exported.value(QStringLiteral("application")).toString(), QStringLiteral("TinyBloom"));
    QCOMPARE(exported.value(QStringLiteral("platform")).toString(), QStringLiteral("desktop"));
    QCOMPARE(exported.value(QStringLiteral("schemaVersion")).toInt(), 2);
    QCOMPARE(exported.value(QStringLiteral("tasks")).toArray().size(), 1);

    QJsonObject invalid{{"version", "0.1.0"}, {"tasks", QJsonArray{QJsonObject{{"title", ""}}}},
        {"subtasks", QJsonArray{}}, {"settings", QJsonObject{}}};
    QVERIFY(!m_database->importObject(invalid));
    QCOMPARE(m_database->loadTasks().size(), 1);

    QVERIFY(m_tasks->createTask(QStringLiteral("Temporary")));
    QJsonObject duplicateIds = exported;
    QJsonArray duplicateTasks = exported.value(QStringLiteral("tasks")).toArray();
    duplicateTasks.append(duplicateTasks.first());
    duplicateIds.insert(QStringLiteral("tasks"), duplicateTasks);
    QVERIFY(!m_database->importObject(duplicateIds));
    QCOMPARE(m_database->loadTasks().size(), 2);

    QVERIFY(m_database->importObject(exported));
    QCOMPARE(m_database->loadTasks().size(), 1);
    QCOMPARE(m_database->loadTasks().first().title, QStringLiteral("Export me"));
}

void CoreTests::dataServiceImportCreatesSafetyBackup()
{
    SettingsManager settings(m_database.get());
    settings.load();
    DataService dataService(m_database.get(), m_tasks.get(), &settings);

    QVERIFY(m_tasks->createTask(QStringLiteral("Export me")));
    const QString importPath = m_temp->filePath(QStringLiteral("import.json"));
    QVERIFY(dataService.exportData(QUrl::fromLocalFile(importPath)));
    QVERIFY(m_tasks->createTask(QStringLiteral("Keep in backup")));
    QCOMPARE(m_tasks->totalCount(), 2);

    QSignalSpy successSpy(&dataService, &DataService::operationSucceeded);
    QVERIFY(dataService.importData(QUrl::fromLocalFile(importPath)));
    QCOMPARE(m_tasks->totalCount(), 1);
    QCOMPARE(m_database->loadTasks().first().title, QStringLiteral("Export me"));
    QCOMPARE(successSpy.count(), 1);

    const QDir dataFolder(QFileInfo(m_database->databasePath()).absolutePath());
    const QStringList backups = dataFolder.entryList(
        {QStringLiteral("TinyBloom-backup-before-import-*.json")}, QDir::Files);
    QCOMPARE(backups.size(), 1);
    QFile backup(dataFolder.filePath(backups.first()));
    QVERIFY(backup.open(QIODevice::ReadOnly));
    QJsonParseError parseError;
    const QJsonDocument backupDocument = QJsonDocument::fromJson(backup.readAll(), &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    QCOMPARE(backupDocument.object().value(QStringLiteral("tasks")).toArray().size(), 2);
}

void CoreTests::backgroundExportUsesThreadPoolAndCallback()
{
    SettingsManager settings(m_database.get());
    settings.load();
    DataService dataService(m_database.get(), m_tasks.get(), &settings);
    QVERIFY(m_tasks->createTask(QStringLiteral("Export without blocking the UI")));

    const QString path = m_temp->filePath(QStringLiteral("background-export.json"));
    QSignalSpy successSpy(&dataService, &DataService::operationSucceeded);
    QSignalSpy failureSpy(&dataService, &DataService::operationFailed);
    QVERIFY(dataService.exportDataAsync(QUrl::fromLocalFile(path)));
    QVERIFY(dataService.busy());
    QVERIFY(!dataService.exportDataAsync(QUrl::fromLocalFile(path)));
    QTRY_COMPARE_WITH_TIMEOUT(successSpy.count(), 1, 5000);
    QCOMPARE(failureSpy.count(), 1);
    QVERIFY(!dataService.busy());

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    QCOMPARE(document.object().value(QStringLiteral("tasks")).toArray().size(), 1);
}

void CoreTests::growthRewardsAreOneTimeAndPersistent()
{
    GrowthManager growth(m_database.get());
    QVERIFY(growth.initialize());
    connect(m_tasks.get(), &TaskManager::taskCompleted, &growth, &GrowthManager::recordTaskCompleted);
    connect(m_tasks.get(), &TaskManager::subtaskCompleted, &growth, &GrowthManager::recordSubtaskCompleted);

    QVERIFY(m_tasks->createTask(QStringLiteral("Grow one leaf")));
    const qint64 taskId = m_database->exportObject().value(QStringLiteral("tasks")).toArray().first().toObject().value(QStringLiteral("id")).toInteger();
    QSignalSpy rewardSpy(&growth, &GrowthManager::experienceAwarded);
    QVERIFY(m_tasks->toggleTask(taskId));
    QCOMPARE(growth.totalXp(), 20);
    QCOMPARE(growth.todayXp(), 20);
    QCOMPARE(growth.progressDays(), 1);
    QCOMPARE(rewardSpy.count(), 1);

    QVERIFY(m_tasks->toggleTask(taskId));
    QVERIFY(m_tasks->toggleTask(taskId));
    QCOMPARE(growth.totalXp(), 20);
    QCOMPARE(rewardSpy.count(), 1);

    QVERIFY(m_tasks->createSubtask(taskId, QStringLiteral("Water it")));
    const QVariantMap task = m_tasks->getTask(taskId);
    const qint64 subtaskId = task.value(QStringLiteral("subtasks")).toList().first().toMap().value(QStringLiteral("id")).toLongLong();
    QVERIFY(m_tasks->toggleSubtask(taskId, subtaskId));
    QCOMPARE(growth.totalXp(), 25);
    QCOMPARE(growth.todayXp(), 25);
    QCOMPARE(rewardSpy.count(), 2);

    GrowthManager reopened(m_database.get());
    QVERIFY(reopened.initialize());
    QCOMPARE(reopened.totalXp(), 25);
    QCOMPARE(reopened.progressDays(), 1);
}

void CoreTests::vitalityDecaysAfterInactiveDays()
{
    QJsonObject exported = m_database->exportObject();
    QJsonObject growth = exported.value(QStringLiteral("growth")).toObject();
    growth.insert(QStringLiteral("vitality"), 100);
    growth.insert(QStringLiteral("vitalityUpdatedDate"), QDate::currentDate().addDays(-5).toString(Qt::ISODate));
    exported.insert(QStringLiteral("growth"), growth);
    QVERIFY(m_database->importObject(exported));

    GrowthManager growthManager(m_database.get());
    QVERIFY(growthManager.initialize());
    QCOMPARE(growthManager.vitality(), 25);
    QCOMPARE(growthManager.vitalityState(), 2);
    QCOMPARE(growthManager.totalXp(), 0);
}

void CoreTests::gardenSeedsPersistAndGrowIndependently()
{
    GrowthManager growth(m_database.get());
    QVERIFY(growth.initialize());
    QCOMPARE(growth.firstPlantSpecies(), QString{});
    QCOMPARE(growth.secondPlantSpecies(), QString{});
    QVERIFY(!growth.plantSeed(2, QStringLiteral("rose")));
    QVERIFY(!growth.plantSeed(0, QStringLiteral("cactus")));
    QVERIFY(growth.plantSeed(0, QStringLiteral("sunflower")));
    QVERIFY(growth.plantSeed(1, QStringLiteral("rose")));
    QCOMPARE(growth.plantStage(0), 0);

    connect(m_tasks.get(), &TaskManager::taskCompleted, &growth, &GrowthManager::recordTaskCompleted);
    for (int i = 0; i < 3; ++i) {
        QVERIFY(m_tasks->createTask(QStringLiteral("Grow %1").arg(i)));
        const qint64 id = m_database->exportObject().value(QStringLiteral("tasks"))
            .toArray().first().toObject().value(QStringLiteral("id")).toInteger();
        QVERIFY(m_tasks->toggleTask(id));
    }
    QCOMPARE(growth.plantEarnedXp(0), 60);
    QCOMPARE(growth.plantStage(0), 1);
    QCOMPARE(growth.plantStage(1), 1);
    QCOMPARE(m_database->exportObject().value(QStringLiteral("settings")).toObject()
        .value(QStringLiteral("garden.plant.0")).toString(), QStringLiteral("sunflower|0"));

    QVERIFY(growth.plantSeed(0, QStringLiteral("tulip")));
    QCOMPARE(growth.firstPlantSpecies(), QStringLiteral("tulip"));
    QCOMPARE(growth.plantEarnedXp(0), 0);
    QCOMPARE(growth.plantStage(0), 0);

    GrowthManager reopened(m_database.get());
    QVERIFY(reopened.initialize());
    QCOMPARE(reopened.firstPlantSpecies(), QStringLiteral("tulip"));
    QCOMPARE(reopened.secondPlantSpecies(), QStringLiteral("rose"));
    QCOMPARE(reopened.plantStage(0), 0);
}

void CoreTests::legacyDatabaseMigratesToGrowthSchema()
{
    const QString path = m_temp->filePath(QStringLiteral("legacy.sqlite3"));
    const QString connectionName = QStringLiteral("legacy_setup");
    {
        QSqlDatabase legacy = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        legacy.setDatabaseName(path);
        QVERIFY(legacy.open());
        QSqlQuery query(legacy);
        QVERIFY(query.exec(QStringLiteral("CREATE TABLE tasks (id INTEGER PRIMARY KEY AUTOINCREMENT,title TEXT NOT NULL,description TEXT NOT NULL DEFAULT '',completed INTEGER NOT NULL DEFAULT 0,priority INTEGER NOT NULL DEFAULT 1,due_date TEXT,estimated_minutes INTEGER NOT NULL DEFAULT 25,category TEXT NOT NULL DEFAULT '',created_at TEXT NOT NULL,updated_at TEXT NOT NULL,completed_at TEXT)")));
        QVERIFY(query.exec(QStringLiteral("CREATE TABLE subtasks (id INTEGER PRIMARY KEY AUTOINCREMENT,task_id INTEGER NOT NULL,title TEXT NOT NULL,completed INTEGER NOT NULL DEFAULT 0,created_at TEXT NOT NULL,FOREIGN KEY(task_id) REFERENCES tasks(id) ON DELETE CASCADE)")));
        QVERIFY(query.exec(QStringLiteral("CREATE TABLE settings (key TEXT PRIMARY KEY,value TEXT NOT NULL)")));
        QVERIFY(query.exec(QStringLiteral("INSERT INTO tasks(title,description,completed,priority,estimated_minutes,category,created_at,updated_at,completed_at) VALUES('Already finished','',1,1,25,'','2026-01-01T00:00:00.000Z','2026-01-01T00:00:00.000Z','2026-01-01T00:00:00.000Z')")));
        legacy.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    DatabaseManager migrated;
    QVERIFY(migrated.initializeDatabase(path));
    QCOMPARE(migrated.loadTasks().size(), 1);
    GrowthManager growth(&migrated);
    QVERIFY(growth.initialize());
    QCOMPARE(growth.totalXp(), 20);
    QCOMPARE(growth.progressDays(), 1);
    QCOMPARE(growth.firstPlantSpecies(), QStringLiteral("pink"));
}

void CoreTests::classificationSearchAndValidation()
{
    const QDate today = QDate::currentDate();
    QVERIFY(m_tasks->createTask(QStringLiteral("Today note"), QStringLiteral("alpha"), today.toString(Qt::ISODate), 1, 10, QStringLiteral("Home")));
    QVERIFY(m_tasks->createTask(QStringLiteral("Tomorrow note"), QStringLiteral("beta"), today.addDays(1).toString(Qt::ISODate)));
    QVERIFY(m_tasks->createTask(QStringLiteral("Later plan"), QStringLiteral("gamma"), today.addDays(5).toString(Qt::ISODate)));
    QVERIFY(m_tasks->createTask(QStringLiteral("Inbox plan")));
    QCOMPARE(m_tasks->todayTasks()->count(), 1);
    QCOMPARE(m_tasks->tomorrowTasks()->count(), 1);
    QCOMPARE(m_tasks->laterTasks()->count(), 2);

    m_tasks->searchTasks(QStringLiteral("gamma"));
    QCOMPARE(m_tasks->allTasks()->count(), 1);
    m_tasks->searchTasks(QStringLiteral("Home"));
    QCOMPARE(m_tasks->allTasks()->count(), 1);
    m_tasks->searchTasks(QString{});
    m_tasks->filterTasks(1);
    m_tasks->setTaskScope(0);
    QCOMPARE(m_tasks->allTasks()->count(), 4);
    m_tasks->setTaskScope(2);
    QCOMPARE(m_tasks->allTasks()->count(), 1);
    m_tasks->setTaskScope(0);
    QVERIFY(m_tasks->toggleTask(m_tasks->getTask(1).value(QStringLiteral("id")).toLongLong()));
    m_tasks->filterTasks(2);
    QCOMPARE(m_tasks->allTasks()->count(), 1);

    QVERIFY(!m_tasks->createTask(QStringLiteral("Bad date"), QString{}, QStringLiteral("2026-99-42")));
    QCOMPARE(m_tasks->totalCount(), 4);
}

void CoreTests::incrementalTaskModelUpdates()
{
    QSignalSpy resetSpy(m_tasks->allTasks(), &QAbstractItemModel::modelReset);
    QSignalSpy insertedSpy(m_tasks->allTasks(), &QAbstractItemModel::rowsInserted);
    QSignalSpy removedSpy(m_tasks->allTasks(), &QAbstractItemModel::rowsRemoved);

    QVERIFY(m_tasks->createTask(QStringLiteral("First task")));
    QVERIFY(m_tasks->createTask(QStringLiteral("Second task")));
    QCOMPARE(resetSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 2);

    const qint64 firstId = m_database->exportObject().value(QStringLiteral("tasks"))
        .toArray().last().toObject().value(QStringLiteral("id")).toInteger();
    QVERIFY(m_tasks->toggleTask(firstId));
    QCOMPARE(resetSpy.count(), 0);
    QVERIFY(m_tasks->deleteTask(firstId));
    QCOMPARE(resetSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
}

void CoreTests::longTermTaskMapPersistsAndCompletes()
{
    QVERIFY(m_tasks->createTask(QStringLiteral("Keep ordinary tasks separate")));
    QVERIFY(m_tasks->createLongTermTask(QStringLiteral("Finish graduation project"),
        QStringLiteral("Build something useful"), QDate::currentDate().addMonths(3).toString(Qt::ISODate)));
    QCOMPARE(m_tasks->totalCount(), 1);
    QCOMPARE(m_tasks->allTasks()->count(), 1);
    QCOMPARE(m_tasks->longTermTasks()->count(), 1);

    const qint64 goalId = m_tasks->longTermTasks()->get(0).value(QStringLiteral("taskId")).toLongLong();
    QVERIFY(goalId > 0);
    QVERIFY(m_tasks->createSubtask(goalId, QStringLiteral("Research")));
    QVariantMap goal = m_tasks->getTask(goalId);
    const qint64 branchId = goal.value(QStringLiteral("subtasks")).toList().first().toMap()
        .value(QStringLiteral("id")).toLongLong();
    QVERIFY(m_tasks->createSubtask(goalId, QStringLiteral("Read three papers"), branchId));
    QVERIFY(m_tasks->createSubtask(goalId, QStringLiteral("Build prototype")));
    goal = m_tasks->getTask(goalId);
    const QVariantList nodes = goal.value(QStringLiteral("subtasks")).toList();
    QCOMPARE(nodes.size(), 3);
    qint64 actionId = 0;
    qint64 standaloneId = 0;
    for (const QVariant &value : nodes) {
        const QVariantMap node = value.toMap();
        if (node.value(QStringLiteral("parentId")).toLongLong() == branchId) actionId = node.value(QStringLiteral("id")).toLongLong();
        if (node.value(QStringLiteral("title")).toString() == QStringLiteral("Build prototype")) standaloneId = node.value(QStringLiteral("id")).toLongLong();
    }
    QVERIFY(actionId > 0);
    QVERIFY(standaloneId > 0);
    QVERIFY(!m_tasks->createSubtask(goalId, QStringLiteral("Too deep"), actionId));
    QVERIFY(m_tasks->updateSubtask(goalId, actionId, QStringLiteral("Read five papers")));
    QVERIFY(m_tasks->toggleSubtask(goalId, actionId));
    QCOMPARE(m_tasks->getTask(goalId).value(QStringLiteral("completed")).toBool(), false);
    QVERIFY(m_tasks->toggleSubtask(goalId, standaloneId));
    QCOMPARE(m_tasks->getTask(goalId).value(QStringLiteral("completed")).toBool(), true);
    QCOMPARE(m_tasks->longTermTasks()->get(0).value(QStringLiteral("leafCount")).toInt(), 2);
    QCOMPARE(m_tasks->longTermTasks()->get(0).value(QStringLiteral("completedLeafCount")).toInt(), 2);

    const QJsonObject exported = m_database->exportObject();
    bool foundLongTerm = false;
    bool foundNestedNode = false;
    for (const auto &value : exported.value(QStringLiteral("tasks")).toArray())
        if (value.toObject().value(QStringLiteral("longTerm")).toBool()) foundLongTerm = true;
    for (const auto &value : exported.value(QStringLiteral("subtasks")).toArray())
        if (value.toObject().value(QStringLiteral("parentId")).toInteger() > 0) foundNestedNode = true;
    QVERIFY(foundLongTerm);
    QVERIFY(foundNestedNode);

    QVERIFY(m_database->importObject(exported));
    m_tasks->reload();
    QCOMPARE(m_tasks->longTermTasks()->count(), 1);
    const qint64 importedGoalId = m_tasks->longTermTasks()->get(0).value(QStringLiteral("taskId")).toLongLong();
    const QVariantList importedNodes = m_tasks->getTask(importedGoalId).value(QStringLiteral("subtasks")).toList();
    QCOMPARE(importedNodes.size(), 3);
    QVERIFY(std::any_of(importedNodes.cbegin(), importedNodes.cend(), [](const QVariant &value) {
        return value.toMap().value(QStringLiteral("parentId")).toLongLong() > 0;
    }));

    qint64 importedBranchId = 0;
    for (const QVariant &value : importedNodes) {
        const QVariantMap node = value.toMap();
        if (node.value(QStringLiteral("title")).toString() == QStringLiteral("Research"))
            importedBranchId = node.value(QStringLiteral("id")).toLongLong();
    }
    QVERIFY(importedBranchId > 0);
    QVERIFY(m_tasks->deleteSubtask(importedGoalId, importedBranchId));
    QCOMPARE(m_tasks->getTask(importedGoalId).value(QStringLiteral("subtasks")).toList().size(), 1);
}

void CoreTests::completeUserJourney()
{
    const QDate today = QDate::currentDate();
    QVERIFY(m_tasks->createTask(QStringLiteral("Read 10 pages"), QString{}, today.toString(Qt::ISODate), 1, 20, QStringLiteral("Reading")));
    const qint64 readingId = m_database->exportObject().value(QStringLiteral("tasks")).toArray().first().toObject().value(QStringLiteral("id")).toInteger();
    QVERIFY(m_tasks->createSubtask(readingId, QStringLiteral("Open the book")));
    QVERIFY(m_tasks->createSubtask(readingId, QStringLiteral("Write one note")));
    QVariantList steps = m_tasks->getTask(readingId).value(QStringLiteral("subtasks")).toList();
    QCOMPARE(steps.size(), 2);
    QVERIFY(m_tasks->toggleSubtask(readingId, steps.first().toMap().value(QStringLiteral("id")).toLongLong()));
    QVERIFY(m_tasks->toggleTask(readingId));

    m_tasks.reset();
    m_tasks = std::make_unique<TaskManager>(m_database.get());
    QVERIFY(m_tasks->initialize());
    const QVariantMap reopenedReading = m_tasks->getTask(readingId);
    QCOMPARE(reopenedReading.value(QStringLiteral("completed")).toBool(), true);
    QCOMPARE(reopenedReading.value(QStringLiteral("subtasks")).toList().first().toMap().value(QStringLiteral("completed")).toBool(), true);

    QVERIFY(m_tasks->createTask(QStringLiteral("Tomorrow task"), QString{}, today.addDays(1).toString(Qt::ISODate)));
    QVERIFY(m_tasks->createTask(QStringLiteral("Later task"), QString{}, today.addDays(7).toString(Qt::ISODate)));
    const QJsonArray currentTasks = m_database->exportObject().value(QStringLiteral("tasks")).toArray();
    qint64 tomorrowId = 0;
    qint64 laterId = 0;
    for (const auto &value : currentTasks) {
        const QJsonObject task = value.toObject();
        if (task.value(QStringLiteral("title")).toString() == QStringLiteral("Tomorrow task")) tomorrowId = task.value(QStringLiteral("id")).toInteger();
        if (task.value(QStringLiteral("title")).toString() == QStringLiteral("Later task")) laterId = task.value(QStringLiteral("id")).toInteger();
    }
    QVERIFY(tomorrowId > 0);
    QVERIFY(laterId > 0);
    QCOMPARE(m_tasks->tomorrowTasks()->count(), 1);
    QCOMPARE(m_tasks->laterTasks()->count(), 1);
    m_tasks->searchTasks(QStringLiteral("Tomorrow"));
    QCOMPARE(m_tasks->allTasks()->count(), 1);
    QVERIFY(m_tasks->updateTask(tomorrowId, QStringLiteral("Tomorrow task edited"), QStringLiteral("Updated"), today.addDays(1).toString(Qt::ISODate), 2, 30, QStringLiteral("Plan")));
    QVERIFY(m_tasks->deleteTask(laterId));

    SettingsManager settings(m_database.get());
    settings.load();
    settings.setTheme(QStringLiteral("midnight"));
    SettingsManager reopenedSettings(m_database.get());
    reopenedSettings.load();
    QCOMPARE(reopenedSettings.theme(), QStringLiteral("midnight"));

    const QJsonObject exported = m_database->exportObject();
    QVERIFY(m_tasks->createTask(QStringLiteral("Not in export")));
    QVERIFY(m_database->importObject(exported));
    m_tasks->reload();
    QCOMPARE(m_tasks->totalCount(), 2);
    QCOMPARE(m_database->setting(QStringLiteral("theme")), QStringLiteral("midnight"));
}

QTEST_GUILESS_MAIN(CoreTests)
#include "tst_core.moc"
