#pragma once

#include "models/Task.h"

#include <QJsonObject>
#include <QObject>
#include <QSqlDatabase>

class DatabaseManager final : public QObject
{
    Q_OBJECT

public:
    struct GrowthStats {
        int totalXp = 0;
        int progressDays = 0;
        int todayXp = 0;
        int vitality = 100;
        QDate lastProgressDate;
        QDate vitalityUpdatedDate;
    };

    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    bool initializeDatabase(const QString &overridePath = {});
    void closeDatabase();
    void setExportMetadata(const QString &platform, const QString &version);
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] QString databasePath() const;
    [[nodiscard]] QString lastError() const;

    [[nodiscard]] QVector<Task> loadTasks() const;
    bool insertTask(Task &task);
    bool updateTask(const Task &task);
    bool deleteTask(qint64 taskId);
    bool insertSubtask(Subtask &subtask);
    bool updateSubtask(const Subtask &subtask);
    bool deleteSubtask(qint64 subtaskId);

    [[nodiscard]] QString setting(const QString &key, const QString &fallback = {}) const;
    bool setSetting(const QString &key, const QString &value);
    [[nodiscard]] QJsonObject exportObject() const;
    bool importObject(const QJsonObject &root);
    [[nodiscard]] GrowthStats loadGrowthStats() const;
    bool awardTaskExperience(qint64 taskId, int xp, int vitality, bool &awarded);
    bool awardSubtaskExperience(qint64 subtaskId, int xp, int vitality, bool &awarded);
    bool reconcileExperience(int taskXp, int taskVitality, int subtaskXp, int subtaskVitality, int &awardedXp);

private:
    bool openDatabase(const QString &path);
    bool createTables();
    bool execute(const QString &sql) const;
    bool ensureColumn(const QString &table, const QString &column, const QString &definition);
    bool awardExperience(const QString &table, qint64 sourceId, int xp, int vitality, bool &awarded);
    bool addExperienceToProfile(int xp, int vitality);
    bool applyVitalityDecay();
    void setError(const QString &context, const QString &technical) const;

    QString m_connectionName;
    QString m_databasePath;
    QSqlDatabase m_database;
    mutable QString m_lastError;
    QString m_exportPlatform = QStringLiteral("desktop");
    QString m_exportVersion = QStringLiteral("0.3.1-beta.1");
};
