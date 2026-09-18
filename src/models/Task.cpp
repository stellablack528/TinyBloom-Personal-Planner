#include "Task.h"

QString priorityName(const Priority priority)
{
    switch (priority) {
    case Priority::Low: return QStringLiteral("Low");
    case Priority::High: return QStringLiteral("High");
    case Priority::Medium: return QStringLiteral("Medium");
    }
    return QStringLiteral("Medium");
}

Priority priorityFromInt(const int value)
{
    if (value == static_cast<int>(Priority::Low)) return Priority::Low;
    if (value == static_cast<int>(Priority::High)) return Priority::High;
    return Priority::Medium;
}

