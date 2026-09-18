#pragma once

#include "Subtask.h"

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QVector>

enum class Priority : int
{
    Low = 0,
    Medium = 1,
    High = 2
};

struct Task
{
    qint64 id = 0;
    QString title;
    QString description;
    bool completed = false;
    Priority priority = Priority::Medium;
    QDate dueDate;
    int estimatedMinutes = 25;
    QString category;
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime completedAt;
    QVector<Subtask> subtasks;
};

QString priorityName(Priority priority);
Priority priorityFromInt(int value);

