#pragma once

#include <QObject>
#include <QPointer>
#include <QThreadPool>
#include <QTimer>
#include <QUrl>
#include <QWindow>
#include <QtCore/qt_windows.h>

class SettingsManager;

class DesktopIntegration final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool customFocusSoundAvailable READ customFocusSoundAvailable NOTIFY customFocusSoundChanged)
    Q_PROPERTY(bool customSoundImporting READ customSoundImporting NOTIFY customSoundImportingChanged)
    Q_PROPERTY(QString restoreShortcutText READ restoreShortcutText NOTIFY restoreShortcutChanged)
    Q_PROPERTY(bool restoreShortcutActive READ restoreShortcutActive NOTIFY restoreShortcutChanged)

public:
    explicit DesktopIntegration(SettingsManager &settings, const QString &customSoundPath = {});
    ~DesktopIntegration() override;

    void setWindow(QWindow *window);
    bool customFocusSoundAvailable() const;
    bool customSoundImporting() const { return m_customSoundImporting; }
    QString restoreShortcutText() const;
    bool restoreShortcutActive() const { return m_restoreHotkeyId != 0; }
    Q_INVOKABLE QString shortcutCandidateText(int key, int modifiers) const;
    Q_INVOKABLE QString saveRestoreShortcut(const QString &shortcut);
    Q_INVOKABLE QString clearRestoreShortcut();
    Q_INVOKABLE void importFocusSound(const QUrl &file);
    Q_INVOKABLE void playFocusEndSound();
    Q_INVOKABLE bool notifyFocusEnd(const QString &title, const QString &message);

signals:
    void trayUnavailable(const QString &message);
    void soundUnavailable(const QString &message);
    void customFocusSoundChanged();
    void customSoundImportingChanged();
    void restoreShortcutChanged();
    void exitRequested();

private:
    static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam,
        UINT_PTR subclassId, DWORD_PTR referenceData);
    bool handleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void hideToTray();
    void restoreWindow();
    void applySavedRestoreShortcut();
    void unregisterRestoreShortcut();
    void showTrayMenu();
    bool addTrayIcon();
    void removeTrayIcon();
    bool showTrayNotification(const QString &title, const QString &message);

    SettingsManager &m_settings;
    QString m_customSoundPath;
    QThreadPool m_audioImportPool;
    QPointer<QWindow> m_window;
    QTimer m_cleanupTimer;
    WId m_windowId = 0;
    unsigned int m_taskbarCreated = 0;
    int m_restoreHotkeyId = 0;
    bool m_savingRestoreShortcut = false;
    Qt::WindowState m_previousWindowState = Qt::WindowNoState;
    bool m_trayIconAdded = false;
    bool m_hiddenToTray = false;
    bool m_previousQuitOnLastWindowClosed = true;
    bool m_trayHintShownThisRun = false;
    bool m_customSoundImporting = false;
};
