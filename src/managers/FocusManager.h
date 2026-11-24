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
    Q_PROPERTY(bool sessionInProgress READ sessionInProgress NOTIFY timerChanged)
    Q_PROPERTY(int todaySeconds READ todaySeconds NOTIFY historyChanged)
    Q_PROPERTY(int historyRevision READ historyRevision NOTIFY historyChanged)

public:
    explicit FocusManager(DatabaseManager *database, QObject *parent = nullptr);
    ~FocusManager() override;

    int remainingSeconds() const;
    bool running() const;
    bool sessionInProgress() const;
    void setDurationMinutes(int minutes);
    int todaySeconds() const;
    int historyRevision() const;

    Q_INVOKABLE void toggle();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();
    Q_INVOKABLE int focusSecondsForDate(const QString &isoDate) const;
    Q_INVOKABLE int focusSecondsForMonth(int year, int month) const;
    Q_INVOKABLE int focusDaysForMonth(int year, int month) const;
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
    int m_durationMinutes;
    int m_remainingSeconds;
    int m_sessionFocusedSeconds = 0;
    int m_todaySeconds = 0;
    int m_historyRevision = 0;
    bool m_running = false;
    bool m_sessionInProgress = false;
    static constexpr int FlushIntervalSeconds = 15;
};
