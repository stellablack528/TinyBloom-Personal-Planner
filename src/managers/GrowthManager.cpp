#include "GrowthManager.h"

#include <array>
#include <QStringList>

namespace {
constexpr std::array<int, 6> StageThresholds{0, 60, 160, 320, 520, 800};

int stageForXp(const int xp)
{
    int stage = 0;
    for (int i = 1; i < static_cast<int>(StageThresholds.size()); ++i) {
        if (xp < StageThresholds.at(i)) break;
        stage = i;
    }
    return stage;
}
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
    return stageForXp(m_stats.totalXp);
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

QString GrowthManager::firstPlantSpecies() const { return m_plantSpecies.at(0); }
QString GrowthManager::secondPlantSpecies() const { return m_plantSpecies.at(1); }

bool GrowthManager::plantSeed(const int slot, const QString &species)
{
    static const QStringList allowed{QStringLiteral("pink"), QStringLiteral("sunflower"),
        QStringLiteral("tulip"), QStringLiteral("rose")};
    if (!validSlot(slot) || !allowed.contains(species)) {
        emit errorOccurred(tr("Please choose a valid flower seed."));
        return false;
    }
    const QString value = QStringLiteral("%1|%2").arg(species).arg(m_stats.totalXp);
    if (!m_database->setSetting(QStringLiteral("garden.plant.%1").arg(slot), value)) {
        emit errorOccurred(m_database->lastError());
        return false;
    }
    m_plantSpecies[slot] = species;
    m_plantStartXp[slot] = m_stats.totalXp;
    emit growthChanged();
    return true;
}

int GrowthManager::plantEarnedXp(const int slot) const
{
    return validSlot(slot) && !m_plantSpecies.at(slot).isEmpty()
        ? qMax(0, m_stats.totalXp - m_plantStartXp.at(slot)) : 0;
}

int GrowthManager::plantStage(const int slot) const
{
    return validSlot(slot) && !m_plantSpecies.at(slot).isEmpty()
        ? stageForXp(plantEarnedXp(slot)) : 0;
}

int GrowthManager::nextPlantStageXp(const int slot) const
{
    if (!validSlot(slot) || m_plantSpecies.at(slot).isEmpty()) return 0;
    const int next = plantStage(slot) + 1;
    return next < static_cast<int>(StageThresholds.size())
        ? StageThresholds.at(next) : plantEarnedXp(slot);
}

QString GrowthManager::plantStageName(const int slot) const
{
    switch (plantStage(slot)) {
    case 0: return tr("A tiny seed");
    case 1: return tr("First sprout");
    case 2: return tr("Young leaves");
    case 3: return tr("Flower bud");
    case 4: return tr("In bloom");
    default: return tr("Flourishing garden");
    }
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
    loadGarden();
    emit growthChanged();
}

void GrowthManager::loadGarden()
{
    static const QStringList allowed{QStringLiteral("pink"), QStringLiteral("sunflower"),
        QStringLiteral("tulip"), QStringLiteral("rose")};
    for (int slot = 0; slot < 2; ++slot) {
        const QString key = QStringLiteral("garden.plant.%1").arg(slot);
        const QString stored = m_database->setting(key, QStringLiteral("__missing__"));
        if (stored == QStringLiteral("__missing__")) {
            m_plantSpecies[slot].clear();
            m_plantStartXp[slot] = m_stats.totalXp;
            if (slot == 0 && m_stats.totalXp > 0) {
                m_plantSpecies[slot] = QStringLiteral("pink");
                m_plantStartXp[slot] = 0;
                m_database->setSetting(key, QStringLiteral("pink|0"));
            }
            continue;
        }
        const QStringList parts = stored.split('|');
        bool baselineOk = false;
        const int baseline = parts.value(1).toInt(&baselineOk);
        if (parts.size() != 2 || !allowed.contains(parts.first()) || !baselineOk || baseline < 0) {
            m_plantSpecies[slot].clear();
            m_plantStartXp[slot] = m_stats.totalXp;
            continue;
        }
        m_plantSpecies[slot] = parts.first();
        m_plantStartXp[slot] = qMin(baseline, m_stats.totalXp);
    }
}

bool GrowthManager::validSlot(const int slot) const
{
    return slot >= 0 && slot < static_cast<int>(m_plantSpecies.size());
}
