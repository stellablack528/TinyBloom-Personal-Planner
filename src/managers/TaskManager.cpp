#include "TaskManager.h"

#include <QDateTime>
#include <QSet>

#include <algorithm>

TaskManager::TaskManager(DatabaseManager *database, QObject *parent)
    : QObject(parent), m_database(database)
{
    m_all.setScope(TaskListModel::Scope::All);
    m_today.setScope(TaskListModel::Scope::Today);
    m_tomorrow.setScope(TaskListModel::Scope::Tomorrow);
    m_later.setScope(TaskListModel::Scope::Later);
    m_longTerm.setScope(TaskListModel::Scope::LongTerm);
    m_all.setSource(&m_tasks);
    m_today.setSource(&m_tasks);
    m_tomorrow.setSource(&m_tasks);
    m_later.setSource(&m_tasks);
    m_longTerm.setSource(&m_tasks);
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
TaskListModel *TaskManager::longTermTasks() { return &m_longTerm; }
int TaskManager::totalCount() const
{
    int count = 0;
    for (const auto &task : m_tasks) if (!task.longTerm) ++count;
    return count;
}
int TaskManager::todayCount() const
{
    int count = 0;
    const auto today = QDate::currentDate();
    for (const auto &task : m_tasks) if (!task.longTerm && task.dueDate == today) ++count;
    return count;
}
int TaskManager::todayCompletedCount() const
{
    int count = 0;
    const auto today = QDate::currentDate();
    for (const auto &task : m_tasks) if (!task.longTerm && task.dueDate == today && task.completed) ++count;
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

bool TaskManager::createLongTermTask(const QString &title, const QString &description,
    const QString &targetDate)
{
    if (!validate(title, description, 0)) return false;
    const QString normalizedDate = targetDate.trimmed();
    if (!normalizedDate.isEmpty() && !QDate::fromString(normalizedDate, Qt::ISODate).isValid())
        return fail(tr("Use a valid target date in YYYY-MM-DD format."));
    Task task;
    task.title = title.trimmed();
    task.description = description.trimmed();
    task.dueDate = QDate::fromString(normalizedDate, Qt::ISODate);
    task.estimatedMinutes = 0;
    task.longTerm = true;
    task.createdAt = task.updatedAt = QDateTime::currentDateTimeUtc();
    if (!m_database->insertTask(task)) return fail(m_database->lastError());
    m_tasks.prepend(task);
    refreshModels();
    return true;
}

bool TaskManager::updateLongTermTask(const qint64 id, const QString &title,
    const QString &description, const QString &targetDate)
{
    if (!validate(title, description, 0)) return false;
    const QString normalizedDate = targetDate.trimmed();
    if (!normalizedDate.isEmpty() && !QDate::fromString(normalizedDate, Qt::ISODate).isValid())
        return fail(tr("Use a valid target date in YYYY-MM-DD format."));
    Task *task = findTask(id);
    if (!task || !task->longTerm) return fail(tr("This long-term task no longer exists."));
    Task updated = *task;
    updated.title = title.trimmed();
    updated.description = description.trimmed();
    updated.dueDate = QDate::fromString(normalizedDate, Qt::ISODate);
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

bool TaskManager::createSubtask(const qint64 taskId, const QString &title, const qint64 parentId)
{
    Task *task = findTask(taskId);
    const QString trimmed = title.trimmed();
    if (!task) return fail(tr("This task no longer exists."));
    if (trimmed.isEmpty() || trimmed.size() > 160) return fail(tr("A small step needs a title under 160 characters."));
    if (parentId > 0) {
        const auto parent = std::find_if(task->subtasks.cbegin(), task->subtasks.cend(),
            [parentId](const Subtask &item) { return item.id == parentId; });
        if (parent == task->subtasks.cend()) return fail(tr("This branch no longer exists."));
        if (parent->parentId > 0) return fail(tr("The task map supports two levels of steps."));
    }
    Subtask subtask{0, taskId, trimmed, false, QDateTime::currentDateTimeUtc()};
    subtask.parentId = parentId;
    if (!m_database->insertSubtask(subtask)) return fail(m_database->lastError());
    task->subtasks.append(subtask);
    if (!updateLongTermCompletion(*task)) return false;
    refreshModels();
    return true;
}

bool TaskManager::updateSubtask(const qint64 taskId, const qint64 subtaskId, const QString &title)
{
    Task *task = findTask(taskId);
    const QString trimmed = title.trimmed();
    if (!task) return fail(tr("This task no longer exists."));
    if (trimmed.isEmpty() || trimmed.size() > 160) return fail(tr("A small step needs a title under 160 characters."));
    for (Subtask &subtask : task->subtasks) {
        if (subtask.id != subtaskId) continue;
        Subtask updated = subtask;
        updated.title = trimmed;
        if (!m_database->updateSubtask(updated)) return fail(m_database->lastError());
        subtask = updated;
        refreshModels();
        return true;
    }
    return fail(tr("This small step no longer exists."));
}

bool TaskManager::deleteSubtask(const qint64 taskId, const qint64 subtaskId)
{
    Task *task = findTask(taskId);
    if (!task) return fail(tr("This task no longer exists."));
    QVector<qint64> ids{subtaskId};
    for (const Subtask &subtask : task->subtasks)
        if (subtask.parentId == subtaskId) ids.append(subtask.id);
    for (auto it = ids.crbegin(); it != ids.crend(); ++it)
        if (!m_database->deleteSubtask(*it)) return fail(m_database->lastError());
    task->subtasks.removeIf([&ids](const Subtask &s) { return ids.contains(s.id); });
    if (!updateLongTermCompletion(*task)) return false;
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
        if (!updateLongTermCompletion(*task)) return false;
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
    for (const auto &s : task->subtasks) subtasks.append(QVariantMap{{"id",s.id},{"title",s.title},
        {"completed",s.completed},{"parentId",s.parentId}});
    return {{"id",task->id},{"title",task->title},{"description",task->description},
        {"completed",task->completed},{"priority",static_cast<int>(task->priority)},
        {"dueDate",task->dueDate.toString(Qt::ISODate)},{"estimatedMinutes",task->estimatedMinutes},
        {"category",task->category},{"subtasks",subtasks},{"longTerm",task->longTerm}};
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
    m_all.refresh(); m_today.refresh(); m_tomorrow.refresh(); m_later.refresh(); m_longTerm.refresh();
    emit tasksChanged();
    emit statisticsChanged();
}

bool TaskManager::updateLongTermCompletion(Task &task)
{
    if (!task.longTerm) return true;
    QSet<qint64> parents;
    for (const Subtask &subtask : task.subtasks)
        if (subtask.parentId > 0) parents.insert(subtask.parentId);
    int leafCount = 0;
    int completedLeafCount = 0;
    for (const Subtask &subtask : task.subtasks) {
        if (parents.contains(subtask.id)) continue;
        ++leafCount;
        if (subtask.completed) ++completedLeafCount;
    }
    const bool completed = leafCount > 0 && leafCount == completedLeafCount;
    if (task.completed == completed) return true;
    Task updated = task;
    updated.completed = completed;
    updated.completedAt = completed ? QDateTime::currentDateTimeUtc() : QDateTime{};
    updated.updatedAt = QDateTime::currentDateTimeUtc();
    if (!m_database->updateTask(updated)) return fail(m_database->lastError());
    task = updated;
    if (completed) emit taskCompleted(task.id, task.title);
    return true;
}

bool TaskManager::fail(const QString &message)
{
    m_lastError = message;
    emit errorOccurred(message);
    return false;
}
