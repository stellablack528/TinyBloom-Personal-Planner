# TinyBloom

**Small Steps, Real Progress.**

TinyBloom is a free, open-source, local-first desktop task manager for turning overwhelming goals into approachable next steps. Version 0.1 focuses on a calm daily workflow, reliable offline storage, and a polished interface without accounts, ads, subscriptions, or pressure-driven overdue warnings.

## Current features (v0.1.0)

- Today dashboard with daily progress
- Create, edit, complete, restore, and delete tasks
- Small steps (subtasks) with completion progress
- Due dates and automatic Today / Tomorrow / Later classification
- Low, medium, and high priority; estimates; categories
- Search and All / Active / Completed filters
- SQLite persistence in the platform application-data directory
- Mint Garden and Midnight themes, persisted across restarts
- Reduced-motion preference and default-duration setting
- Transactional JSON import and atomic JSON export
- Keyboard shortcuts: `Ctrl+N` to add, `Ctrl+F` to search, `Esc` to close dialogs
- Friendly validation, storage errors, and empty states

## Technology

C++20 · Qt 6 · Qt Quick/QML · Qt SQL/SQLite · CMake

The application follows a straightforward flow:

```text
QML presentation → C++ application logic → DatabaseManager → SQLite
```

## Build

Requirements:

- Qt 6.5 or newer with Quick, QML, Quick Controls 2, SQL, and Test modules
- CMake 3.21 or newer
- A C++20 compiler supported by your Qt installation
- Ninja (recommended)

On Windows with the Qt online installer, open a matching Qt command prompt, then run:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\mingw_64" -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Run `build/TinyBloom.exe`. When deploying outside the Qt development environment, use Qt's `windeployqt` tool to collect runtime libraries.

On Linux, install the matching Qt 6 development packages and use the same CMake commands with your Qt prefix (often no explicit prefix is needed).

## Project structure

```text
src/database/     SQLite lifecycle and queries
src/managers/     task and settings application logic
src/models/       task data and QML list model
src/services/     JSON import/export
qml/components/   reusable interface components
qml/pages/        Today, Tasks, and Settings pages
qml/themes/       application-wide color system
tests/            core integration tests
docs/             architecture and development notes
```

## Data and privacy

TinyBloom uses no network services. Its database is stored under the operating system's standard per-user application-data directory. JSON export is only performed when the user chooses a destination.

## Roadmap

Potential post-v0.1 work includes recurring tasks, drag-and-drop ordering, a lightweight weekly review, accessible localization, and optional task breakdown assistance. Cloud accounts, collaboration, gamification, and complex calendars are intentionally outside v0.1.

## License

[MIT](LICENSE) — free and open source.

