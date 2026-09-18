#include "TaskListModel.h"

#include <QDate>
#include <QVariantList>

TaskListModel::TaskListModel(QObject *parent) : QAbstractListModel(parent) {}

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

int TaskListModel::count() const { return m_rows.size(); }

QVariant TaskListModel::data(const QModelIndex &index, const int role) const
{
    if (!m_tasks || !index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) return {};
    const Task &task = m_tasks->at(m_rows.at(index.row()));
    switch (role) {
    case IdRole: return task.id;
    case TitleRole: return task.title;
    case DescriptionRole: return task.description;
    case CompletedRole: return task.completed;
    case PriorityRole: return static_cast<int>(task.priority);
    case PriorityNameRole: return priorityName(task.priority);
    case DueDateRole: return task.dueDate;
    case EstimatedMinutesRole: return task.estimatedMinutes;
    case CategoryRole: return task.category;
    case CreatedAtRole: return task.createdAt;
    case UpdatedAtRole: return task.updatedAt;
    case CompletedAtRole: return task.completedAt;
    case SubtaskCountRole: return task.subtasks.size();
    case CompletedSubtaskCountRole: {
        int count = 0;
        for (const auto &subtask : task.subtasks) if (subtask.completed) ++count;
        return count;
    }
    case SubtasksRole: {
        QVariantList list;
        for (const auto &subtask : task.subtasks) {
            list.append(QVariantMap{{"id", subtask.id}, {"taskId", subtask.taskId},
                {"title", subtask.title}, {"completed", subtask.completed}});
        }
        return list;
    }
    default: return {};
    }
}

QHash<int, QByteArray> TaskListModel::roleNames() const
{
    return {{IdRole,"taskId"}, {TitleRole,"title"}, {DescriptionRole,"description"},
        {CompletedRole,"completed"}, {PriorityRole,"priority"}, {PriorityNameRole,"priorityName"},
        {DueDateRole,"dueDate"}, {EstimatedMinutesRole,"estimatedMinutes"}, {CategoryRole,"category"},
        {CreatedAtRole,"createdAt"}, {UpdatedAtRole,"updatedAt"}, {CompletedAtRole,"completedAt"},
        {SubtasksRole,"subtasks"}, {SubtaskCountRole,"subtaskCount"},
        {CompletedSubtaskCountRole,"completedSubtaskCount"}};
}

void TaskListModel::setSource(const QVector<Task> *tasks) { m_tasks = tasks; refresh(); }
void TaskListModel::setScope(const Scope scope) { m_scope = scope; refresh(); }
void TaskListModel::setSearchText(const QString &text) { m_searchText = text.trimmed(); refresh(); }
void TaskListModel::setStatusFilter(const int status)
{
    m_status = status == 1 ? Status::Active : status == 2 ? Status::Completed : Status::Any;
    refresh();
}

void TaskListModel::refresh()
{
    beginResetModel();
    m_rows.clear();
    if (m_tasks) {
        for (int i = 0; i < m_tasks->size(); ++i) if (matches(m_tasks->at(i))) m_rows.append(i);
    }
    endResetModel();
    emit countChanged();
}

const Task *TaskListModel::taskAt(const int row) const
{
    if (!m_tasks || row < 0 || row >= m_rows.size()) return nullptr;
    return &m_tasks->at(m_rows.at(row));
}

bool TaskListModel::matches(const Task &task) const
{
    if (m_status == Status::Active && task.completed) return false;
    if (m_status == Status::Completed && !task.completed) return false;
    const QDate today = QDate::currentDate();
    if (m_scope == Scope::Today && task.dueDate != today) return false;
    if (m_scope == Scope::Tomorrow && task.dueDate != today.addDays(1)) return false;
    if (m_scope == Scope::Later && task.dueDate.isValid() && task.dueDate <= today.addDays(1)) return false;
    if (!m_searchText.isEmpty()) {
        const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
        if (!task.title.contains(m_searchText, cs) && !task.description.contains(m_searchText, cs)
            && !task.category.contains(m_searchText, cs)) return false;
    }
    return true;
}
