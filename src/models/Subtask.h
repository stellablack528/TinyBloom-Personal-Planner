#pragma once

#include <QDateTime>
#include <QString>

struct Subtask
{
    qint64 id = 0;
    qint64 taskId = 0;
    QString title;
    bool completed = false;
    QDateTime createdAt;
    bool experienceAwarded = false;
    qint64 parentId = 0;
};
