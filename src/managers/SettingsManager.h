#pragma once

#include "database/DatabaseManager.h"

#include <QObject>

class SettingsManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(bool reduceAnimations READ reduceAnimations WRITE setReduceAnimations NOTIFY reduceAnimationsChanged)
    Q_PROPERTY(bool compatibilityRendering READ compatibilityRendering WRITE setCompatibilityRendering NOTIFY compatibilityRenderingChanged)
    Q_PROPERTY(bool focusEndSound READ focusEndSound WRITE setFocusEndSound NOTIFY focusEndSoundChanged)
    Q_PROPERTY(QString focusEndSoundStyle READ focusEndSoundStyle WRITE setFocusEndSoundStyle NOTIFY focusEndSoundStyleChanged)
    Q_PROPERTY(bool closeToTray READ closeToTray WRITE setCloseToTray NOTIFY closeToTrayChanged)
    Q_PROPERTY(int defaultTaskDuration READ defaultTaskDuration NOTIFY defaultTaskDurationChanged)
    Q_PROPERTY(int focusDurationMinutes READ focusDurationMinutes NOTIFY focusDurationMinutesChanged)
    Q_PROPERTY(int minimumFocusMinutes READ minimumFocusMinutes CONSTANT)
    Q_PROPERTY(int maximumFocusMinutes READ maximumFocusMinutes CONSTANT)

public:
    explicit SettingsManager(DatabaseManager *database, QObject *parent = nullptr);
    void load();
    QString theme() const;
    QString language() const;
    bool reduceAnimations() const;
    bool compatibilityRendering() const;
    bool focusEndSound() const;
    QString focusEndSoundStyle() const;
    bool closeToTray() const;
    bool closeToTrayHintShown() const;
    void markCloseToTrayHintShown();
    int defaultTaskDuration() const;
    int focusDurationMinutes() const;
    int minimumFocusMinutes() const { return MinimumFocusMinutes; }
    int maximumFocusMinutes() const { return MaximumFocusMinutes; }

    static constexpr int MinimumFocusMinutes = 5;
    static constexpr int MaximumFocusMinutes = 120;
    static constexpr int DefaultFocusMinutes = 25;
    Q_INVOKABLE bool setDefaultTaskDuration(int value);
    Q_INVOKABLE bool setFocusDurationMinutes(int minutes);

public slots:
    void setTheme(const QString &theme);
    void setLanguage(const QString &language);
    void setReduceAnimations(bool value);
    void setCompatibilityRendering(bool value);
    void setFocusEndSound(bool enabled);
    void setFocusEndSoundStyle(const QString &style);
    void setCloseToTray(bool enabled);

signals:
    void themeChanged();
    void languageChanged();
    void reduceAnimationsChanged();
    void compatibilityRenderingChanged();
    void focusEndSoundChanged();
    void focusEndSoundStyleChanged();
    void closeToTrayChanged();
    void defaultTaskDurationChanged();
    void focusDurationMinutesChanged();
    void errorOccurred(const QString &message);

private:
    DatabaseManager *m_database;
    QString m_theme = QStringLiteral("mint");
    QString m_language = QStringLiteral("zh_CN");
    bool m_reduceAnimations = false;
    bool m_compatibilityRendering = false;
    bool m_focusEndSound = true;
    QString m_focusEndSoundStyle = QStringLiteral("wind");
    bool m_closeToTray = false;
    int m_defaultTaskDuration = 25;
    int m_focusDurationMinutes = DefaultFocusMinutes;
};
