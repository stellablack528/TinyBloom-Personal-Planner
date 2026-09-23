#include "SettingsManager.h"

SettingsManager::SettingsManager(DatabaseManager *database, QObject *parent)
    : QObject(parent), m_database(database) {}

void SettingsManager::load()
{
    const QString previousTheme = m_theme;
    const QString previousLanguage = m_language;
    const bool previousReduceAnimations = m_reduceAnimations;
    const bool previousCompatibilityRendering = m_compatibilityRendering;
    const int previousDuration = m_defaultTaskDuration;
    const QString storedTheme = m_database->setting(QStringLiteral("theme"), QStringLiteral("mint"));
    m_theme = storedTheme == QStringLiteral("midnight") ? storedTheme : QStringLiteral("mint");
    const QString storedLanguage = m_database->setting(QStringLiteral("language"), QStringLiteral("zh_CN"));
    m_language = storedLanguage == QStringLiteral("en") ? QStringLiteral("en") : QStringLiteral("zh_CN");
    m_reduceAnimations = m_database->setting(QStringLiteral("reduceAnimations"), QStringLiteral("false")) == QStringLiteral("true");
    m_compatibilityRendering = m_database->setting(QStringLiteral("compatibilityRendering"), QStringLiteral("false")) == QStringLiteral("true");
    bool ok = false;
    const int duration = m_database->setting(QStringLiteral("defaultTaskDuration"), QStringLiteral("25")).toInt(&ok);
    m_defaultTaskDuration = ok ? qBound(0, duration, 10080) : 25;
    if (previousTheme != m_theme) emit themeChanged();
    if (previousLanguage != m_language) emit languageChanged();
    if (previousReduceAnimations != m_reduceAnimations) emit reduceAnimationsChanged();
    if (previousCompatibilityRendering != m_compatibilityRendering) emit compatibilityRenderingChanged();
    if (previousDuration != m_defaultTaskDuration) emit defaultTaskDurationChanged();
}

QString SettingsManager::theme() const { return m_theme; }
QString SettingsManager::language() const { return m_language; }
bool SettingsManager::reduceAnimations() const { return m_reduceAnimations; }
bool SettingsManager::compatibilityRendering() const { return m_compatibilityRendering; }
int SettingsManager::defaultTaskDuration() const { return m_defaultTaskDuration; }

void SettingsManager::setTheme(const QString &theme)
{
    const QString normalized = theme == QStringLiteral("midnight") ? theme : QStringLiteral("mint");
    if (m_theme == normalized) return;
    if (!m_database->setSetting(QStringLiteral("theme"), normalized)) { emit errorOccurred(m_database->lastError()); return; }
    m_theme = normalized;
    emit themeChanged();
}

void SettingsManager::setLanguage(const QString &language)
{
    const QString normalized = language == QStringLiteral("en") ? QStringLiteral("en") : QStringLiteral("zh_CN");
    if (m_language == normalized) return;
    if (!m_database->setSetting(QStringLiteral("language"), normalized)) {
        emit errorOccurred(m_database->lastError());
        return;
    }
    m_language = normalized;
    emit languageChanged();
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

void SettingsManager::setCompatibilityRendering(const bool value)
{
    if (m_compatibilityRendering == value) return;
    if (!m_database->setSetting(QStringLiteral("compatibilityRendering"), value ? QStringLiteral("true") : QStringLiteral("false"))) {
        emit errorOccurred(m_database->lastError()); return;
    }
    m_compatibilityRendering = value;
    emit compatibilityRenderingChanged();
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
