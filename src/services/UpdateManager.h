#pragma once

#include "database/DatabaseManager.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

class UpdateManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateStateChanged)
    Q_PROPERTY(QString releaseName READ releaseName NOTIFY updateStateChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY updateStateChanged)
    Q_PROPERTY(QString publishedDate READ publishedDate NOTIFY updateStateChanged)
    Q_PROPERTY(bool checking READ checking NOTIFY updateStateChanged)
    Q_PROPERTY(bool checked READ checked NOTIFY updateStateChanged)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateStateChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY updateStateChanged)

public:
    explicit UpdateManager(DatabaseManager *database, const QString &currentVersion,
        QObject *parent = nullptr);

    QString currentVersion() const;
    QString latestVersion() const;
    QString releaseName() const;
    QString releaseNotes() const;
    QString publishedDate() const;
    bool checking() const;
    bool checked() const;
    bool updateAvailable() const;
    QString errorMessage() const;

    Q_INVOKABLE void checkForUpdates(bool force = true);
    Q_INVOKABLE void downloadUpdate();
    Q_INVOKABLE void openReleasePage();
    Q_INVOKABLE void skipLatestVersion();
    static bool isNewerVersion(const QString &candidate, const QString &current);

signals:
    void updateStateChanged();
    void updateAvailableDetected();

private:
    void applyRelease(const QJsonObject &release, bool fromCache = false);
    void loadCache();
    void saveCache() const;
    QString selectAssetUrl(const QJsonArray &assets) const;

    DatabaseManager *m_database;
    QNetworkAccessManager m_network;
    QString m_currentVersion;
    QString m_latestVersion;
    QString m_releaseName;
    QString m_releaseNotes;
    QString m_publishedDate;
    QString m_downloadUrl;
    QString m_releasePageUrl;
    QString m_errorMessage;
    bool m_checking = false;
    bool m_checked = false;
    bool m_updateAvailable = false;
};
