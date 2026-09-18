#include "database/DatabaseManager.h"
#include "managers/SettingsManager.h"
#include "managers/TaskManager.h"

#include <QJsonArray>
#include <QTemporaryDir>
#include <QtTest>

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
    void classificationSearchAndValidation();
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
    settings.setDefaultTaskDuration(40);
    SettingsManager reopened(m_database.get());
    reopened.load();
    QCOMPARE(reopened.theme(), QStringLiteral("midnight"));
    QCOMPARE(reopened.language(), QStringLiteral("en"));
    QCOMPARE(reopened.reduceAnimations(), true);
    QCOMPARE(reopened.defaultTaskDuration(), 40);
}

void CoreTests::jsonRoundTripAndInvalidImportSafety()
{
    QVERIFY(m_tasks->createTask(QStringLiteral("Export me"), QString{}, QDate::currentDate().addDays(1).toString(Qt::ISODate)));
    const QJsonObject exported = m_database->exportObject();
    QVERIFY(exported.value(QStringLiteral("version")).isString());
    QCOMPARE(exported.value(QStringLiteral("tasks")).toArray().size(), 1);

    QJsonObject invalid{{"version", "0.1.0"}, {"tasks", QJsonArray{QJsonObject{{"title", ""}}}},
        {"subtasks", QJsonArray{}}, {"settings", QJsonObject{}}};
    QVERIFY(!m_database->importObject(invalid));
    QCOMPARE(m_database->loadTasks().size(), 1);

    QVERIFY(m_tasks->createTask(QStringLiteral("Temporary")));
    QVERIFY(m_database->importObject(exported));
    QCOMPARE(m_database->loadTasks().size(), 1);
    QCOMPARE(m_database->loadTasks().first().title, QStringLiteral("Export me"));
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
    m_tasks->setTaskScope(2);
    QCOMPARE(m_tasks->allTasks()->count(), 1);
    m_tasks->setTaskScope(0);
    QVERIFY(m_tasks->toggleTask(m_tasks->getTask(1).value(QStringLiteral("id")).toLongLong()));
    m_tasks->filterTasks(2);
    QCOMPARE(m_tasks->allTasks()->count(), 1);

    QVERIFY(!m_tasks->createTask(QStringLiteral("Bad date"), QString{}, QStringLiteral("2026-99-42")));
    QCOMPARE(m_tasks->totalCount(), 4);
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
