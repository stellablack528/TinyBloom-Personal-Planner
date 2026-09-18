#include "database/DatabaseManager.h"
#include "managers/SettingsManager.h"
#include "managers/TaskManager.h"
#include "services/DataService.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("TinyBloom"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("tinybloom.app"));
    QCoreApplication::setApplicationName(QStringLiteral("TinyBloom"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    DatabaseManager database;
    const QString databaseOverride = qEnvironmentVariable("TINYBLOOM_DATABASE_PATH");
    if (!database.initializeDatabase(databaseOverride)) qCritical().noquote() << database.lastError();
    TaskManager taskManager(&database);
    SettingsManager settingsManager(&database);
    settingsManager.load();
    const QString languageOverride = qEnvironmentVariable("TINYBLOOM_LANGUAGE_OVERRIDE");
    if (!languageOverride.isEmpty()) settingsManager.setLanguage(languageOverride);
    DataService dataService(&database, &taskManager, &settingsManager);

    QQmlApplicationEngine engine;
    QTranslator translator;
    const auto applyLanguage = [&app, &engine, &translator, &settingsManager, &taskManager](const bool retranslate) {
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
        }
    };
    applyLanguage(false);
    taskManager.initialize();
    QObject::connect(&settingsManager, &SettingsManager::languageChanged, &app,
        [&applyLanguage] { applyLanguage(true); });
    engine.rootContext()->setContextProperty(QStringLiteral("taskManager"), &taskManager);
    engine.rootContext()->setContextProperty(QStringLiteral("settingsManager"), &settingsManager);
    engine.rootContext()->setContextProperty(QStringLiteral("dataService"), &dataService);
    engine.rootContext()->setContextProperty(QStringLiteral("databaseReady"), database.isOpen());
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
