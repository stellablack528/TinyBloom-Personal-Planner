#include "DatabaseManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>

Q_LOGGING_CATEGORY(databaseLog, "tinybloom.database")

namespace {
QString iso(const QDateTime &value)
{
    return value.isValid() ? value.toUTC().toString(Qt::ISODateWithMs) : QString{};
}

QDateTime dateTime(const QVariant &value)
{
    return QDateTime::fromString(value.toString(), Qt::ISODateWithMs);
}
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent),
      m_connectionName(QStringLiteral("tinybloom_%1").arg(QUuid::createUuid().toString(QUuid::Id128)))
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::initializeDatabase(const QString &overridePath)
{
    QString path = overridePath;
    if (path.isEmpty()) {
        const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (directory.isEmpty() || !QDir().mkpath(directory)) {
            setError(tr("Unable to prepare the data folder."), directory);
            return false;
        }
        path = QDir(directory).filePath(QStringLiteral("tinybloom.sqlite3"));
    } else {
        const QFileInfo info(path);
        if (!QDir().mkpath(info.absolutePath())) {
            setError(tr("Unable to prepare the data folder."), info.absolutePath());
            return false;
        }
    }
    return openDatabase(path) && createTables();
}

bool DatabaseManager::openDatabase(const QString &path)
{
    if (m_database.isOpen()) closeDatabase();
    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(path);
    if (!m_database.open()) {
        setError(tr("Unable to open the local database."), m_database.lastError().text());
        return false;
    }
    m_databasePath = path;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA foreign_keys = ON"))) {
        setError(tr("Unable to enable database integrity checks."), query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::createTables()
{
    if (!(execute(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL CHECK(length(title) BETWEEN 1 AND 160),
            description TEXT NOT NULL DEFAULT '',
            completed INTEGER NOT NULL DEFAULT 0,
            priority INTEGER NOT NULL DEFAULT 1 CHECK(priority BETWEEN 0 AND 2),
            due_date TEXT,
            estimated_minutes INTEGER NOT NULL DEFAULT 25 CHECK(estimated_minutes >= 0),
            category TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            completed_at TEXT,
            experience_awarded INTEGER NOT NULL DEFAULT 0,
            long_term INTEGER NOT NULL DEFAULT 0
        ))"))
        && execute(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS subtasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            task_id INTEGER NOT NULL,
            title TEXT NOT NULL CHECK(length(title) BETWEEN 1 AND 160),
            completed INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL,
            experience_awarded INTEGER NOT NULL DEFAULT 0,
            parent_id INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY(task_id) REFERENCES tasks(id) ON DELETE CASCADE
        ))"))
        && execute(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        ))"))
        && execute(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS growth_profile (
            id INTEGER PRIMARY KEY CHECK(id = 1),
            total_xp INTEGER NOT NULL DEFAULT 0 CHECK(total_xp >= 0),
            progress_days INTEGER NOT NULL DEFAULT 0 CHECK(progress_days >= 0),
            today_xp INTEGER NOT NULL DEFAULT 0 CHECK(today_xp >= 0),
            last_progress_date TEXT NOT NULL DEFAULT '',
            vitality INTEGER NOT NULL DEFAULT 100 CHECK(vitality BETWEEN 0 AND 100),
            vitality_updated_date TEXT NOT NULL DEFAULT ''
        ))"))
        && execute(QStringLiteral("INSERT OR IGNORE INTO growth_profile(id,total_xp,progress_days,today_xp,last_progress_date,vitality,vitality_updated_date) VALUES(1,0,0,0,'',100,'')"))
        && execute(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date)"))
        && execute(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_subtasks_task_id ON subtasks(task_id)")))) return false;

    return ensureColumn(QStringLiteral("tasks"), QStringLiteral("experience_awarded"),
               QStringLiteral("INTEGER NOT NULL DEFAULT 0"))
        && ensureColumn(QStringLiteral("tasks"), QStringLiteral("long_term"),
               QStringLiteral("INTEGER NOT NULL DEFAULT 0"))
        && ensureColumn(QStringLiteral("subtasks"), QStringLiteral("experience_awarded"),
               QStringLiteral("INTEGER NOT NULL DEFAULT 0"))
        && ensureColumn(QStringLiteral("subtasks"), QStringLiteral("parent_id"),
               QStringLiteral("INTEGER NOT NULL DEFAULT 0"));
}

void DatabaseManager::closeDatabase()
{
    if (!m_database.isValid()) return;
    const QString name = m_connectionName;
    m_database.close();
    m_database = {};
    QSqlDatabase::removeDatabase(name);
}

bool DatabaseManager::isOpen() const { return m_database.isOpen(); }
QString DatabaseManager::databasePath() const { return m_databasePath; }
QString DatabaseManager::lastError() const { return m_lastError; }

void DatabaseManager::setExportMetadata(const QString &platform, const QString &version)
{
    const QString normalizedPlatform = platform.trimmed().toLower();
    const QString normalizedVersion = version.trimmed();
    m_exportPlatform = normalizedPlatform.isEmpty() ? QStringLiteral("desktop") : normalizedPlatform.left(32);
    m_exportVersion = normalizedVersion.isEmpty() ? QStringLiteral("0.3.1-beta.1") : normalizedVersion.left(64);
}

QVector<Task> DatabaseManager::loadTasks() const
{
    QVector<Task> tasks;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT id,title,description,completed,priority,due_date,estimated_minutes,category,created_at,updated_at,completed_at,experience_awarded,long_term FROM tasks ORDER BY completed ASC, due_date IS NULL, due_date ASC, created_at DESC"))) {
        setError(tr("Unable to load tasks."), query.lastError().text());
        return tasks;
    }
    while (query.next()) {
        Task task;
        task.id = query.value(0).toLongLong();
        task.title = query.value(1).toString();
        task.description = query.value(2).toString();
        task.completed = query.value(3).toBool();
        task.priority = priorityFromInt(query.value(4).toInt());
        task.dueDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        task.estimatedMinutes = query.value(6).toInt();
        task.category = query.value(7).toString();
        task.createdAt = dateTime(query.value(8));
        task.updatedAt = dateTime(query.value(9));
        task.completedAt = dateTime(query.value(10));
        task.experienceAwarded = query.value(11).toBool();
        task.longTerm = query.value(12).toBool();
        tasks.append(task);
    }

    QHash<qint64, int> taskIndexes;
    for (int i = 0; i < tasks.size(); ++i) taskIndexes.insert(tasks[i].id, i);
    QSqlQuery subquery(m_database);
    if (!subquery.exec(QStringLiteral("SELECT id,task_id,title,completed,created_at,experience_awarded,parent_id FROM subtasks ORDER BY id ASC"))) {
        setError(tr("Unable to load subtasks."), subquery.lastError().text());
        return tasks;
    }
    while (subquery.next()) {
        const qint64 taskId = subquery.value(1).toLongLong();
        if (!taskIndexes.contains(taskId)) continue;
        Subtask subtask;
        subtask.id = subquery.value(0).toLongLong();
        subtask.taskId = taskId;
        subtask.title = subquery.value(2).toString();
        subtask.completed = subquery.value(3).toBool();
        subtask.createdAt = dateTime(subquery.value(4));
        subtask.experienceAwarded = subquery.value(5).toBool();
        subtask.parentId = subquery.value(6).toLongLong();
        tasks[taskIndexes.value(taskId)].subtasks.append(subtask);
    }
    return tasks;
}

bool DatabaseManager::insertTask(Task &task)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("INSERT INTO tasks(title,description,completed,priority,due_date,estimated_minutes,category,created_at,updated_at,completed_at,experience_awarded,long_term) VALUES(?,?,?,?,?,?,?,?,?,?,?,?)"));
    query.addBindValue(task.title);
    query.addBindValue(task.description.isNull() ? QStringLiteral("") : task.description);
    query.addBindValue(task.completed);
    query.addBindValue(static_cast<int>(task.priority));
    query.addBindValue(task.dueDate.isValid() ? task.dueDate.toString(Qt::ISODate) : QVariant{});
    query.addBindValue(task.estimatedMinutes);
    query.addBindValue(task.category.isNull() ? QStringLiteral("") : task.category);
    query.addBindValue(iso(task.createdAt));
    query.addBindValue(iso(task.updatedAt));
    query.addBindValue(task.completedAt.isValid() ? iso(task.completedAt) : QVariant{});
    query.addBindValue(task.experienceAwarded);
    query.addBindValue(task.longTerm);
    if (!query.exec()) {
        setError(tr("Unable to save this task."), query.lastError().text());
        return false;
    }
    task.id = query.lastInsertId().toLongLong();
    return true;
}

bool DatabaseManager::updateTask(const Task &task)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("UPDATE tasks SET title=?,description=?,completed=?,priority=?,due_date=?,estimated_minutes=?,category=?,updated_at=?,completed_at=?,long_term=? WHERE id=?"));
    query.addBindValue(task.title);
    query.addBindValue(task.description.isNull() ? QStringLiteral("") : task.description);
    query.addBindValue(task.completed);
    query.addBindValue(static_cast<int>(task.priority));
    query.addBindValue(task.dueDate.isValid() ? task.dueDate.toString(Qt::ISODate) : QVariant{});
    query.addBindValue(task.estimatedMinutes);
    query.addBindValue(task.category.isNull() ? QStringLiteral("") : task.category);
    query.addBindValue(iso(task.updatedAt));
    query.addBindValue(task.completedAt.isValid() ? iso(task.completedAt) : QVariant{});
    query.addBindValue(task.longTerm);
    query.addBindValue(task.id);
    if (!query.exec()) {
        setError(tr("Unable to update this task."), query.lastError().text());
        return false;
    }
    return query.numRowsAffected() == 1;
}

bool DatabaseManager::deleteTask(const qint64 taskId)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM tasks WHERE id=?"));
    query.addBindValue(taskId);
    if (!query.exec()) {
        setError(tr("Unable to delete this task."), query.lastError().text());
        return false;
    }
    return query.numRowsAffected() == 1;
}

bool DatabaseManager::insertSubtask(Subtask &subtask)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("INSERT INTO subtasks(task_id,title,completed,created_at,experience_awarded,parent_id) VALUES(?,?,?,?,?,?)"));
    query.addBindValue(subtask.taskId);
    query.addBindValue(subtask.title);
    query.addBindValue(subtask.completed);
    query.addBindValue(iso(subtask.createdAt));
    query.addBindValue(subtask.experienceAwarded);
    query.addBindValue(subtask.parentId);
    if (!query.exec()) {
        setError(tr("Unable to save this small step."), query.lastError().text());
        return false;
    }
    subtask.id = query.lastInsertId().toLongLong();
    return true;
}

bool DatabaseManager::updateSubtask(const Subtask &subtask)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("UPDATE subtasks SET title=?,completed=? WHERE id=?"));
    query.addBindValue(subtask.title);
    query.addBindValue(subtask.completed);
    query.addBindValue(subtask.id);
    if (!query.exec()) {
        setError(tr("Unable to update this small step."), query.lastError().text());
        return false;
    }
    return query.numRowsAffected() == 1;
}

bool DatabaseManager::deleteSubtask(const qint64 subtaskId)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM subtasks WHERE id=?"));
    query.addBindValue(subtaskId);
    if (!query.exec()) {
        setError(tr("Unable to delete this small step."), query.lastError().text());
        return false;
    }
    return query.numRowsAffected() == 1;
}

QString DatabaseManager::setting(const QString &key, const QString &fallback) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT value FROM settings WHERE key=?"));
    query.addBindValue(key);
    if (!query.exec() || !query.next()) return fallback;
    return query.value(0).toString();
}

bool DatabaseManager::setSetting(const QString &key, const QString &value)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("INSERT INTO settings(key,value) VALUES(?,?) ON CONFLICT(key) DO UPDATE SET value=excluded.value"));
    query.addBindValue(key);
    query.addBindValue(value);
    if (!query.exec()) {
        setError(tr("Unable to save settings."), query.lastError().text());
        return false;
    }
    return true;
}

QJsonObject DatabaseManager::exportObject() const
{
    QJsonArray tasksArray;
    QJsonArray subtasksArray;
    for (const Task &task : loadTasks()) {
        tasksArray.append(QJsonObject{{"id", task.id}, {"title", task.title}, {"description", task.description},
            {"completed", task.completed}, {"priority", static_cast<int>(task.priority)},
            {"dueDate", task.dueDate.toString(Qt::ISODate)}, {"estimatedMinutes", task.estimatedMinutes},
            {"category", task.category}, {"createdAt", iso(task.createdAt)}, {"updatedAt", iso(task.updatedAt)},
            {"completedAt", iso(task.completedAt)}, {"experienceAwarded", task.experienceAwarded},
            {"longTerm", task.longTerm}});
        for (const Subtask &subtask : task.subtasks) {
            subtasksArray.append(QJsonObject{{"id", subtask.id}, {"taskId", subtask.taskId}, {"title", subtask.title},
                {"completed", subtask.completed}, {"createdAt", iso(subtask.createdAt)},
                {"experienceAwarded", subtask.experienceAwarded}, {"parentId", subtask.parentId}});
        }
    }
    QJsonObject settings;
    QSqlQuery query(m_database);
    if (query.exec(QStringLiteral("SELECT key,value FROM settings"))) {
        while (query.next()) settings.insert(query.value(0).toString(), query.value(1).toString());
    }
    const GrowthStats growth = loadGrowthStats();
    const QJsonObject growthObject{{"totalXp", growth.totalXp}, {"progressDays", growth.progressDays},
        {"todayXp", growth.todayXp}, {"lastProgressDate", growth.lastProgressDate.toString(Qt::ISODate)},
        {"vitality", growth.vitality}, {"vitalityUpdatedDate", growth.vitalityUpdatedDate.toString(Qt::ISODate)}};
    return QJsonObject{{"application", QStringLiteral("TinyBloom")},
        {"platform", m_exportPlatform}, {"schemaVersion", 2},
        {"version", m_exportVersion},
        {"exportedAt", iso(QDateTime::currentDateTimeUtc())}, {"tasks", tasksArray},
        {"subtasks", subtasksArray}, {"settings", settings}, {"growth", growthObject}};
}

bool DatabaseManager::importObject(const QJsonObject &root)
{
    if (!root.value("version").isString() || !root.value("tasks").isArray()
        || !root.value("subtasks").isArray() || !root.value("settings").isObject()) {
        setError(tr("This file is not a valid TinyBloom export."), QStringLiteral("Missing required JSON fields"));
        return false;
    }
    const QJsonArray tasks = root.value("tasks").toArray();
    QSet<qint64> taskIds;
    for (const auto &value : tasks) {
        const QJsonObject item = value.toObject();
        const qint64 taskId = item.value("id").toInteger();
        if (!value.isObject() || taskId <= 0 || taskIds.contains(taskId)
            || item.value("title").toString().trimmed().isEmpty()) {
            setError(tr("This file contains an invalid task."), QStringLiteral("Invalid task identity or title"));
            return false;
        }
        taskIds.insert(taskId);
    }
    QSet<qint64> subtaskIds;
    QHash<qint64, qint64> subtaskTaskIds;
    for (const auto &value : root.value("subtasks").toArray()) {
        const QJsonObject item = value.toObject();
        const qint64 subtaskId = item.value("id").toInteger();
        const qint64 parentId = item.value("taskId").toInteger();
        if (!value.isObject() || subtaskId <= 0 || subtaskIds.contains(subtaskId)
            || !taskIds.contains(parentId) || item.value("title").toString().trimmed().isEmpty()) {
            setError(tr("This file contains an invalid small step."), QStringLiteral("Invalid subtask identity"));
            return false;
        }
        subtaskIds.insert(subtaskId);
        subtaskTaskIds.insert(subtaskId, parentId);
    }
    for (const auto &value : root.value("subtasks").toArray()) {
        const QJsonObject item = value.toObject();
        const qint64 parentNodeId = item.value("parentId").toInteger(0);
        if (parentNodeId > 0 && (!subtaskIds.contains(parentNodeId)
            || subtaskTaskIds.value(parentNodeId) != item.value("taskId").toInteger())) {
            setError(tr("This file contains an invalid task tree."), QStringLiteral("Invalid subtask parent"));
            return false;
        }
    }
    if (!m_database.transaction()) {
        setError(tr("Unable to start data import."), m_database.lastError().text());
        return false;
    }
    auto rollback = [this](const QString &message) {
        m_database.rollback();
        setError(tr("Unable to import data. Your existing data is unchanged."), message);
        return false;
    };
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("DELETE FROM subtasks")) || !query.exec(QStringLiteral("DELETE FROM tasks"))
        || !query.exec(QStringLiteral("DELETE FROM settings"))) return rollback(query.lastError().text());

    QHash<qint64, qint64> ids;
    for (const auto &value : tasks) {
        const QJsonObject item = value.toObject();
        Task task;
        const qint64 oldId = item.value("id").toInteger();
        task.title = item.value("title").toString().trimmed().left(160);
        task.description = item.value("description").toString().left(4000);
        task.completed = item.value("completed").toBool();
        task.priority = priorityFromInt(item.value("priority").toInt(1));
        task.dueDate = QDate::fromString(item.value("dueDate").toString(), Qt::ISODate);
        task.estimatedMinutes = qBound(0, item.value("estimatedMinutes").toInt(25), 10080);
        task.category = item.value("category").toString().left(80);
        task.createdAt = QDateTime::fromString(item.value("createdAt").toString(), Qt::ISODateWithMs);
        if (!task.createdAt.isValid()) task.createdAt = QDateTime::currentDateTimeUtc();
        task.updatedAt = QDateTime::fromString(item.value("updatedAt").toString(), Qt::ISODateWithMs);
        if (!task.updatedAt.isValid()) task.updatedAt = task.createdAt;
        task.completedAt = QDateTime::fromString(item.value("completedAt").toString(), Qt::ISODateWithMs);
        task.experienceAwarded = item.value("experienceAwarded").toBool(false);
        task.longTerm = item.value("longTerm").toBool(false);
        if (!insertTask(task)) return rollback(m_lastError);
        ids.insert(oldId, task.id);
    }
    QHash<qint64, qint64> subtaskIdMap;
    const QJsonArray subtaskItems = root.value("subtasks").toArray();
    for (int pass = 0; pass < 2; ++pass) {
        for (const auto &value : subtaskItems) {
            const QJsonObject item = value.toObject();
            const qint64 oldParentNodeId = item.value("parentId").toInteger(0);
            if ((pass == 0) != (oldParentNodeId == 0)) continue;
            const qint64 parentTaskId = ids.value(item.value("taskId").toInteger(), 0);
            const QString title = item.value("title").toString().trimmed();
            if (!value.isObject() || parentTaskId == 0 || title.isEmpty()) return rollback(QStringLiteral("Invalid subtask"));
            Subtask subtask{0, parentTaskId, title.left(160), item.value("completed").toBool(),
                QDateTime::fromString(item.value("createdAt").toString(), Qt::ISODateWithMs)};
            subtask.experienceAwarded = item.value("experienceAwarded").toBool(false);
            subtask.parentId = oldParentNodeId > 0 ? subtaskIdMap.value(oldParentNodeId, 0) : 0;
            if (oldParentNodeId > 0 && subtask.parentId == 0) return rollback(QStringLiteral("Invalid subtask parent"));
            if (!subtask.createdAt.isValid()) subtask.createdAt = QDateTime::currentDateTimeUtc();
            if (!insertSubtask(subtask)) return rollback(m_lastError);
            subtaskIdMap.insert(item.value("id").toInteger(), subtask.id);
        }
    }
    const QJsonObject settings = root.value("settings").toObject();
    for (auto it = settings.begin(); it != settings.end(); ++it) {
        if (!it.value().isString() || !setSetting(it.key().left(80), it.value().toString().left(500)))
            return rollback(m_lastError);
    }
    const QJsonObject growth = root.value("growth").toObject();
    const int totalXp = qMax(0, growth.value("totalXp").toInt(0));
    const int progressDays = qMax(0, growth.value("progressDays").toInt(0));
    const QDate lastProgressDate = QDate::fromString(growth.value("lastProgressDate").toString(), Qt::ISODate);
    const int todayXp = lastProgressDate == QDate::currentDate()
        ? qMax(0, growth.value("todayXp").toInt(0)) : 0;
    const int vitality = qBound(0, growth.value("vitality").toInt(100), 100);
    const QDate vitalityUpdatedDate = QDate::fromString(growth.value("vitalityUpdatedDate").toString(), Qt::ISODate);
    query.prepare(QStringLiteral("UPDATE growth_profile SET total_xp=?,progress_days=?,today_xp=?,last_progress_date=?,vitality=?,vitality_updated_date=? WHERE id=1"));
    query.addBindValue(totalXp);
    query.addBindValue(progressDays);
    query.addBindValue(todayXp);
    query.addBindValue(lastProgressDate.isValid() ? lastProgressDate.toString(Qt::ISODate) : QStringLiteral(""));
    query.addBindValue(vitality);
    query.addBindValue((vitalityUpdatedDate.isValid() ? vitalityUpdatedDate : QDate::currentDate()).toString(Qt::ISODate));
    if (!query.exec()) return rollback(query.lastError().text());
    if (!m_database.commit()) return rollback(m_database.lastError().text());
    return true;
}

DatabaseManager::GrowthStats DatabaseManager::loadGrowthStats() const
{
    GrowthStats stats;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT total_xp,progress_days,today_xp,last_progress_date,vitality,vitality_updated_date FROM growth_profile WHERE id=1"))
        || !query.next()) {
        setError(tr("Unable to load your garden."), query.lastError().text());
        return stats;
    }
    stats.totalXp = query.value(0).toInt();
    stats.progressDays = query.value(1).toInt();
    stats.todayXp = query.value(2).toInt();
    stats.lastProgressDate = QDate::fromString(query.value(3).toString(), Qt::ISODate);
    stats.vitality = query.value(4).toInt();
    stats.vitalityUpdatedDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
    return stats;
}

bool DatabaseManager::awardTaskExperience(const qint64 taskId, const int xp, const int vitality, bool &awarded)
{
    return awardExperience(QStringLiteral("tasks"), taskId, xp, vitality, awarded);
}

bool DatabaseManager::awardSubtaskExperience(const qint64 subtaskId, const int xp, const int vitality, bool &awarded)
{
    return awardExperience(QStringLiteral("subtasks"), subtaskId, xp, vitality, awarded);
}

bool DatabaseManager::awardExperience(const QString &table, const qint64 sourceId, const int xp,
    const int vitality, bool &awarded)
{
    awarded = false;
    if (table != QStringLiteral("tasks") && table != QStringLiteral("subtasks")) return false;
    if (!m_database.transaction()) {
        setError(tr("Unable to update your garden."), m_database.lastError().text());
        return false;
    }
    const auto rollback = [this](const QString &technical) {
        m_database.rollback();
        setError(tr("Unable to update your garden."), technical);
        return false;
    };
    if (!applyVitalityDecay()) return rollback(m_lastError);
    QSqlQuery update(m_database);
    update.prepare(QStringLiteral("UPDATE %1 SET experience_awarded=1 WHERE id=? AND completed=1 AND experience_awarded=0").arg(table));
    update.addBindValue(sourceId);
    if (!update.exec()) return rollback(update.lastError().text());
    awarded = update.numRowsAffected() == 1;
    if (awarded && !addExperienceToProfile(qMax(0, xp), qMax(0, vitality))) return rollback(m_lastError);
    if (!m_database.commit()) return rollback(m_database.lastError().text());
    return true;
}

bool DatabaseManager::reconcileExperience(const int taskXp, const int taskVitality,
    const int subtaskXp, const int subtaskVitality, int &awardedXp)
{
    awardedXp = 0;
    if (!m_database.transaction()) {
        setError(tr("Unable to update your garden."), m_database.lastError().text());
        return false;
    }
    const auto rollback = [this](const QString &technical) {
        m_database.rollback();
        setError(tr("Unable to update your garden."), technical);
        return false;
    };
    if (!applyVitalityDecay()) return rollback(m_lastError);

    QSqlQuery taskCountQuery(m_database);
    QSqlQuery subtaskCountQuery(m_database);
    if (!taskCountQuery.exec(QStringLiteral("SELECT COUNT(*) FROM tasks WHERE completed=1 AND experience_awarded=0"))
        || !taskCountQuery.next()
        || !subtaskCountQuery.exec(QStringLiteral("SELECT COUNT(*) FROM subtasks WHERE completed=1 AND experience_awarded=0"))
        || !subtaskCountQuery.next()) return rollback(QStringLiteral("Unable to count unrecorded progress"));
    const int taskCount = taskCountQuery.value(0).toInt();
    const int subtaskCount = subtaskCountQuery.value(0).toInt();
    awardedXp = taskCount * qMax(0, taskXp) + subtaskCount * qMax(0, subtaskXp);
    const int restoredVitality = taskCount * qMax(0, taskVitality) + subtaskCount * qMax(0, subtaskVitality);

    QSqlQuery update(m_database);
    if (!update.exec(QStringLiteral("UPDATE tasks SET experience_awarded=1 WHERE completed=1 AND experience_awarded=0"))
        || !update.exec(QStringLiteral("UPDATE subtasks SET experience_awarded=1 WHERE completed=1 AND experience_awarded=0")))
        return rollback(update.lastError().text());
    if (awardedXp > 0 && !addExperienceToProfile(awardedXp, restoredVitality)) return rollback(m_lastError);
    if (!m_database.commit()) return rollback(m_database.lastError().text());
    return true;
}

bool DatabaseManager::addExperienceToProfile(const int xp, const int vitality)
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT progress_days,today_xp,last_progress_date,vitality FROM growth_profile WHERE id=1"))
        || !query.next()) {
        setError(tr("Unable to update your garden."), query.lastError().text());
        return false;
    }
    const QDate today = QDate::currentDate();
    const QDate lastProgress = QDate::fromString(query.value(2).toString(), Qt::ISODate);
    const bool firstProgressToday = lastProgress != today;
    const int progressDays = query.value(0).toInt() + (firstProgressToday ? 1 : 0);
    const int todayXp = (firstProgressToday ? 0 : query.value(1).toInt()) + xp;
    const int restoredVitality = qBound(0, query.value(3).toInt() + vitality, 100);

    query.prepare(QStringLiteral("UPDATE growth_profile SET total_xp=total_xp+?,progress_days=?,today_xp=?,last_progress_date=?,vitality=?,vitality_updated_date=? WHERE id=1"));
    query.addBindValue(xp);
    query.addBindValue(progressDays);
    query.addBindValue(todayXp);
    query.addBindValue(today.toString(Qt::ISODate));
    query.addBindValue(restoredVitality);
    query.addBindValue(today.toString(Qt::ISODate));
    if (query.exec()) return true;
    setError(tr("Unable to update your garden."), query.lastError().text());
    return false;
}

bool DatabaseManager::applyVitalityDecay()
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT vitality,vitality_updated_date,last_progress_date FROM growth_profile WHERE id=1"))
        || !query.next()) {
        setError(tr("Unable to update your garden."), query.lastError().text());
        return false;
    }
    const QDate today = QDate::currentDate();
    const QDate updatedDate = QDate::fromString(query.value(1).toString(), Qt::ISODate);
    const QDate lastProgressDate = QDate::fromString(query.value(2).toString(), Qt::ISODate);
    const int elapsedDays = updatedDate.isValid() ? qMax(0, updatedDate.daysTo(today)) : 0;
    const int vitality = qMax(0, query.value(0).toInt() - elapsedDays * 15);
    const int todayXp = lastProgressDate == today ? -1 : 0;

    query.prepare(todayXp < 0
        ? QStringLiteral("UPDATE growth_profile SET vitality=?,vitality_updated_date=? WHERE id=1")
        : QStringLiteral("UPDATE growth_profile SET vitality=?,vitality_updated_date=?,today_xp=0 WHERE id=1"));
    query.addBindValue(vitality);
    query.addBindValue(today.toString(Qt::ISODate));
    if (query.exec()) return true;
    setError(tr("Unable to update your garden."), query.lastError().text());
    return false;
}

bool DatabaseManager::ensureColumn(const QString &table, const QString &column, const QString &definition)
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA table_info(%1)").arg(table))) {
        setError(tr("Unable to initialize local storage."), query.lastError().text());
        return false;
    }
    while (query.next()) if (query.value(1).toString() == column) return true;
    return execute(QStringLiteral("ALTER TABLE %1 ADD COLUMN %2 %3").arg(table, column, definition));
}

bool DatabaseManager::execute(const QString &sql) const
{
    QSqlQuery query(m_database);
    if (query.exec(sql)) return true;
    setError(tr("Unable to initialize local storage."), query.lastError().text());
    return false;
}

void DatabaseManager::setError(const QString &context, const QString &technical) const
{
    m_lastError = context;
    qCCritical(databaseLog).noquote() << context << technical;
}
