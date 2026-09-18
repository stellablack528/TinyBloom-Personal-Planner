#pragma once

#include "models/Task.h"

#include <QJsonObject>
#include <QObject>
#include <QSqlDatabase>

class DatabaseManager final : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    bool initializeDatabase(const QString &overridePath = {});
    void closeDatabase();
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] QString databasePath() const;
    [[nodiscard]] QString lastError() const;

    [[nodiscard]] QVector<Task> loadTasks() const;
    bool insertTask(Task &task);
    bool updateTask(const Task &task);
    bool deleteTask(qint64 taskId);
    bool insertSubtask(Subtask &subtask);
    bool updateSubtask(const Subtask &subtask);
    bool deleteSubtask(qint64 subtaskId);

    [[nodiscard]] QString setting(const QString &key, const QString &fallback = {}) const;
    bool setSetting(const QString &key, const QString &value);
    [[nodiscard]] QJsonObject exportObject() const;
    bool importObject(const QJsonObject &root);

private:
    bool openDatabase(const QString &path);
    bool createTables();
    bool execute(const QString &sql) const;
    void setError(const QString &context, const QString &technical) const;

    QString m_connectionName;
    QString m_databasePath;
    QSqlDatabase m_database;
    mutable QString m_lastError;
};

