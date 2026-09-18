#include "DatabaseManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
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
    return execute(QStringLiteral(R"(
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
            completed_at TEXT
        ))"))
        && execute(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS subtasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            task_id INTEGER NOT NULL,
            title TEXT NOT NULL CHECK(length(title) BETWEEN 1 AND 160),
            completed INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL,
            FOREIGN KEY(task_id) REFERENCES tasks(id) ON DELETE CASCADE
        ))"))
        && execute(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        ))"))
        && execute(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date)"))
        && execute(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_subtasks_task_id ON subtasks(task_id)"));
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

QVector<Task> DatabaseManager::loadTasks() const
{
    QVector<Task> tasks;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT id,title,description,completed,priority,due_date,estimated_minutes,category,created_at,updated_at,completed_at FROM tasks ORDER BY completed ASC, due_date IS NULL, due_date ASC, created_at DESC"))) {
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
        tasks.append(task);
    }

    QHash<qint64, int> taskIndexes;
    for (int i = 0; i < tasks.size(); ++i) taskIndexes.insert(tasks[i].id, i);
    QSqlQuery subquery(m_database);
    if (!subquery.exec(QStringLiteral("SELECT id,task_id,title,completed,created_at FROM subtasks ORDER BY id ASC"))) {
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
        tasks[taskIndexes.value(taskId)].subtasks.append(subtask);
    }
    return tasks;
}

bool DatabaseManager::insertTask(Task &task)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("INSERT INTO tasks(title,description,completed,priority,due_date,estimated_minutes,category,created_at,updated_at,completed_at) VALUES(?,?,?,?,?,?,?,?,?,?)"));
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
    query.prepare(QStringLiteral("UPDATE tasks SET title=?,description=?,completed=?,priority=?,due_date=?,estimated_minutes=?,category=?,updated_at=?,completed_at=? WHERE id=?"));
    query.addBindValue(task.title);
    query.addBindValue(task.description.isNull() ? QStringLiteral("") : task.description);
    query.addBindValue(task.completed);
    query.addBindValue(static_cast<int>(task.priority));
    query.addBindValue(task.dueDate.isValid() ? task.dueDate.toString(Qt::ISODate) : QVariant{});
    query.addBindValue(task.estimatedMinutes);
    query.addBindValue(task.category.isNull() ? QStringLiteral("") : task.category);
    query.addBindValue(iso(task.updatedAt));
    query.addBindValue(task.completedAt.isValid() ? iso(task.completedAt) : QVariant{});
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
    query.prepare(QStringLiteral("INSERT INTO subtasks(task_id,title,completed,created_at) VALUES(?,?,?,?)"));
    query.addBindValue(subtask.taskId);
    query.addBindValue(subtask.title);
    query.addBindValue(subtask.completed);
    query.addBindValue(iso(subtask.createdAt));
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
            {"completedAt", iso(task.completedAt)}});
        for (const Subtask &subtask : task.subtasks) {
            subtasksArray.append(QJsonObject{{"id", subtask.id}, {"taskId", subtask.taskId}, {"title", subtask.title},
                {"completed", subtask.completed}, {"createdAt", iso(subtask.createdAt)}});
        }
    }
    QJsonObject settings;
    QSqlQuery query(m_database);
    if (query.exec(QStringLiteral("SELECT key,value FROM settings"))) {
        while (query.next()) settings.insert(query.value(0).toString(), query.value(1).toString());
    }
    return QJsonObject{{"version", QStringLiteral("0.1.1")},
        {"exportedAt", iso(QDateTime::currentDateTimeUtc())}, {"tasks", tasksArray},
        {"subtasks", subtasksArray}, {"settings", settings}};
}

bool DatabaseManager::importObject(const QJsonObject &root)
{
    if (!root.value("version").isString() || !root.value("tasks").isArray()
        || !root.value("subtasks").isArray() || !root.value("settings").isObject()) {
        setError(tr("This file is not a valid TinyBloom export."), QStringLiteral("Missing required JSON fields"));
        return false;
    }
    const QJsonArray tasks = root.value("tasks").toArray();
    for (const auto &value : tasks) {
        const QJsonObject item = value.toObject();
        if (!value.isObject() || item.value("title").toString().trimmed().isEmpty()) {
            setError(tr("This file contains an invalid task."), QStringLiteral("Task title missing"));
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
        if (!insertTask(task)) return rollback(m_lastError);
        ids.insert(oldId, task.id);
    }
    for (const auto &value : root.value("subtasks").toArray()) {
        const QJsonObject item = value.toObject();
        const qint64 parentId = ids.value(item.value("taskId").toInteger(), 0);
        const QString title = item.value("title").toString().trimmed();
        if (!value.isObject() || parentId == 0 || title.isEmpty()) return rollback(QStringLiteral("Invalid subtask"));
        Subtask subtask{0, parentId, title.left(160), item.value("completed").toBool(),
            QDateTime::fromString(item.value("createdAt").toString(), Qt::ISODateWithMs)};
        if (!subtask.createdAt.isValid()) subtask.createdAt = QDateTime::currentDateTimeUtc();
        if (!insertSubtask(subtask)) return rollback(m_lastError);
    }
    const QJsonObject settings = root.value("settings").toObject();
    for (auto it = settings.begin(); it != settings.end(); ++it) {
        if (!it.value().isString() || !setSetting(it.key().left(80), it.value().toString().left(500)))
            return rollback(m_lastError);
    }
    if (!m_database.commit()) return rollback(m_database.lastError().text());
    return true;
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
