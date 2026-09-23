#include "TaskListModel.h"

#include <QDate>
#include <QSet>
#include <QVariantList>

TaskListModel::TaskListModel(QObject *parent) : QAbstractListModel(parent) {}

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_ids.size();
}

int TaskListModel::count() const { return m_ids.size(); }

QVariant TaskListModel::data(const QModelIndex &index, const int role) const
{
    if (!m_tasks || !index.isValid() || index.row() < 0 || index.row() >= m_ids.size()) return {};
    const Task *taskPointer = taskById(m_ids.at(index.row()));
    if (!taskPointer) return {};
    const Task &task = *taskPointer;
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
    case LongTermRole: return task.longTerm;
    case LeafCountRole:
    case CompletedLeafCountRole: {
        QSet<qint64> parents;
        for (const auto &subtask : task.subtasks) if (subtask.parentId > 0) parents.insert(subtask.parentId);
        int leaves = 0;
        int completedLeaves = 0;
        for (const auto &subtask : task.subtasks) {
            if (parents.contains(subtask.id)) continue;
            ++leaves;
            if (subtask.completed) ++completedLeaves;
        }
        return role == LeafCountRole ? leaves : completedLeaves;
    }
    case SubtasksRole: {
        QVariantList list;
        for (const auto &subtask : task.subtasks) {
            list.append(QVariantMap{{"id", subtask.id}, {"taskId", subtask.taskId},
                {"title", subtask.title}, {"completed", subtask.completed},
                {"parentId", subtask.parentId}});
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
        {CompletedSubtaskCountRole,"completedSubtaskCount"}, {LongTermRole,"longTerm"},
        {LeafCountRole,"leafCount"}, {CompletedLeafCountRole,"completedLeafCount"}};
}

void TaskListModel::setSource(const QVector<Task> *tasks) { m_tasks = tasks; refresh(); }
void TaskListModel::setScope(const Scope scope) { m_scope = scope; refresh(); }
void TaskListModel::setSearchText(const QString &text) { m_searchText = text.trimmed(); refresh(); }
void TaskListModel::setStatusFilter(const int status)
{
    m_status = status == 1 ? Status::Active : status == 2 ? Status::Completed : Status::Any;
    refresh();
}

QVariantMap TaskListModel::get(const int row) const
{
    const Task *task = taskAt(row);
    if (!task) return {};
    QVariantMap result;
    const auto names = roleNames();
    const QModelIndex modelIndex = index(row, 0);
    for (auto it = names.cbegin(); it != names.cend(); ++it)
        result.insert(QString::fromUtf8(it.value()), data(modelIndex, it.key()));
    return result;
}

void TaskListModel::refresh()
{
    QVector<qint64> nextIds;
    if (m_tasks) {
        for (const Task &task : *m_tasks) if (matches(task)) nextIds.append(task.id);
    }

    const int previousCount = m_ids.size();
    const QSet<qint64> nextSet(nextIds.cbegin(), nextIds.cend());
    for (int row = m_ids.size() - 1; row >= 0; --row) {
        if (nextSet.contains(m_ids.at(row))) continue;
        beginRemoveRows({}, row, row);
        m_ids.removeAt(row);
        endRemoveRows();
    }

    for (int targetRow = 0; targetRow < nextIds.size(); ++targetRow) {
        const qint64 id = nextIds.at(targetRow);
        if (targetRow < m_ids.size() && m_ids.at(targetRow) == id) continue;
        const int currentRow = m_ids.indexOf(id, targetRow + 1);
        if (currentRow >= 0) {
            beginMoveRows({}, currentRow, currentRow, {}, targetRow);
            m_ids.move(currentRow, targetRow);
            endMoveRows();
        } else {
            beginInsertRows({}, targetRow, targetRow);
            m_ids.insert(targetRow, id);
            endInsertRows();
        }
    }

    if (!m_ids.isEmpty()) emit dataChanged(index(0), index(m_ids.size() - 1));
    if (previousCount != m_ids.size()) emit countChanged();
}

const Task *TaskListModel::taskAt(const int row) const
{
    if (!m_tasks || row < 0 || row >= m_ids.size()) return nullptr;
    return taskById(m_ids.at(row));
}

const Task *TaskListModel::taskById(const qint64 id) const
{
    if (!m_tasks) return nullptr;
    for (const Task &task : *m_tasks) if (task.id == id) return &task;
    return nullptr;
}

bool TaskListModel::matches(const Task &task) const
{
    if (m_scope == Scope::LongTerm) {
        if (!task.longTerm) return false;
    } else if (task.longTerm) {
        return false;
    }
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
