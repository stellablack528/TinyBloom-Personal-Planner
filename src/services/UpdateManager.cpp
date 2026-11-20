#include "UpdateManager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>
#include <QVersionNumber>

namespace {
struct ParsedVersion {
    QVersionNumber core;
    int stage = 3;
    int stageNumber = 0;
};

ParsedVersion parseVersion(QString value)
{
    value = value.trimmed();
    if (value.startsWith('v', Qt::CaseInsensitive)) value.remove(0, 1);
    const int dash = value.indexOf('-');
    const QString coreText = dash >= 0 ? value.left(dash) : value;
    const QString suffix = dash >= 0 ? value.mid(dash + 1).toLower() : QString{};
    ParsedVersion parsed;
    parsed.core = QVersionNumber::fromString(coreText);
    if (suffix.isEmpty()) return parsed;
    parsed.stage = suffix.startsWith(QStringLiteral("rc")) ? 2
        : suffix.startsWith(QStringLiteral("beta")) ? 1 : 0;
    const QRegularExpression numberExpression(QStringLiteral("(\\d+)$"));
    const QRegularExpressionMatch match = numberExpression.match(suffix);
    if (match.hasMatch()) parsed.stageNumber = match.captured(1).toInt();
    return parsed;
}
}

UpdateManager::UpdateManager(DatabaseManager *database, const QString &currentVersion, QObject *parent)
    : QObject(parent), m_database(database), m_currentVersion(currentVersion)
{
    loadCache();
}

QString UpdateManager::currentVersion() const { return m_currentVersion; }
QString UpdateManager::latestVersion() const { return m_latestVersion; }
QString UpdateManager::releaseName() const { return m_releaseName; }
QString UpdateManager::releaseNotes() const { return m_releaseNotes; }
QString UpdateManager::publishedDate() const { return m_publishedDate; }
bool UpdateManager::checking() const { return m_checking; }
bool UpdateManager::checked() const { return m_checked; }
bool UpdateManager::updateAvailable() const { return m_updateAvailable; }
QString UpdateManager::errorMessage() const { return m_errorMessage; }

bool UpdateManager::isNewerVersion(const QString &candidate, const QString &current)
{
    const ParsedVersion left = parseVersion(candidate);
    const ParsedVersion right = parseVersion(current);
    const int coreComparison = QVersionNumber::compare(left.core, right.core);
    if (coreComparison != 0) return coreComparison > 0;
    if (left.stage != right.stage) return left.stage > right.stage;
    return left.stageNumber > right.stageNumber;
}

void UpdateManager::checkForUpdates(const bool force)
{
    if (m_checking) return;
    if (!force) {
        const QDateTime checkedAt = QDateTime::fromString(
            m_database->setting(QStringLiteral("updates.lastCheckedUtc")), Qt::ISODateWithMs);
        if (checkedAt.isValid() && checkedAt.secsTo(QDateTime::currentDateTimeUtc()) < 6 * 60 * 60) {
            m_checked = true;
            emit updateStateChanged();
            if (m_updateAvailable
                && m_database->setting(QStringLiteral("updates.skippedVersion")) != m_latestVersion)
                emit updateAvailableDetected();
            return;
        }
    }

    m_checking = true;
    m_errorMessage.clear();
    emit updateStateChanged();
    QNetworkRequest request(QUrl(QStringLiteral(
        "https://api.github.com/repos/stellablack528/TinyBloom-Personal-Planner/releases?per_page=10")));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("User-Agent", QByteArray("TinyBloom/") + m_currentVersion.toUtf8());
    request.setTransferTimeout(15000);
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        m_checking = false;
        m_checked = true;
        if (reply->error() != QNetworkReply::NoError) {
            m_errorMessage = tr("Unable to check for updates. Please check your network connection.");
            emit updateStateChanged();
            reply->deleteLater();
            return;
        }
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        reply->deleteLater();
        if (parseError.error != QJsonParseError::NoError || !document.isArray()) {
            m_errorMessage = tr("The update service returned an invalid response.");
            emit updateStateChanged();
            return;
        }
        QJsonObject newest;
        for (const QJsonValue &value : document.array()) {
            const QJsonObject release = value.toObject();
            if (!release.value(QStringLiteral("draft")).toBool(true)) {
                newest = release;
                break;
            }
        }
        if (newest.isEmpty()) {
            m_errorMessage = tr("No published release was found.");
            emit updateStateChanged();
            return;
        }
        applyRelease(newest);
        m_database->setSetting(QStringLiteral("updates.lastCheckedUtc"),
            QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        saveCache();
        emit updateStateChanged();
        if (m_updateAvailable
            && m_database->setting(QStringLiteral("updates.skippedVersion")) != m_latestVersion)
            emit updateAvailableDetected();
    });
}

void UpdateManager::applyRelease(const QJsonObject &release, const bool fromCache)
{
    m_latestVersion = release.value(QStringLiteral("tag_name")).toString();
    if (m_latestVersion.startsWith('v', Qt::CaseInsensitive)) m_latestVersion.remove(0, 1);
    m_releaseName = release.value(QStringLiteral("name")).toString();
    m_releaseNotes = release.value(QStringLiteral("body")).toString();
    m_publishedDate = release.value(QStringLiteral("published_at")).toString();
    m_releasePageUrl = release.value(QStringLiteral("html_url")).toString();
    m_downloadUrl = fromCache
        ? release.value(QStringLiteral("download_url")).toString()
        : selectAssetUrl(release.value(QStringLiteral("assets")).toArray());
    m_updateAvailable = isNewerVersion(m_latestVersion, m_currentVersion);
}

QString UpdateManager::selectAssetUrl(const QJsonArray &assets) const
{
    QString fallback;
    for (const QJsonValue &value : assets) {
        const QJsonObject asset = value.toObject();
        const QString name = asset.value(QStringLiteral("name")).toString();
        const QString url = asset.value(QStringLiteral("browser_download_url")).toString();
#if defined(Q_OS_ANDROID)
        if (name.endsWith(QStringLiteral(".apk"), Qt::CaseInsensitive)) return url;
#elif defined(Q_OS_WIN)
        if (name.endsWith(QStringLiteral("Setup.exe"), Qt::CaseInsensitive)) return url;
        if (fallback.isEmpty() && name.endsWith(QStringLiteral(".zip"), Qt::CaseInsensitive)) fallback = url;
#else
        if (fallback.isEmpty()) fallback = url;
#endif
    }
    return fallback;
}

void UpdateManager::downloadUpdate()
{
    const QString target = m_downloadUrl.isEmpty() ? m_releasePageUrl : m_downloadUrl;
    if (!target.isEmpty()) QDesktopServices::openUrl(QUrl(target));
}

void UpdateManager::openReleasePage()
{
    if (!m_releasePageUrl.isEmpty()) QDesktopServices::openUrl(QUrl(m_releasePageUrl));
}

void UpdateManager::skipLatestVersion()
{
    if (!m_latestVersion.isEmpty())
        m_database->setSetting(QStringLiteral("updates.skippedVersion"), m_latestVersion);
}

void UpdateManager::loadCache()
{
    QJsonObject cached{{QStringLiteral("tag_name"), m_database->setting(QStringLiteral("updates.latestVersion"))},
        {QStringLiteral("name"), m_database->setting(QStringLiteral("updates.releaseName"))},
        {QStringLiteral("body"), m_database->setting(QStringLiteral("updates.releaseNotes"))},
        {QStringLiteral("published_at"), m_database->setting(QStringLiteral("updates.publishedDate"))},
        {QStringLiteral("html_url"), m_database->setting(QStringLiteral("updates.releasePageUrl"))},
        {QStringLiteral("download_url"), m_database->setting(QStringLiteral("updates.downloadUrl"))}};
    if (!cached.value(QStringLiteral("tag_name")).toString().isEmpty()) applyRelease(cached, true);
}

void UpdateManager::saveCache() const
{
    m_database->setSetting(QStringLiteral("updates.latestVersion"), m_latestVersion);
    m_database->setSetting(QStringLiteral("updates.releaseName"), m_releaseName.left(160));
    m_database->setSetting(QStringLiteral("updates.releaseNotes"), m_releaseNotes.left(1200));
    m_database->setSetting(QStringLiteral("updates.publishedDate"), m_publishedDate);
    m_database->setSetting(QStringLiteral("updates.releasePageUrl"), m_releasePageUrl);
    m_database->setSetting(QStringLiteral("updates.downloadUrl"), m_downloadUrl);
}
