#include "DesktopIntegration.h"
#include "FocusClip.h"
#include "managers/SettingsManager.h"

#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QKeySequence>
#include <QSaveFile>
#include <QScopedValueRollback>

#include <commctrl.h>
#include <mmsystem.h>
#include <shellapi.h>

namespace {
constexpr UINT TrayIconId = 1;
constexpr UINT_PTR WindowSubclassId = 1;
constexpr UINT TrayCallbackMessage = WM_APP + 1;
constexpr UINT OpenWindowCommand = 1;
constexpr UINT ExitCommand = 2;
constexpr int FirstRestoreHotkeyId = 1;
constexpr int SecondRestoreHotkeyId = 2;

bool isRestoreShortcut(QKeyCombination combination)
{
    const auto modifiers = combination.keyboardModifiers();
    const auto key = combination.key();
    const auto base = Qt::ControlModifier | Qt::ShiftModifier;
    const bool functionKey = key >= Qt::Key_F1 && key <= Qt::Key_F11;
    const bool letterOrDigit = (key >= Qt::Key_A && key <= Qt::Key_Z)
        || (key >= Qt::Key_0 && key <= Qt::Key_9);
    return (modifiers == base && functionKey)
        || (modifiers == (base | Qt::AltModifier) && (letterOrDigit || functionKey));
}

bool registerRestoreShortcut(HWND window, int id, QKeyCombination combination)
{
    const auto key = combination.key();
    const UINT virtualKey = key >= Qt::Key_F1 ? VK_F1 + (key - Qt::Key_F1) : UINT(key);
    const UINT modifiers = MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT
        | (combination.keyboardModifiers().testFlag(Qt::AltModifier) ? MOD_ALT : 0);
    return RegisterHotKey(window, id, modifiers, virtualKey);
}

NOTIFYICONDATAW iconData(WId window)
{
    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = reinterpret_cast<HWND>(window);
    data.uID = TrayIconId;
    return data;
}

template<std::size_t Capacity>
void copyTrayText(wchar_t (&buffer)[Capacity], const QString &text)
{
    text.left(static_cast<qsizetype>(Capacity - 1)).toWCharArray(buffer);
}
}

DesktopIntegration::DesktopIntegration(SettingsManager &settings, const QString &customSoundPath)
    : m_settings(settings), m_customSoundPath(customSoundPath)
{
    m_cleanupTimer.setSingleShot(true);
    connect(&m_cleanupTimer, &QTimer::timeout, this, &DesktopIntegration::removeTrayIcon);
    connect(&settings, &SettingsManager::restoreShortcutChanged, this, [this] {
        if (!m_savingRestoreShortcut) applySavedRestoreShortcut();
    });
}

DesktopIntegration::~DesktopIntegration()
{
    m_audioImportPool.waitForDone();
    unregisterRestoreShortcut();
    removeTrayIcon();
    if (m_windowId)
        RemoveWindowSubclass(reinterpret_cast<HWND>(m_windowId), &DesktopIntegration::windowProcedure, WindowSubclassId);
    if (m_hiddenToTray) QGuiApplication::setQuitOnLastWindowClosed(m_previousQuitOnLastWindowClosed);
}

void DesktopIntegration::setWindow(QWindow *window)
{
    if (!window || QGuiApplication::platformName() != QStringLiteral("windows")) return;
    const WId id = window->winId();
    if (!SetWindowSubclass(reinterpret_cast<HWND>(id), &DesktopIntegration::windowProcedure,
            WindowSubclassId, reinterpret_cast<DWORD_PTR>(this))) return;
    m_window = window;
    m_windowId = id;
    m_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    applySavedRestoreShortcut();
}

LRESULT CALLBACK DesktopIntegration::windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam,
    UINT_PTR subclassId, DWORD_PTR referenceData)
{
    auto *desktop = reinterpret_cast<DesktopIntegration *>(referenceData);
    if (message == WM_NCDESTROY) {
        desktop->unregisterRestoreShortcut();
        desktop->removeTrayIcon();
        desktop->m_windowId = 0;
        RemoveWindowSubclass(window, &DesktopIntegration::windowProcedure, subclassId);
    } else if (desktop->handleWindowMessage(message, wParam, lParam)) {
        return 0;
    }
    return DefSubclassProc(window, message, wParam, lParam);
}

bool DesktopIntegration::handleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!m_window) return false;
    if (message == WM_HOTKEY && m_restoreHotkeyId && wParam == WPARAM(m_restoreHotkeyId)) {
        restoreWindow();
        return true;
    }
    if (m_taskbarCreated && message == m_taskbarCreated) {
        m_trayIconAdded = false;
        m_cleanupTimer.stop();
        if (m_hiddenToTray && !addTrayIcon()) restoreWindow();
        return false;
    }

    // Only the window's Close command is intercepted. A normal WM_CLOSE
    // from an installer or another application still closes the window.
    if (message == WM_SYSCOMMAND && (wParam & 0xfff0) == SC_CLOSE && m_settings.closeToTray()) {
        hideToTray();
        return true;
    }
    if (message == WM_CLOSE && m_hiddenToTray) {
        emit exitRequested();
        return true;
    }

    if (message != TrayCallbackMessage || !m_trayIconAdded || HIWORD(lParam) != TrayIconId) return false;
    switch (LOWORD(lParam)) {
    case WM_LBUTTONUP:
    case NIN_SELECT:
    case NIN_KEYSELECT:
    case NIN_BALLOONUSERCLICK:
        restoreWindow();
        break;
    case WM_CONTEXTMENU:
        showTrayMenu();
        break;
    default:
        return false;
    }
    return true;
}

QString DesktopIntegration::restoreShortcutText() const
{
    const auto sequence = QKeySequence::fromString(m_settings.restoreShortcut(), QKeySequence::PortableText);
    return sequence.count() == 1 && isRestoreShortcut(sequence[0]) ? sequence.toString() : QString();
}

QString DesktopIntegration::shortcutCandidateText(int key, int modifiers) const
{
    const QKeyCombination combination(Qt::KeyboardModifiers::fromInt(modifiers), Qt::Key(key));
    return isRestoreShortcut(combination) ? QKeySequence(combination).toString(QKeySequence::PortableText) : QString();
}

QString DesktopIntegration::saveRestoreShortcut(const QString &shortcut)
{
    const auto sequence = QKeySequence::fromString(shortcut.trimmed(), QKeySequence::PortableText);
    if (sequence.count() != 1 || !isRestoreShortcut(sequence[0]))
        return tr("Choose a supported shortcut.");
    if (!m_windowId)
        return tr("The shortcut is unavailable. Reopen TinyBloom and try again.");

    const auto stored = sequence.toString(QKeySequence::PortableText);
    if (m_restoreHotkeyId && stored == m_settings.restoreShortcut()) return {};

    const auto window = reinterpret_cast<HWND>(m_windowId);
    const int candidateId = m_restoreHotkeyId == FirstRestoreHotkeyId ? SecondRestoreHotkeyId : FirstRestoreHotkeyId;
    // Register the new combination first, so a conflict leaves the old shortcut intact.
    if (!registerRestoreShortcut(window, candidateId, sequence[0])) {
        if (GetLastError() == ERROR_HOTKEY_ALREADY_REGISTERED)
            return tr("This shortcut is already in use. Choose another combination.");
        return tr("Windows couldn't register this shortcut. Choose another combination.");
    }

    const QScopedValueRollback saving(m_savingRestoreShortcut, true);
    if (!m_settings.setRestoreShortcut(stored)) {
        UnregisterHotKey(window, candidateId);
        return tr("Couldn't save the shortcut. Please try again.");
    }
    unregisterRestoreShortcut();
    m_restoreHotkeyId = candidateId;
    emit restoreShortcutChanged();
    return {};
}

QString DesktopIntegration::clearRestoreShortcut()
{
    const QScopedValueRollback saving(m_savingRestoreShortcut, true);
    if (!m_settings.setRestoreShortcut(QString()))
        return tr("Couldn't save the shortcut. Please try again.");
    unregisterRestoreShortcut();
    emit restoreShortcutChanged();
    return {};
}

void DesktopIntegration::applySavedRestoreShortcut()
{
    unregisterRestoreShortcut();
    const auto sequence = QKeySequence::fromString(m_settings.restoreShortcut(), QKeySequence::PortableText);
    if (m_windowId && sequence.count() == 1 && isRestoreShortcut(sequence[0])
        && registerRestoreShortcut(reinterpret_cast<HWND>(m_windowId), FirstRestoreHotkeyId, sequence[0]))
        m_restoreHotkeyId = FirstRestoreHotkeyId;
    emit restoreShortcutChanged();
}

void DesktopIntegration::unregisterRestoreShortcut()
{
    if (m_windowId && m_restoreHotkeyId)
        UnregisterHotKey(reinterpret_cast<HWND>(m_windowId), m_restoreHotkeyId);
    m_restoreHotkeyId = 0;
}

void DesktopIntegration::hideToTray()
{
    if (m_hiddenToTray) return;
    if (!addTrayIcon()) {
        emit trayUnavailable(tr("The system tray is unavailable. TinyBloom stays open."));
        return;
    }
    m_cleanupTimer.stop();
    m_previousWindowState = m_window->windowState();
    m_previousQuitOnLastWindowClosed = QGuiApplication::quitOnLastWindowClosed();
    QGuiApplication::setQuitOnLastWindowClosed(false);
    m_hiddenToTray = true;
    m_window->hide();

    if (!m_trayHintShownThisRun && !m_settings.closeToTrayHintShown()) {
        m_trayHintShownThisRun = true;
        if (showTrayNotification(QStringLiteral("TinyBloom"),
                tr("Still running in the system tray. Click the leaf near the clock to reopen. It may be under the ^ arrow."))) {
            m_settings.markCloseToTrayHintShown();
        }
    }
}

void DesktopIntegration::restoreWindow()
{
    if (!m_window) return;
    if (m_hiddenToTray) {
        m_hiddenToTray = false;
        m_window->setWindowState(m_previousWindowState == Qt::WindowMinimized
            ? Qt::WindowNoState : m_previousWindowState);
        m_window->setVisible(true);
        QGuiApplication::setQuitOnLastWindowClosed(m_previousQuitOnLastWindowClosed);
    } else if (m_window->windowState() == Qt::WindowMinimized) {
        ShowWindow(reinterpret_cast<HWND>(m_windowId), SW_RESTORE);
    }
    m_window->requestActivate();
    removeTrayIcon();
}

void DesktopIntegration::showTrayMenu()
{
    const HMENU menu = CreatePopupMenu();
    if (!menu) return;
    const auto openLabel = tr("Open TinyBloom").toStdWString();
    const auto exitLabel = tr("Exit TinyBloom").toStdWString();
    AppendMenuW(menu, MF_STRING, OpenWindowCommand, openLabel.c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ExitCommand, exitLabel.c_str());
    SetMenuDefaultItem(menu, OpenWindowCommand, FALSE);
    const HWND window = reinterpret_cast<HWND>(m_windowId);
    const bool alignRight = GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0;
    NOTIFYICONIDENTIFIER identifier{};
    identifier.cbSize = sizeof(identifier);
    identifier.hWnd = window;
    identifier.uID = TrayIconId;
    RECT iconRect{};
    POINT anchor{};
    if (SUCCEEDED(Shell_NotifyIconGetRect(&identifier, &iconRect))) {
        anchor.x = alignRight ? iconRect.right : iconRect.left;
        anchor.y = iconRect.top;
    } else {
        GetCursorPos(&anchor);
    }
    SetForegroundWindow(window);
    const auto alignment = alignRight ? TPM_RIGHTALIGN : TPM_LEFTALIGN;
    const auto command = TrackPopupMenu(menu, alignment | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON,
        anchor.x, anchor.y, 0, window, nullptr);
    PostMessageW(window, WM_NULL, 0, 0);
    DestroyMenu(menu);
    if (command == OpenWindowCommand) restoreWindow();
    if (command == ExitCommand) emit exitRequested();
    if (command == 0 && m_trayIconAdded) {
        auto data = iconData(m_windowId);
        Shell_NotifyIconW(NIM_SETFOCUS, &data);
    }
}

bool DesktopIntegration::addTrayIcon()
{
    if (m_trayIconAdded) return true;
    if (!m_windowId) return false;
    auto data = iconData(m_windowId);
    const QSize iconSize(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
    const HICON icon = QGuiApplication::windowIcon().pixmap(iconSize).toImage().toHICON();
    if (!icon) return false;
    data.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
    data.hIcon = icon;
    data.uCallbackMessage = TrayCallbackMessage;
    copyTrayText(data.szTip, QGuiApplication::applicationDisplayName());
    m_trayIconAdded = Shell_NotifyIconW(NIM_ADD, &data);
    DestroyIcon(icon);
    if (!m_trayIconAdded) return false;
    data.uVersion = NOTIFYICON_VERSION_4;
    if (Shell_NotifyIconW(NIM_SETVERSION, &data)) return true;
    removeTrayIcon();
    return false;
}

void DesktopIntegration::removeTrayIcon()
{
    m_cleanupTimer.stop();
    if (!m_trayIconAdded) return;
    auto data = iconData(m_windowId);
    Shell_NotifyIconW(NIM_DELETE, &data);
    m_trayIconAdded = false;
}

bool DesktopIntegration::showTrayNotification(const QString &title, const QString &message)
{
    auto data = iconData(m_windowId);
    data.uFlags = NIF_INFO;
    data.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
    copyTrayText(data.szInfoTitle, title);
    copyTrayText(data.szInfo, message);
    return Shell_NotifyIconW(NIM_MODIFY, &data);
}

bool DesktopIntegration::customFocusSoundAvailable() const
{
    return QFileInfo(m_customSoundPath).isFile();
}

void DesktopIntegration::importFocusSound(const QUrl &file)
{
    if (m_customSoundImporting) return;
    const QFileInfo source(file.toLocalFile());
    const QString extension = source.suffix().toLower();
    if (!file.isLocalFile() || !source.isFile() || source.size() > 32 * 1024 * 1024
        || (extension != QStringLiteral("wav") && extension != QStringLiteral("mp3"))) {
        emit soundUnavailable(tr("Choose a local WAV or MP3 file under 32 MB."));
        return;
    }
    if (m_customSoundPath.isEmpty()) {
        emit soundUnavailable(tr("Couldn't save the custom focus sound."));
        return;
    }

    m_customSoundImporting = true;
    emit customSoundImportingChanged();
    m_audioImportPool.start([this, path = source.absoluteFilePath()] {
        const QByteArray clip = prepareFocusClip(path);
        QMetaObject::invokeMethod(this, [this, clip] {
            m_customSoundImporting = false;
            emit customSoundImportingChanged();
            if (clip.isEmpty()) {
                emit soundUnavailable(tr("Couldn't read that audio. Try a WAV or MP3 with sound in the first five seconds."));
                return;
            }
            QSaveFile output(m_customSoundPath);
            if (!output.open(QIODevice::WriteOnly) || output.write(clip) != clip.size() || !output.commit()) {
                emit soundUnavailable(tr("Couldn't save the custom focus sound."));
                return;
            }
            m_settings.setFocusEndSoundStyle(QStringLiteral("custom"));
            emit customFocusSoundChanged();
        }, Qt::QueuedConnection);
    });
}

void DesktopIntegration::playFocusEndSound()
{
    const QString style = m_settings.focusEndSoundStyle();
    BOOL played;
    if (style == QStringLiteral("custom") && customFocusSoundAvailable()) {
        const auto path = QDir::toNativeSeparators(m_customSoundPath).toStdWString();
        played = PlaySoundW(path.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    } else {
        const auto resource = style == QStringLiteral("glass") ? L"FOCUSSOUND_GLASS" : L"FOCUSSOUND_WIND";
        played = PlaySoundW(resource, GetModuleHandleW(nullptr), SND_RESOURCE | SND_ASYNC | SND_NODEFAULT);
    }
    if (!played)
        emit soundUnavailable(tr("Couldn't play the focus sound. Check your audio output."));
}

bool DesktopIntegration::notifyFocusEnd(const QString &title, const QString &message)
{
    if (m_settings.focusEndSound()) playFocusEndSound();
    if (!m_window || (m_window->isVisible() && m_window->isActive() && m_window->visibility() != QWindow::Minimized))
        return false;

    if (!addTrayIcon()) return false;
    const bool delivered = showTrayNotification(title, message);
    if (!m_hiddenToTray) {
        if (delivered) m_cleanupTimer.start(30'000);
        else removeTrayIcon();
    }
    return delivered;
}
