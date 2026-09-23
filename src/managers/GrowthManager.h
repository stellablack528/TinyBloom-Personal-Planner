#pragma once

#include "database/DatabaseManager.h"

#include <array>
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
    Q_PROPERTY(QString firstPlantSpecies READ firstPlantSpecies NOTIFY growthChanged)
    Q_PROPERTY(QString secondPlantSpecies READ secondPlantSpecies NOTIFY growthChanged)

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
    QString firstPlantSpecies() const;
    QString secondPlantSpecies() const;

    Q_INVOKABLE bool plantSeed(int slot, const QString &species);
    Q_INVOKABLE int plantStage(int slot) const;
    Q_INVOKABLE int plantEarnedXp(int slot) const;
    Q_INVOKABLE int nextPlantStageXp(int slot) const;
    Q_INVOKABLE QString plantStageName(int slot) const;

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
    void loadGarden();
    [[nodiscard]] bool validSlot(int slot) const;

    DatabaseManager *m_database;
    DatabaseManager::GrowthStats m_stats;
    std::array<QString, 2> m_plantSpecies;
    std::array<int, 2> m_plantStartXp{0, 0};
    static constexpr int TaskXp = 20;
    static constexpr int TaskVitality = 28;
    static constexpr int SubtaskXp = 5;
    static constexpr int SubtaskVitality = 10;
};
