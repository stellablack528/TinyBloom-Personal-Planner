#include "SettingsManager.h"

namespace {
bool isFocusEndSoundStyle(const QString &style)
{
    return style == QStringLiteral("wind") || style == QStringLiteral("glass") || style == QStringLiteral("custom");
}
}

SettingsManager::SettingsManager(DatabaseManager *database, QObject *parent)
    : QObject(parent), m_database(database) {}

void SettingsManager::load()
{
    const QString previousTheme = m_theme;
    const QString previousLanguage = m_language;
    const bool previousReduceAnimations = m_reduceAnimations;
    const bool previousCompatibilityRendering = m_compatibilityRendering;
    const bool previousFocusEndSound = m_focusEndSound;
    const QString previousFocusEndSoundStyle = m_focusEndSoundStyle;
    const bool previousCloseToTray = m_closeToTray;
    const QString previousRestoreShortcut = m_restoreShortcut;
    const int previousDuration = m_defaultTaskDuration;
    const int previousFocusDuration = m_focusDurationMinutes;
    const QString storedTheme = m_database->setting(QStringLiteral("theme"), QStringLiteral("mint"));
    m_theme = storedTheme == QStringLiteral("midnight") ? storedTheme : QStringLiteral("mint");
    const QString storedLanguage = m_database->setting(QStringLiteral("language"), QStringLiteral("zh_CN"));
    m_language = storedLanguage == QStringLiteral("en") ? QStringLiteral("en") : QStringLiteral("zh_CN");
    m_reduceAnimations = m_database->setting(QStringLiteral("reduceAnimations"), QStringLiteral("false")) == QStringLiteral("true");
    m_compatibilityRendering = m_database->setting(QStringLiteral("compatibilityRendering"), QStringLiteral("false")) == QStringLiteral("true");
    m_focusEndSound = m_database->setting(QStringLiteral("focusEndSound"), QStringLiteral("true")) == QStringLiteral("true");
    const QString storedSoundStyle = m_database->setting(QStringLiteral("focusEndSoundStyle"));
    m_focusEndSoundStyle = isFocusEndSoundStyle(storedSoundStyle) ? storedSoundStyle : QStringLiteral("wind");
    m_closeToTray = m_database->setting(QStringLiteral("closeToTray"), QStringLiteral("false")) == QStringLiteral("true");
    m_restoreShortcut = m_database->setting(QStringLiteral("restoreShortcut"));
    bool ok = false;
    const int duration = m_database->setting(QStringLiteral("defaultTaskDuration"), QStringLiteral("25")).toInt(&ok);
    m_defaultTaskDuration = ok ? qBound(0, duration, 10080) : 25;
    const int focusMinutes = m_database->setting(QStringLiteral("focusDurationMinutes")).toInt(&ok);
    m_focusDurationMinutes = ok ? qBound(MinimumFocusMinutes, focusMinutes, MaximumFocusMinutes) : DefaultFocusMinutes;
    if (previousTheme != m_theme) emit themeChanged();
    if (previousLanguage != m_language) emit languageChanged();
    if (previousReduceAnimations != m_reduceAnimations) emit reduceAnimationsChanged();
    if (previousCompatibilityRendering != m_compatibilityRendering) emit compatibilityRenderingChanged();
    if (previousFocusEndSound != m_focusEndSound) emit focusEndSoundChanged();
    if (previousFocusEndSoundStyle != m_focusEndSoundStyle) emit focusEndSoundStyleChanged();
    if (previousCloseToTray != m_closeToTray) emit closeToTrayChanged();
    if (previousRestoreShortcut != m_restoreShortcut) emit restoreShortcutChanged();
    if (previousDuration != m_defaultTaskDuration) emit defaultTaskDurationChanged();
    if (previousFocusDuration != m_focusDurationMinutes) emit focusDurationMinutesChanged();
}

QString SettingsManager::theme() const { return m_theme; }
QString SettingsManager::language() const { return m_language; }
bool SettingsManager::reduceAnimations() const { return m_reduceAnimations; }
bool SettingsManager::compatibilityRendering() const { return m_compatibilityRendering; }
bool SettingsManager::focusEndSound() const { return m_focusEndSound; }
QString SettingsManager::focusEndSoundStyle() const { return m_focusEndSoundStyle; }
bool SettingsManager::closeToTray() const { return m_closeToTray; }
QString SettingsManager::restoreShortcut() const { return m_restoreShortcut; }
int SettingsManager::defaultTaskDuration() const { return m_defaultTaskDuration; }
int SettingsManager::focusDurationMinutes() const { return m_focusDurationMinutes; }

bool SettingsManager::closeToTrayHintShown() const
{
    return m_database->setting(QStringLiteral("closeToTrayHintShown")) == QStringLiteral("true");
}

void SettingsManager::markCloseToTrayHintShown()
{
    m_database->setSetting(QStringLiteral("closeToTrayHintShown"), QStringLiteral("true"));
}

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

void SettingsManager::setFocusEndSound(const bool enabled)
{
    if (m_focusEndSound == enabled) return;
    if (!m_database->setSetting(QStringLiteral("focusEndSound"), enabled ? QStringLiteral("true") : QStringLiteral("false"))) {
        emit errorOccurred(m_database->lastError());
        return;
    }
    m_focusEndSound = enabled;
    emit focusEndSoundChanged();
}

void SettingsManager::setFocusEndSoundStyle(const QString &style)
{
    if (!isFocusEndSoundStyle(style) || m_focusEndSoundStyle == style) return;
    if (!m_database->setSetting(QStringLiteral("focusEndSoundStyle"), style)) {
        emit errorOccurred(m_database->lastError());
        return;
    }
    m_focusEndSoundStyle = style;
    emit focusEndSoundStyleChanged();
}

void SettingsManager::setCloseToTray(const bool enabled)
{
    if (m_closeToTray == enabled) return;
    if (!m_database->setSetting(QStringLiteral("closeToTray"), enabled ? QStringLiteral("true") : QStringLiteral("false"))) {
        emit errorOccurred(m_database->lastError());
        return;
    }
    m_closeToTray = enabled;
    emit closeToTrayChanged();
}

bool SettingsManager::setRestoreShortcut(const QString &shortcut)
{
    if (m_restoreShortcut == shortcut) return true;
    const QString stored = shortcut.isEmpty() ? QStringLiteral("") : shortcut;
    if (!m_database->setSetting(QStringLiteral("restoreShortcut"), stored)) return false;
    m_restoreShortcut = stored;
    emit restoreShortcutChanged();
    return true;
}

bool SettingsManager::setDefaultTaskDuration(const int value)
{
    const int bounded = qBound(0, value, 10080);
    if (m_defaultTaskDuration == bounded) return true;
    if (!m_database->setSetting(QStringLiteral("defaultTaskDuration"), QString::number(bounded))) {
        emit errorOccurred(m_database->lastError());
        return false;
    }
    m_defaultTaskDuration = bounded;
    emit defaultTaskDurationChanged();
    return true;
}

bool SettingsManager::setFocusDurationMinutes(const int minutes)
{
    if (minutes < MinimumFocusMinutes || minutes > MaximumFocusMinutes) return false;
    if (m_focusDurationMinutes == minutes) return true;
    if (!m_database->setSetting(QStringLiteral("focusDurationMinutes"), QString::number(minutes))) {
        emit errorOccurred(m_database->lastError());
        return false;
    }
    m_focusDurationMinutes = minutes;
    emit focusDurationMinutesChanged();
    return true;
}
