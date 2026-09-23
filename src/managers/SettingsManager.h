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
    Q_PROPERTY(int defaultTaskDuration READ defaultTaskDuration WRITE setDefaultTaskDuration NOTIFY defaultTaskDurationChanged)

public:
    explicit SettingsManager(DatabaseManager *database, QObject *parent = nullptr);
    void load();
    QString theme() const;
    QString language() const;
    bool reduceAnimations() const;
    bool compatibilityRendering() const;
    int defaultTaskDuration() const;

public slots:
    void setTheme(const QString &theme);
    void setLanguage(const QString &language);
    void setReduceAnimations(bool value);
    void setCompatibilityRendering(bool value);
    void setDefaultTaskDuration(int value);

signals:
    void themeChanged();
    void languageChanged();
    void reduceAnimationsChanged();
    void compatibilityRenderingChanged();
    void defaultTaskDurationChanged();
    void errorOccurred(const QString &message);

private:
    DatabaseManager *m_database;
    QString m_theme = QStringLiteral("mint");
    QString m_language = QStringLiteral("zh_CN");
    bool m_reduceAnimations = false;
    bool m_compatibilityRendering = false;
    int m_defaultTaskDuration = 25;
};
