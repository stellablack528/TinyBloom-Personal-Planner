#pragma once

#include "database/DatabaseManager.h"

#include <QObject>

class SettingsManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(bool reduceAnimations READ reduceAnimations WRITE setReduceAnimations NOTIFY reduceAnimationsChanged)
    Q_PROPERTY(int defaultTaskDuration READ defaultTaskDuration WRITE setDefaultTaskDuration NOTIFY defaultTaskDurationChanged)

public:
    explicit SettingsManager(DatabaseManager *database, QObject *parent = nullptr);
    void load();
    QString theme() const;
    bool reduceAnimations() const;
    int defaultTaskDuration() const;

public slots:
    void setTheme(const QString &theme);
    void setReduceAnimations(bool value);
    void setDefaultTaskDuration(int value);

signals:
    void themeChanged();
    void reduceAnimationsChanged();
    void defaultTaskDurationChanged();
    void errorOccurred(const QString &message);

private:
    DatabaseManager *m_database;
    QString m_theme = QStringLiteral("mint");
    bool m_reduceAnimations = false;
    int m_defaultTaskDuration = 25;
};

