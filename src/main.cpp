#include "database/DatabaseManager.h"
#include "managers/SettingsManager.h"
#include "managers/TaskManager.h"
#include "managers/GrowthManager.h"
#include "managers/FocusManager.h"
#include "services/DataService.h"
#include "services/UpdateManager.h"

#include <QGuiApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QTimer>
#include <QTranslator>

#if defined(Q_OS_WIN)
#include "DesktopVersion.h"
#include "platform/windows/DesktopIntegration.h"
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
#if defined(Q_OS_WIN)
    const QString applicationVersion = QStringLiteral(TINYBLOOM_DESKTOP_VERSION);
#else
    const QString applicationVersion = QStringLiteral(TINYBLOOM_RELEASE_VERSION);
#endif
    const bool mobilePreview = qEnvironmentVariableIntValue("TINYBLOOM_MOBILE_PREVIEW") == 1;
#if defined(Q_OS_ANDROID)
    const bool mobilePlatform = true;
    const QString platformId = QStringLiteral("android");
    const QString platformLabel = QStringLiteral("Android");
#elif defined(Q_OS_IOS)
    const bool mobilePlatform = true;
    const QString platformId = QStringLiteral("ios");
    const QString platformLabel = QStringLiteral("iOS");
#else
    const bool mobilePlatform = false;
    const QString platformId = QStringLiteral("desktop");
    const QString platformLabel = QStringLiteral("Windows");
#endif
    QCoreApplication::setOrganizationName(QStringLiteral("TinyBloom"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("tinybloom.app"));
    QCoreApplication::setApplicationName(QStringLiteral("TinyBloom"));
    QGuiApplication::setApplicationDisplayName(mobilePlatform
        ? QStringLiteral("TinyBloom Mobile") : QStringLiteral("TinyBloom Desktop"));
    QCoreApplication::setApplicationVersion(applicationVersion);
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/resources/icons/app-icon.svg")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    const QString screenshotScenario = qEnvironmentVariable("TINYBLOOM_SCREENSHOT_SCENARIO");

    DatabaseManager database;
    database.setExportMetadata(platformId, applicationVersion);
    const QString databaseOverride = qEnvironmentVariable("TINYBLOOM_DATABASE_PATH");
    if (!database.initializeDatabase(databaseOverride)) qCritical().noquote() << database.lastError();
    if (screenshotScenario == QStringLiteral("focus-month-demo")) {
        const QDate today = QDate::currentDate();
        const QDate firstDay(today.year(), today.month(), 1);
        const int dailyMinutes[] = {25, 50, 75, 40, 0, 90, 110, 0, 25, 60,
            120, 45, 80, 150, 0, 55, 100, 35, 130, 105};
        const int daysToSeed = qMin(today.day(), static_cast<int>(std::size(dailyMinutes)));
        for (int index = 0; index < daysToSeed; ++index) {
            const QDate date = firstDay.addDays(index);
            if (dailyMinutes[index] > 0 && database.focusSecondsForDate(date) == 0)
                database.addFocusSeconds(date, dailyMinutes[index] * 60);
        }
    }
    TaskManager taskManager(&database);
    SettingsManager settingsManager(&database);
    settingsManager.load();
    if (!mobilePlatform && (settingsManager.compatibilityRendering()
        || qEnvironmentVariableIntValue("TINYBLOOM_SOFTWARE_RENDERING") == 1)) {
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    }
    GrowthManager growthManager(&database);
    growthManager.initialize();
    FocusManager focusManager(&database);
    const auto applyFocusDuration = [&settingsManager, &focusManager] {
        focusManager.setDurationMinutes(settingsManager.focusDurationMinutes());
    };
    applyFocusDuration();
    QObject::connect(&settingsManager, &SettingsManager::focusDurationMinutesChanged, &focusManager, applyFocusDuration);
    const QString languageOverride = qEnvironmentVariable("TINYBLOOM_LANGUAGE_OVERRIDE");
    if (!languageOverride.isEmpty()) settingsManager.setLanguage(languageOverride);
    DataService dataService(&database, &taskManager, &settingsManager, &growthManager, &focusManager);
    UpdateManager updateManager(&database, applicationVersion);
    QObject::connect(&taskManager, &TaskManager::taskCompleted,
        &growthManager, &GrowthManager::recordTaskCompleted);
    QObject::connect(&taskManager, &TaskManager::subtaskCompleted,
        &growthManager, &GrowthManager::recordSubtaskCompleted);

    QQmlApplicationEngine engine;
#if defined(Q_OS_WIN)
    const QString customSoundPath = database.isOpen()
        ? QFileInfo(database.databasePath()).dir().filePath(QStringLiteral("focus-custom.wav")) : QString();
    DesktopIntegration windowsDesktop(settingsManager, customSoundPath);
    QObject *desktopIntegration = mobilePreview ? nullptr : &windowsDesktop;
    QObject::connect(&windowsDesktop, &DesktopIntegration::exitRequested, &app, [&app, &focusManager] {
        focusManager.pause();
        app.quit();
    });
#else
    QObject *desktopIntegration = nullptr;
#endif
    QTranslator translator;
    const auto applyLanguage = [&app, &engine, &translator, &settingsManager, &taskManager, &growthManager](const bool retranslate) {
        app.removeTranslator(&translator);
        if (settingsManager.language() == QStringLiteral("zh_CN")) {
            if (!translator.load(QStringLiteral(":/i18n/tinybloom_zh_CN.qm")))
                qWarning() << "Unable to load Simplified Chinese translation";
            else
                app.installTranslator(&translator);
        }
        if (retranslate) {
            engine.retranslate();
            taskManager.retranslate();
            growthManager.retranslate();
        }
    };
    applyLanguage(false);
    taskManager.initialize();
    QObject::connect(&settingsManager, &SettingsManager::languageChanged, &app,
        [&applyLanguage] { applyLanguage(true); });
    engine.rootContext()->setContextProperty(QStringLiteral("taskManager"), &taskManager);
    engine.rootContext()->setContextProperty(QStringLiteral("settingsManager"), &settingsManager);
    engine.rootContext()->setContextProperty(QStringLiteral("growthManager"), &growthManager);
    engine.rootContext()->setContextProperty(QStringLiteral("focusManager"), &focusManager);
    engine.rootContext()->setContextProperty(QStringLiteral("desktopIntegration"), desktopIntegration);
    engine.rootContext()->setContextProperty(QStringLiteral("dataService"), &dataService);
    engine.rootContext()->setContextProperty(QStringLiteral("updateManager"), &updateManager);
    engine.rootContext()->setContextProperty(QStringLiteral("databaseReady"), database.isOpen());
    engine.rootContext()->setContextProperty(QStringLiteral("mobilePlatform"), mobilePlatform);
    engine.rootContext()->setContextProperty(QStringLiteral("mobilePreview"), mobilePreview);
    engine.rootContext()->setContextProperty(QStringLiteral("platformId"), platformId);
    engine.rootContext()->setContextProperty(QStringLiteral("platformLabel"), platformLabel);
    engine.rootContext()->setContextProperty(QStringLiteral("screenshotScenario"), screenshotScenario);
    engine.loadFromModule(QStringLiteral("TinyBloom"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) return -1;
#if defined(Q_OS_WIN)
    if (desktopIntegration) windowsDesktop.setWindow(qobject_cast<QWindow *>(engine.rootObjects().constFirst()));
#endif
    const QString screenshotPath = qEnvironmentVariable("TINYBLOOM_SCREENSHOT_PATH");
    if (!screenshotPath.isEmpty()) {
        QTimer::singleShot(1200, &app, [&app, &engine, screenshotPath] {
            auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
            const bool saved = window && window->grabWindow().save(screenshotPath);
            app.exit(saved ? 0 : 2);
        });
    }
    return app.exec();
}
