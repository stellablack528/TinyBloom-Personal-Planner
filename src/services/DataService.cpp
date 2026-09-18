#include "DataService.h"
#include "managers/SettingsManager.h"
#include "managers/TaskManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>

DataService::DataService(DatabaseManager *database, TaskManager *tasks, SettingsManager *settings, QObject *parent)
    : QObject(parent), m_database(database), m_tasks(tasks), m_settings(settings) {}

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
