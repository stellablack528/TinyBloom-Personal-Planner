#include "Task.h"

#include <QCoreApplication>

QString priorityName(const Priority priority)
{
    switch (priority) {
    case Priority::Low: return QCoreApplication::translate("Priority", "Low");
    case Priority::High: return QCoreApplication::translate("Priority", "High");
    case Priority::Medium: return QCoreApplication::translate("Priority", "Medium");
    }
    return QCoreApplication::translate("Priority", "Medium");
}

Priority priorityFromInt(const int value)
{
    if (value == static_cast<int>(Priority::Low)) return Priority::Low;
    if (value == static_cast<int>(Priority::High)) return Priority::High;
    return Priority::Medium;
}
