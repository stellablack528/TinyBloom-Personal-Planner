#pragma once

#include "Task.h"

#include <QAbstractListModel>

class TaskListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        IdRole = Qt::UserRole + 1, TitleRole, DescriptionRole, CompletedRole,
        PriorityRole, PriorityNameRole, DueDateRole, EstimatedMinutesRole,
        CategoryRole, CreatedAtRole, UpdatedAtRole, CompletedAtRole,
        SubtasksRole, SubtaskCountRole, CompletedSubtaskCountRole,
        LongTermRole, LeafCountRole, CompletedLeafCountRole
    };
    Q_ENUM(Role)

    enum class Scope { All, Today, Tomorrow, Later, LongTerm };
    enum class Status { Any, Active, Completed };

    explicit TaskListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = {}) const override;
    int count() const;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setSource(const QVector<Task> *tasks);
    void setScope(Scope scope);
    Q_INVOKABLE void setSearchText(const QString &text);
    Q_INVOKABLE void setStatusFilter(int status);
    Q_INVOKABLE QVariantMap get(int row) const;
    void refresh();
    [[nodiscard]] const Task *taskAt(int row) const;

signals:
    void countChanged();

private:
    bool matches(const Task &task) const;
    [[nodiscard]] const Task *taskById(qint64 id) const;

    const QVector<Task> *m_tasks = nullptr;
    QVector<qint64> m_ids;
    Scope m_scope = Scope::All;
    Status m_status = Status::Any;
    QString m_searchText;
};
