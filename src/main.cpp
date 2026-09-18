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
    taskManager.initialize();
    SettingsManager settingsManager(&database);
    settingsManager.load();
    DataService dataService(&database, &taskManager, &settingsManager);

    QQmlApplicationEngine engine;
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
