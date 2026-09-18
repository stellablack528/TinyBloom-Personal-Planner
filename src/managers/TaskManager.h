#pragma once

#include "database/DatabaseManager.h"
#include "models/TaskListModel.h"

#include <QObject>

class TaskManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(TaskListModel *allTasks READ allTasks CONSTANT)
    Q_PROPERTY(TaskListModel *todayTasks READ todayTasks CONSTANT)
    Q_PROPERTY(TaskListModel *tomorrowTasks READ tomorrowTasks CONSTANT)
    Q_PROPERTY(TaskListModel *laterTasks READ laterTasks CONSTANT)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY statisticsChanged)
    Q_PROPERTY(int todayCount READ todayCount NOTIFY statisticsChanged)
    Q_PROPERTY(int todayCompletedCount READ todayCompletedCount NOTIFY statisticsChanged)
    Q_PROPERTY(double todayProgress READ todayProgress NOTIFY statisticsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)

public:
    explicit TaskManager(DatabaseManager *database, QObject *parent = nullptr);
    bool initialize();

    TaskListModel *allTasks();
    TaskListModel *todayTasks();
    TaskListModel *tomorrowTasks();
    TaskListModel *laterTasks();
    int totalCount() const;
    int todayCount() const;
    int todayCompletedCount() const;
    double todayProgress() const;
    QString lastError() const;

    Q_INVOKABLE bool createTask(const QString &title, const QString &description = {},
        const QString &dueDate = {}, int priority = 1, int estimatedMinutes = 25,
        const QString &category = {});
    Q_INVOKABLE bool updateTask(qint64 id, const QString &title, const QString &description,
        const QString &dueDate, int priority, int estimatedMinutes, const QString &category);
    Q_INVOKABLE bool deleteTask(qint64 id);
    Q_INVOKABLE bool toggleTask(qint64 id);
    Q_INVOKABLE bool createSubtask(qint64 taskId, const QString &title);
    Q_INVOKABLE bool deleteSubtask(qint64 taskId, qint64 subtaskId);
    Q_INVOKABLE bool toggleSubtask(qint64 taskId, qint64 subtaskId);
    Q_INVOKABLE QVariantMap getTask(qint64 id) const;
    Q_INVOKABLE void searchTasks(const QString &text);
    Q_INVOKABLE void filterTasks(int status);
    Q_INVOKABLE void setTaskScope(int scope);
    Q_INVOKABLE void reload();

signals:
    void tasksChanged();
    void statisticsChanged();
    void errorOccurred(const QString &message);

private:
    Task *findTask(qint64 id);
    const Task *findTask(qint64 id) const;
    bool validate(const QString &title, const QString &description, int estimatedMinutes);
    void refreshModels();
    bool fail(const QString &message);

    DatabaseManager *m_database;
    QVector<Task> m_tasks;
    TaskListModel m_all;
    TaskListModel m_today;
    TaskListModel m_tomorrow;
    TaskListModel m_later;
    QString m_lastError;
};
