#pragma once

#include "database/DatabaseManager.h"

#include <QObject>
#include <QThreadPool>

class TaskManager;
class SettingsManager;
class GrowthManager;

class DataService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    DataService(DatabaseManager *database, TaskManager *tasks, SettingsManager *settings,
        GrowthManager *growth = nullptr, QObject *parent = nullptr);
    ~DataService() override;

    Q_INVOKABLE bool exportData(const QUrl &fileUrl);
    Q_INVOKABLE bool exportDataAsync(const QUrl &fileUrl);
    Q_INVOKABLE bool importData(const QUrl &fileUrl);
    [[nodiscard]] bool busy() const;

signals:
    void operationSucceeded(const QString &message);
    void operationFailed(const QString &message);
    void busyChanged();

private:
    [[nodiscard]] QString createSafetyBackup();

    DatabaseManager *m_database;
    TaskManager *m_tasks;
    SettingsManager *m_settings;
    GrowthManager *m_growth;
    QThreadPool m_threadPool;
    bool m_busy = false;
};
