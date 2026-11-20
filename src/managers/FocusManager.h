#pragma once

#include "database/DatabaseManager.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QTimer>

class FocusManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY timerChanged)
    Q_PROPERTY(bool running READ running NOTIFY timerChanged)
    Q_PROPERTY(int todaySeconds READ todaySeconds NOTIFY historyChanged)
    Q_PROPERTY(int historyRevision READ historyRevision NOTIFY historyChanged)

public:
    explicit FocusManager(DatabaseManager *database, QObject *parent = nullptr);
    ~FocusManager() override;

    int remainingSeconds() const;
    bool running() const;
    int todaySeconds() const;
    int historyRevision() const;

    Q_INVOKABLE void toggle();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();
    Q_INVOKABLE int focusSecondsForDate(const QString &isoDate) const;
    void flushPending();
    void reload();

signals:
    void timerChanged();
    void historyChanged();
    void sessionCompleted(int focusedSeconds);
    void errorOccurred(const QString &message);

private slots:
    void updateTimer();

private:
    void start();
    void accumulate(const QDateTime &from, int seconds);

    DatabaseManager *m_database;
    QTimer m_timer;
    QDateTime m_lastTick;
    QHash<QDate, int> m_pendingByDate;
    QDate m_todayDate;
    int m_remainingSeconds = 25 * 60;
    int m_sessionFocusedSeconds = 0;
    int m_todaySeconds = 0;
    int m_historyRevision = 0;
    bool m_running = false;
    static constexpr int FlushIntervalSeconds = 15;
};
