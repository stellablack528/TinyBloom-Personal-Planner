#include "SettingsManager.h"

SettingsManager::SettingsManager(DatabaseManager *database, QObject *parent)
    : QObject(parent), m_database(database) {}

void SettingsManager::load()
{
    const QString previousTheme = m_theme;
    const bool previousReduceAnimations = m_reduceAnimations;
    const int previousDuration = m_defaultTaskDuration;
    const QString storedTheme = m_database->setting(QStringLiteral("theme"), QStringLiteral("mint"));
    m_theme = storedTheme == QStringLiteral("midnight") ? storedTheme : QStringLiteral("mint");
    m_reduceAnimations = m_database->setting(QStringLiteral("reduceAnimations"), QStringLiteral("false")) == QStringLiteral("true");
    bool ok = false;
    const int duration = m_database->setting(QStringLiteral("defaultTaskDuration"), QStringLiteral("25")).toInt(&ok);
    m_defaultTaskDuration = ok ? qBound(0, duration, 10080) : 25;
    if (previousTheme != m_theme) emit themeChanged();
    if (previousReduceAnimations != m_reduceAnimations) emit reduceAnimationsChanged();
    if (previousDuration != m_defaultTaskDuration) emit defaultTaskDurationChanged();
}

QString SettingsManager::theme() const { return m_theme; }
bool SettingsManager::reduceAnimations() const { return m_reduceAnimations; }
int SettingsManager::defaultTaskDuration() const { return m_defaultTaskDuration; }

void SettingsManager::setTheme(const QString &theme)
{
    const QString normalized = theme == QStringLiteral("midnight") ? theme : QStringLiteral("mint");
    if (m_theme == normalized) return;
    if (!m_database->setSetting(QStringLiteral("theme"), normalized)) { emit errorOccurred(m_database->lastError()); return; }
    m_theme = normalized;
    emit themeChanged();
}

void SettingsManager::setReduceAnimations(const bool value)
{
    if (m_reduceAnimations == value) return;
    if (!m_database->setSetting(QStringLiteral("reduceAnimations"), value ? QStringLiteral("true") : QStringLiteral("false"))) {
        emit errorOccurred(m_database->lastError()); return;
    }
    m_reduceAnimations = value;
    emit reduceAnimationsChanged();
}

void SettingsManager::setDefaultTaskDuration(const int value)
{
    const int bounded = qBound(0, value, 10080);
    if (m_defaultTaskDuration == bounded) return;
    if (!m_database->setSetting(QStringLiteral("defaultTaskDuration"), QString::number(bounded))) {
        emit errorOccurred(m_database->lastError()); return;
    }
    m_defaultTaskDuration = bounded;
    emit defaultTaskDurationChanged();
}
