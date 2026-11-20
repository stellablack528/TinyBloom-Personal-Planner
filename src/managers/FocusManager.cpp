#include "FocusManager.h"

#include <QtGlobal>

FocusManager::FocusManager(DatabaseManager *database, QObject *parent)
    : QObject(parent), m_database(database)
{
    m_timer.setInterval(1000);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &FocusManager::updateTimer);
    reload();
}

FocusManager::~FocusManager()
{
    if (m_running) updateTimer();
    flushPending();
}

int FocusManager::remainingSeconds() const { return m_remainingSeconds; }
bool FocusManager::running() const { return m_running; }
int FocusManager::todaySeconds() const { return m_todaySeconds; }
int FocusManager::historyRevision() const { return m_historyRevision; }

void FocusManager::toggle()
{
    m_running ? pause() : start();
}

void FocusManager::start()
{
    if (m_running) return;
    if (m_todayDate != QDate::currentDate()) {
        m_todayDate = QDate::currentDate();
        m_todaySeconds = m_database->focusSecondsForDate(m_todayDate)
            + m_pendingByDate.value(m_todayDate, 0);
        ++m_historyRevision;
        emit historyChanged();
    }
    if (m_remainingSeconds <= 0) {
        m_remainingSeconds = 25 * 60;
        m_sessionFocusedSeconds = 0;
    }
    m_lastTick = QDateTime::currentDateTime();
    m_running = true;
    m_timer.start();
    emit timerChanged();
}

void FocusManager::pause()
{
    if (!m_running) return;
    updateTimer();
    m_timer.stop();
    m_running = false;
    flushPending();
    emit timerChanged();
}

void FocusManager::reset()
{
    pause();
    m_remainingSeconds = 25 * 60;
    m_sessionFocusedSeconds = 0;
    emit timerChanged();
}

int FocusManager::focusSecondsForDate(const QString &isoDate) const
{
    const QDate date = QDate::fromString(isoDate, Qt::ISODate);
    return m_database->focusSecondsForDate(date) + m_pendingByDate.value(date, 0);
}

void FocusManager::reload()
{
    m_pendingByDate.clear();
    m_todayDate = QDate::currentDate();
    m_todaySeconds = m_database->focusSecondsForDate(m_todayDate);
    ++m_historyRevision;
    emit historyChanged();
}

void FocusManager::updateTimer()
{
    if (!m_running) return;
    const QDateTime now = QDateTime::currentDateTime();
    const qint64 wallSeconds = m_lastTick.secsTo(now);
    if (wallSeconds <= 0) return;

    if (m_todayDate != now.date()) {
        m_todayDate = now.date();
        m_todaySeconds = m_database->focusSecondsForDate(m_todayDate)
            + m_pendingByDate.value(m_todayDate, 0);
    }
    const int elapsed = qMin<int>(m_remainingSeconds,
        static_cast<int>(qMin<qint64>(wallSeconds, 25 * 60)));
    accumulate(m_lastTick, elapsed);
    m_lastTick = now;
    m_remainingSeconds -= elapsed;
    m_sessionFocusedSeconds += elapsed;
    emit timerChanged();

    int pendingSeconds = 0;
    for (auto it = m_pendingByDate.cbegin(); it != m_pendingByDate.cend(); ++it)
        pendingSeconds += it.value();
    if (pendingSeconds >= FlushIntervalSeconds) flushPending();

    if (m_remainingSeconds > 0) return;
    m_timer.stop();
    m_running = false;
    flushPending();
    emit timerChanged();
    emit sessionCompleted(m_sessionFocusedSeconds);
}

void FocusManager::accumulate(const QDateTime &from, int seconds)
{
    QDateTime cursor = from;
    while (seconds > 0) {
        const QDateTime nextMidnight(cursor.date().addDays(1).startOfDay());
        const int untilMidnight = qMax(1, static_cast<int>(cursor.secsTo(nextMidnight)));
        const int chunk = qMin(seconds, untilMidnight);
        m_pendingByDate[cursor.date()] += chunk;
        if (cursor.date() == QDate::currentDate()) m_todaySeconds += chunk;
        cursor = cursor.addSecs(chunk);
        seconds -= chunk;
    }
    ++m_historyRevision;
    emit historyChanged();
}

void FocusManager::flushPending()
{
    if (m_pendingByDate.isEmpty()) return;
    const auto pending = m_pendingByDate;
    for (auto it = pending.cbegin(); it != pending.cend(); ++it) {
        if (!m_database->addFocusSeconds(it.key(), it.value())) {
            emit errorOccurred(m_database->lastError());
            return;
        }
        m_pendingByDate.remove(it.key());
    }
    ++m_historyRevision;
    emit historyChanged();
}
