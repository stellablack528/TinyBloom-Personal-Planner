#include "TaskManager.h"

#include <QDateTime>

TaskManager::TaskManager(DatabaseManager *database, QObject *parent)
    : QObject(parent), m_database(database)
{
    m_all.setScope(TaskListModel::Scope::All);
    m_today.setScope(TaskListModel::Scope::Today);
    m_tomorrow.setScope(TaskListModel::Scope::Tomorrow);
    m_later.setScope(TaskListModel::Scope::Later);
    m_all.setSource(&m_tasks);
    m_today.setSource(&m_tasks);
    m_tomorrow.setSource(&m_tasks);
    m_later.setSource(&m_tasks);
}

bool TaskManager::initialize()
{
    if (!m_database->isOpen()) return fail(tr("Local storage is not available."));
    reload();
    return true;
}

TaskListModel *TaskManager::allTasks() { return &m_all; }
TaskListModel *TaskManager::todayTasks() { return &m_today; }
TaskListModel *TaskManager::tomorrowTasks() { return &m_tomorrow; }
TaskListModel *TaskManager::laterTasks() { return &m_later; }
int TaskManager::totalCount() const { return m_tasks.size(); }
int TaskManager::todayCount() const
{
    int count = 0;
    const auto today = QDate::currentDate();
    for (const auto &task : m_tasks) if (task.dueDate == today) ++count;
    return count;
}
int TaskManager::todayCompletedCount() const
{
    int count = 0;
    const auto today = QDate::currentDate();
    for (const auto &task : m_tasks) if (task.dueDate == today && task.completed) ++count;
    return count;
}
double TaskManager::todayProgress() const
{
    return todayCount() == 0 ? 0.0 : static_cast<double>(todayCompletedCount()) / todayCount();
}
QString TaskManager::lastError() const { return m_lastError; }

bool TaskManager::createTask(const QString &title, const QString &description, const QString &dueDate,
    const int priority, const int estimatedMinutes, const QString &category)
{
    if (!validate(title, description, estimatedMinutes)) return false;
    const QString normalizedDate = dueDate.trimmed();
    if (!normalizedDate.isEmpty() && !QDate::fromString(normalizedDate, Qt::ISODate).isValid())
        return fail(tr("Use a valid due date in YYYY-MM-DD format."));
    Task task;
    task.title = title.trimmed();
    task.description = description.trimmed();
    task.priority = priorityFromInt(priority);
    task.dueDate = QDate::fromString(normalizedDate, Qt::ISODate);
    task.estimatedMinutes = estimatedMinutes;
    task.category = category.trimmed().left(80);
    task.createdAt = task.updatedAt = QDateTime::currentDateTimeUtc();
    if (!m_database->insertTask(task)) return fail(m_database->lastError());
    m_tasks.prepend(task);
    refreshModels();
    return true;
}

bool TaskManager::updateTask(const qint64 id, const QString &title, const QString &description,
    const QString &dueDate, const int priority, const int estimatedMinutes, const QString &category)
{
    if (!validate(title, description, estimatedMinutes)) return false;
    const QString normalizedDate = dueDate.trimmed();
    if (!normalizedDate.isEmpty() && !QDate::fromString(normalizedDate, Qt::ISODate).isValid())
        return fail(tr("Use a valid due date in YYYY-MM-DD format."));
    Task *task = findTask(id);
    if (!task) return fail(tr("This task no longer exists."));
    Task updated = *task;
    updated.title = title.trimmed();
    updated.description = description.trimmed();
    updated.dueDate = QDate::fromString(normalizedDate, Qt::ISODate);
    updated.priority = priorityFromInt(priority);
    updated.estimatedMinutes = estimatedMinutes;
    updated.category = category.trimmed().left(80);
    updated.updatedAt = QDateTime::currentDateTimeUtc();
    if (!m_database->updateTask(updated)) return fail(m_database->lastError());
    *task = updated;
    refreshModels();
    return true;
}

bool TaskManager::deleteTask(const qint64 id)
{
    if (!findTask(id)) return fail(tr("This task no longer exists."));
    if (!m_database->deleteTask(id)) return fail(m_database->lastError());
    m_tasks.removeIf([id](const Task &task) { return task.id == id; });
    refreshModels();
    return true;
}

bool TaskManager::toggleTask(const qint64 id)
{
    Task *task = findTask(id);
    if (!task) return fail(tr("This task no longer exists."));
    Task updated = *task;
    updated.completed = !updated.completed;
    updated.completedAt = updated.completed ? QDateTime::currentDateTimeUtc() : QDateTime{};
    updated.updatedAt = QDateTime::currentDateTimeUtc();
    if (!m_database->updateTask(updated)) return fail(m_database->lastError());
    *task = updated;
    refreshModels();
    if (updated.completed) emit taskCompleted(updated.id, updated.title);
    return true;
}

bool TaskManager::createSubtask(const qint64 taskId, const QString &title)
{
    Task *task = findTask(taskId);
    const QString trimmed = title.trimmed();
    if (!task) return fail(tr("This task no longer exists."));
    if (trimmed.isEmpty() || trimmed.size() > 160) return fail(tr("A small step needs a title under 160 characters."));
    Subtask subtask{0, taskId, trimmed, false, QDateTime::currentDateTimeUtc()};
    if (!m_database->insertSubtask(subtask)) return fail(m_database->lastError());
    task->subtasks.append(subtask);
    refreshModels();
    return true;
}

bool TaskManager::deleteSubtask(const qint64 taskId, const qint64 subtaskId)
{
    Task *task = findTask(taskId);
    if (!task) return fail(tr("This task no longer exists."));
    if (!m_database->deleteSubtask(subtaskId)) return fail(m_database->lastError());
    task->subtasks.removeIf([subtaskId](const Subtask &s) { return s.id == subtaskId; });
    refreshModels();
    return true;
}

bool TaskManager::toggleSubtask(const qint64 taskId, const qint64 subtaskId)
{
    Task *task = findTask(taskId);
    if (!task) return fail(tr("This task no longer exists."));
    for (Subtask &subtask : task->subtasks) {
        if (subtask.id != subtaskId) continue;
        Subtask updated = subtask;
        updated.completed = !updated.completed;
        if (!m_database->updateSubtask(updated)) return fail(m_database->lastError());
        subtask = updated;
        refreshModels();
        if (updated.completed) emit subtaskCompleted(updated.id, updated.title);
        return true;
    }
    return fail(tr("This small step no longer exists."));
}

QVariantMap TaskManager::getTask(const qint64 id) const
{
    const Task *task = findTask(id);
    if (!task) return {};
    QVariantList subtasks;
    for (const auto &s : task->subtasks) subtasks.append(QVariantMap{{"id",s.id},{"title",s.title},{"completed",s.completed}});
    return {{"id",task->id},{"title",task->title},{"description",task->description},
        {"completed",task->completed},{"priority",static_cast<int>(task->priority)},
        {"dueDate",task->dueDate.toString(Qt::ISODate)},{"estimatedMinutes",task->estimatedMinutes},
        {"category",task->category},{"subtasks",subtasks}};
}

void TaskManager::searchTasks(const QString &text) { m_all.setSearchText(text); }
void TaskManager::filterTasks(const int status) { m_all.setStatusFilter(status); }
void TaskManager::setTaskScope(const int scope)
{
    m_all.setScope(scope == 1 ? TaskListModel::Scope::Today
        : scope == 2 ? TaskListModel::Scope::Tomorrow
        : scope == 3 ? TaskListModel::Scope::Later
        : TaskListModel::Scope::All);
}
void TaskManager::reload() { m_tasks = m_database->loadTasks(); refreshModels(); }
void TaskManager::retranslate() { refreshModels(); }

Task *TaskManager::findTask(const qint64 id)
{
    for (Task &task : m_tasks) if (task.id == id) return &task;
    return nullptr;
}
const Task *TaskManager::findTask(const qint64 id) const
{
    for (const Task &task : m_tasks) if (task.id == id) return &task;
    return nullptr;
}

bool TaskManager::validate(const QString &title, const QString &description, const int estimatedMinutes)
{
    if (title.trimmed().isEmpty()) return fail(tr("Please give this task a name."));
    if (title.trimmed().size() > 160) return fail(tr("Task names can be up to 160 characters."));
    if (description.size() > 4000) return fail(tr("Descriptions can be up to 4,000 characters."));
    if (estimatedMinutes < 0 || estimatedMinutes > 10080) return fail(tr("Choose an estimate between 0 and 10,080 minutes."));
    return true;
}

void TaskManager::refreshModels()
{
    m_all.refresh(); m_today.refresh(); m_tomorrow.refresh(); m_later.refresh();
    emit tasksChanged();
    emit statisticsChanged();
}

bool TaskManager::fail(const QString &message)
{
    m_lastError = message;
    emit errorOccurred(message);
    return false;
}
