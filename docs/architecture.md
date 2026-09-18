# Architecture

TinyBloom v0.1 intentionally uses a small, explicit architecture.

## Layers

1. **QML presentation** renders pages and reusable controls. It contains view state only and invokes C++ operations.
2. **Application logic** is held by `TaskManager` and `SettingsManager`. They validate input, coordinate persistence, update models, and expose friendly errors.
3. **Models** use `QAbstractListModel` delegates rather than creating task objects manually in QML. Four lightweight views share one in-memory task snapshot.
4. **Database** is owned by one `DatabaseManager` connection. It enables SQLite foreign keys, creates the schema, and performs parameterized queries.
5. **Data service** serializes a versioned JSON document. Exports use `QSaveFile`; imports validate, create a timestamped safety backup beside the database, and use a transaction so failure leaves existing data intact.

## Data lifecycle

The database path comes from `QStandardPaths::AppDataLocation`. At startup, the schema is created idempotently and tasks are read once into `TaskManager`. Successful mutations update SQLite first, then update the in-memory snapshot and reset the small filtered models. Import is the only operation that intentionally reloads the snapshot.

`subtasks.task_id` is a foreign key with `ON DELETE CASCADE`, preventing orphan rows. Dates are stored as ISO calendar dates; timestamps are stored as UTC ISO 8601 strings.

For isolated automated checks, `TINYBLOOM_DATABASE_PATH` can point the executable at a temporary database. `TINYBLOOM_SCREENSHOT_PATH` asks a test run to save one rendered frame and exit. `TINYBLOOM_SCREENSHOT_SCENARIO` can select a stable UI state such as `tasks`, `settings-midnight`, `task-dialog`, `task-dialog-advanced`, or `date-picker`; normal launches use none of these hooks.

Interface text uses Qt Linguist translations. Simplified Chinese is the default for a new profile, while English uses the source strings. The selected language is stored in SQLite and `QQmlApplicationEngine::retranslate()` updates the live interface. `TINYBLOOM_LANGUAGE_OVERRIDE` exists only for deterministic localization smoke tests and persists the requested test language in the isolated database.

## Boundaries

QML never performs SQL or edits persistence state directly. The project does not introduce repositories per table, dependency-injection frameworks, or an event bus at this size. `DatabaseManager` can accept an alternate path for isolated tests without changing production path behavior.

## Extension points

- Additional themes can extend the centralized QML theme object.
- New task views can reuse `TaskListModel` scope/status matching.
- Schema evolution should add an explicit version table and migrations before changing existing columns.
- Future recurring tasks or ordering should be modelled in C++ and SQLite first, then exposed to QML.
