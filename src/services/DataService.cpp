#include "DataService.h"
#include "managers/SettingsManager.h"
#include "managers/TaskManager.h"
#include "managers/GrowthManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMetaObject>
#include <QPointer>
#include <QSaveFile>
#include <QThread>

DataService::DataService(DatabaseManager *database, TaskManager *tasks, SettingsManager *settings,
    GrowthManager *growth, QObject *parent)
    : QObject(parent), m_database(database), m_tasks(tasks), m_settings(settings), m_growth(growth)
{
    const int availableWorkers = qMax(1, QThread::idealThreadCount() - 1);
    m_threadPool.setMaxThreadCount(qMin(2, availableWorkers));
    m_threadPool.setExpiryTimeout(15000);
}

DataService::~DataService()
{
    m_threadPool.waitForDone();
}

bool DataService::busy() const { return m_busy; }

bool DataService::exportData(const QUrl &fileUrl)
{
    const QString path = fileUrl.toLocalFile();
    if (path.isEmpty()) { emit operationFailed(tr("Please choose a valid save location.")); return false; }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { emit operationFailed(tr("Unable to write the export file.")); return false; }
    file.write(QJsonDocument(m_database->exportObject()).toJson(QJsonDocument::Indented));
    if (!file.commit()) { emit operationFailed(tr("Unable to finish writing the export file.")); return false; }
    emit operationSucceeded(tr("Your TinyBloom data was exported."));
    return true;
}

bool DataService::exportDataAsync(const QUrl &fileUrl)
{
    const QString path = fileUrl.toLocalFile();
    if (path.isEmpty()) { emit operationFailed(tr("Please choose a valid save location.")); return false; }
    if (m_busy) { emit operationFailed(tr("A data operation is already in progress.")); return false; }

    // SQLite remains on its owning (UI) thread. Only immutable JSON and file I/O
    // cross the thread boundary, so the render and database threads stay safe.
    const QJsonObject snapshot = m_database->exportObject();
    m_busy = true;
    emit busyChanged();
    QPointer<DataService> self(this);
    m_threadPool.start([self, snapshot, path] {
        int result = 0;
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            result = 1;
        } else {
            const QByteArray payload = QJsonDocument(snapshot).toJson(QJsonDocument::Indented);
            if (file.write(payload) != payload.size()) result = 1;
            else if (!file.commit()) result = 2;
        }
        if (!self) return;
        QMetaObject::invokeMethod(self, [self, result] {
            if (!self) return;
            self->m_busy = false;
            emit self->busyChanged();
            if (result == 0) emit self->operationSucceeded(DataService::tr("Your TinyBloom data was exported in the background."));
            else if (result == 1) emit self->operationFailed(DataService::tr("Unable to write the export file."));
            else emit self->operationFailed(DataService::tr("Unable to finish writing the export file."));
        }, Qt::QueuedConnection);
    });
    return true;
}

bool DataService::importData(const QUrl &fileUrl)
{
    QFile file(fileUrl.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) { emit operationFailed(tr("Unable to read this file.")); return false; }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        emit operationFailed(tr("This file does not contain valid JSON."));
        return false;
    }
    const QString backupPath = createSafetyBackup();
    if (backupPath.isEmpty()) return false;
    if (!m_database->importObject(document.object())) { emit operationFailed(m_database->lastError()); return false; }
    m_tasks->reload();
    m_settings->load();
    if (m_growth) m_growth->reload();
    emit operationSucceeded(tr("Your TinyBloom data was imported. Backup saved to %1")
        .arg(QDir::toNativeSeparators(backupPath)));
    return true;
}

QString DataService::createSafetyBackup()
{
    const QFileInfo databaseInfo(m_database->databasePath());
    if (databaseInfo.absolutePath().isEmpty()) {
        emit operationFailed(tr("Unable to find the TinyBloom data folder."));
        return {};
    }
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
    const QString backupPath = databaseInfo.dir().filePath(
        QStringLiteral("TinyBloom-backup-before-import-%1.json").arg(timestamp));
    QSaveFile backup(backupPath);
    if (!backup.open(QIODevice::WriteOnly)) {
        emit operationFailed(tr("Unable to create a safety backup. Import was cancelled."));
        return {};
    }
    backup.write(QJsonDocument(m_database->exportObject()).toJson(QJsonDocument::Indented));
    if (!backup.commit()) {
        emit operationFailed(tr("Unable to finish the safety backup. Import was cancelled."));
        return {};
    }
    return backupPath;
}
