#pragma once

#include "database/DatabaseManager.h"

#include <QObject>

class TaskManager;
class SettingsManager;
class GrowthManager;

class DataService final : public QObject
{
    Q_OBJECT
public:
    DataService(DatabaseManager *database, TaskManager *tasks, SettingsManager *settings,
        GrowthManager *growth = nullptr, QObject *parent = nullptr);

    Q_INVOKABLE bool exportData(const QUrl &fileUrl);
    Q_INVOKABLE bool importData(const QUrl &fileUrl);

signals:
    void operationSucceeded(const QString &message);
    void operationFailed(const QString &message);

private:
    [[nodiscard]] QString createSafetyBackup();

    DatabaseManager *m_database;
    TaskManager *m_tasks;
    SettingsManager *m_settings;
    GrowthManager *m_growth;
};
