#include "database/DatabaseManager.h"
#include "managers/SettingsManager.h"
#include "managers/TaskManager.h"
#include "managers/GrowthManager.h"
#include "services/DataService.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QTimer>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    const QString applicationVersion = QStringLiteral("0.3.0-beta.1");
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
#if defined(Q_OS_ANDROID)
    // Use the native-looking style already deployed by Qt on Android. Keeping
    // Basic as a second full style adds noticeable size to the install APK.
    QQuickStyle::setStyle(QStringLiteral("Material"));
#else
    QQuickStyle::setStyle(QStringLiteral("Basic"));
#endif

    DatabaseManager database;
    database.setExportMetadata(platformId, applicationVersion);
    const QString databaseOverride = qEnvironmentVariable("TINYBLOOM_DATABASE_PATH");
    if (!database.initializeDatabase(databaseOverride)) qCritical().noquote() << database.lastError();
    TaskManager taskManager(&database);
    SettingsManager settingsManager(&database);
    settingsManager.load();
    if (!mobilePlatform && (settingsManager.compatibilityRendering()
        || qEnvironmentVariableIntValue("TINYBLOOM_SOFTWARE_RENDERING") == 1)) {
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    }
    GrowthManager growthManager(&database);
    growthManager.initialize();
    const QString languageOverride = qEnvironmentVariable("TINYBLOOM_LANGUAGE_OVERRIDE");
    if (!languageOverride.isEmpty()) settingsManager.setLanguage(languageOverride);
    DataService dataService(&database, &taskManager, &settingsManager, &growthManager);
    QObject::connect(&taskManager, &TaskManager::taskCompleted,
        &growthManager, &GrowthManager::recordTaskCompleted);
    QObject::connect(&taskManager, &TaskManager::subtaskCompleted,
        &growthManager, &GrowthManager::recordSubtaskCompleted);

    QQmlApplicationEngine engine;
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
    engine.rootContext()->setContextProperty(QStringLiteral("dataService"), &dataService);
    engine.rootContext()->setContextProperty(QStringLiteral("databaseReady"), database.isOpen());
    engine.rootContext()->setContextProperty(QStringLiteral("mobilePlatform"), mobilePlatform);
    engine.rootContext()->setContextProperty(QStringLiteral("mobilePreview"), mobilePreview);
    engine.rootContext()->setContextProperty(QStringLiteral("platformId"), platformId);
    engine.rootContext()->setContextProperty(QStringLiteral("platformLabel"), platformLabel);
    engine.rootContext()->setContextProperty(QStringLiteral("screenshotScenario"),
        qEnvironmentVariable("TINYBLOOM_SCREENSHOT_SCENARIO"));
    engine.loadFromModule(QStringLiteral("TinyBloom"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) return -1;
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
