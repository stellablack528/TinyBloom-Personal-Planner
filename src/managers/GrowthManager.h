#pragma once

#include "database/DatabaseManager.h"

#include <QObject>

class GrowthManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int totalXp READ totalXp NOTIFY growthChanged)
    Q_PROPERTY(int todayXp READ todayXp NOTIFY growthChanged)
    Q_PROPERTY(int level READ level NOTIFY growthChanged)
    Q_PROPERTY(int levelXp READ levelXp NOTIFY growthChanged)
    Q_PROPERTY(int xpForNextLevel READ xpForNextLevel CONSTANT)
    Q_PROPERTY(double levelProgress READ levelProgress NOTIFY growthChanged)
    Q_PROPERTY(int progressDays READ progressDays NOTIFY growthChanged)
    Q_PROPERTY(int vitality READ vitality NOTIFY growthChanged)
    Q_PROPERTY(int vitalityState READ vitalityState NOTIFY growthChanged)
    Q_PROPERTY(int gardenStage READ gardenStage NOTIFY growthChanged)
    Q_PROPERTY(QString gardenStageName READ gardenStageName NOTIFY growthChanged)
    Q_PROPERTY(QString gardenMessage READ gardenMessage NOTIFY growthChanged)
    Q_PROPERTY(QString vitalityMessage READ vitalityMessage NOTIFY growthChanged)
    Q_PROPERTY(int nextStageXp READ nextStageXp NOTIFY growthChanged)

public:
    explicit GrowthManager(DatabaseManager *database, QObject *parent = nullptr);

    bool initialize();
    int totalXp() const;
    int todayXp() const;
    int level() const;
    int levelXp() const;
    int xpForNextLevel() const;
    double levelProgress() const;
    int progressDays() const;
    int vitality() const;
    int vitalityState() const;
    int gardenStage() const;
    QString gardenStageName() const;
    QString gardenMessage() const;
    QString vitalityMessage() const;
    int nextStageXp() const;

public slots:
    void recordTaskCompleted(qint64 taskId, const QString &title);
    void recordSubtaskCompleted(qint64 subtaskId, const QString &title);
    void reload();
    void retranslate();

signals:
    void growthChanged();
    void experienceAwarded(int amount, const QString &reason);
    void errorOccurred(const QString &message);

private:
    bool reconcile();
    void load();

    DatabaseManager *m_database;
    DatabaseManager::GrowthStats m_stats;
    static constexpr int TaskXp = 20;
    static constexpr int TaskVitality = 28;
    static constexpr int SubtaskXp = 5;
    static constexpr int SubtaskVitality = 10;
};
