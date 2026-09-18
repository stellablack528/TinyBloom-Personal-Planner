#include "GrowthManager.h"

#include <array>

namespace {
constexpr std::array<int, 6> StageThresholds{0, 60, 160, 320, 520, 800};
}

GrowthManager::GrowthManager(DatabaseManager *database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool GrowthManager::initialize()
{
    if (!m_database->isOpen()) {
        emit errorOccurred(tr("Local storage is not available."));
        return false;
    }
    if (!reconcile()) return false;
    load();
    return true;
}

int GrowthManager::totalXp() const { return m_stats.totalXp; }
int GrowthManager::todayXp() const { return m_stats.todayXp; }
int GrowthManager::level() const { return m_stats.totalXp / xpForNextLevel() + 1; }
int GrowthManager::levelXp() const { return m_stats.totalXp % xpForNextLevel(); }
int GrowthManager::xpForNextLevel() const { return 100; }
double GrowthManager::levelProgress() const
{
    return static_cast<double>(levelXp()) / xpForNextLevel();
}
int GrowthManager::progressDays() const { return m_stats.progressDays; }
int GrowthManager::vitality() const { return m_stats.vitality; }
int GrowthManager::vitalityState() const
{
    return m_stats.vitality >= 65 ? 0 : m_stats.vitality >= 30 ? 1 : 2;
}

int GrowthManager::gardenStage() const
{
    int stage = 0;
    for (int i = 1; i < static_cast<int>(StageThresholds.size()); ++i) {
        if (m_stats.totalXp < StageThresholds.at(i)) break;
        stage = i;
    }
    return stage;
}

QString GrowthManager::gardenStageName() const
{
    switch (gardenStage()) {
    case 0: return tr("A tiny seed");
    case 1: return tr("First sprout");
    case 2: return tr("Young leaves");
    case 3: return tr("Flower bud");
    case 4: return tr("In bloom");
    default: return tr("Flourishing garden");
    }
}

QString GrowthManager::gardenMessage() const
{
    switch (gardenStage()) {
    case 0: return tr("Every garden begins with one small step.");
    case 1: return tr("Your effort has broken through the soil.");
    case 2: return tr("Steady care is growing something real.");
    case 3: return tr("A bloom is close. Keep showing up.");
    case 4: return tr("Your small steps have become a flower.");
    default: return tr("This garden carries every step you kept.");
    }
}

QString GrowthManager::vitalityMessage() const
{
    if (vitalityState() == 0) return tr("Your garden feels bright and cared for.");
    if (vitalityState() == 1) return tr("Your plant is getting tired. A completed step will help it recover.");
    return tr("Your plant is wilting. Come back with one small, doable step.");
}

int GrowthManager::nextStageXp() const
{
    const int next = gardenStage() + 1;
    return next < static_cast<int>(StageThresholds.size()) ? StageThresholds.at(next) : m_stats.totalXp;
}

void GrowthManager::recordTaskCompleted(const qint64 taskId, const QString &title)
{
    bool awarded = false;
    if (!m_database->awardTaskExperience(taskId, TaskXp, TaskVitality, awarded)) {
        emit errorOccurred(m_database->lastError());
        return;
    }
    if (!awarded) return;
    load();
    emit experienceAwarded(TaskXp, tr("Task complete: %1").arg(title));
}

void GrowthManager::recordSubtaskCompleted(const qint64 subtaskId, const QString &title)
{
    bool awarded = false;
    if (!m_database->awardSubtaskExperience(subtaskId, SubtaskXp, SubtaskVitality, awarded)) {
        emit errorOccurred(m_database->lastError());
        return;
    }
    if (!awarded) return;
    load();
    emit experienceAwarded(SubtaskXp, tr("Small step complete: %1").arg(title));
}

void GrowthManager::reload()
{
    if (!reconcile()) return;
    load();
}

void GrowthManager::retranslate()
{
    emit growthChanged();
}

bool GrowthManager::reconcile()
{
    int awardedXp = 0;
    if (m_database->reconcileExperience(TaskXp, TaskVitality, SubtaskXp, SubtaskVitality, awardedXp)) return true;
    emit errorOccurred(m_database->lastError());
    return false;
}

void GrowthManager::load()
{
    m_stats = m_database->loadGrowthStats();
    emit growthChanged();
}
