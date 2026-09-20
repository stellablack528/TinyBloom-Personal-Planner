# Development Log

## MinGW compiler could not compile CMake's probe

**Problem:** CMake found `g++.exe` but its first compiler check failed without a diagnostic message.

**Cause:** The compiler executable was supplied directly, but the MinGW `bin` directory was absent from `PATH`, so compiler helper executables could not be resolved.

**Solution:** Prepend the matching MinGW, Qt, and Ninja directories to `PATH` before configuring.

**Result:** CMake identified GNU C++ 13.1.0 and completed its compiler feature checks.

## Quick-add tasks failed the database constraint

**Problem:** Core tests showed that creating or updating a task with an omitted description failed the `tasks.description` `NOT NULL` constraint.

**Cause:** A default-constructed Qt `QString` is null rather than merely empty. Binding it through Qt SQL produced SQL `NULL`, even though an empty description is valid application input.

**Solution:** Normalize nullable optional text fields to explicit empty strings at the database binding boundary.

**Result:** Title-only quick-add tasks persist correctly while the schema keeps its integrity constraint.

## Product self-check found incomplete task views and refresh edges

**Problem:** The first working UI had C++ models for Tomorrow and Later, but the Tasks page did not expose those scopes. Imported settings and subtask changes also needed an explicit refresh path in open views.

**Cause:** The initial pass concentrated on storage and CRUD paths; a later user-flow review exposed presentation-level gaps that compilation could not detect.

**Solution:** Add All/Today/Tomorrow/Later scope controls, a model count property, immediate settings change notifications after import, and dialog task-data refresh on manager changes. Invalid non-empty date strings are now rejected rather than silently becoming undated.

**Result:** The required classifications are directly usable, and open UI state stays consistent with successful persistence changes.

## Offscreen visual check rendered text as boxes

**Problem:** The first automated screenshot showed correct geometry but every glyph as a square.

**Cause:** Qt's headless `offscreen` Windows platform plugin did not enumerate the system fonts in this environment.

**Solution:** Repeat the screenshot smoke test using the native Windows platform and software Quick renderer against an isolated database.

**Result:** The native render showed the expected Segoe-style text, icons, Mint Garden colors, spacing, and empty state. The application exited cleanly after saving the frame.

## First release needed a real language switch

**Problem:** The first release candidate only exposed English strings, while the intended initial audience needed Simplified Chinese by default and an English option.

**Cause:** v0.1 originally centralized colors and settings but had no localization layer.

**Solution:** Integrate Qt Linguist, translate all 127 user-facing strings, persist a `language` setting in SQLite, retranslate QML at runtime, refresh translated model roles, and format dates with the selected locale.

**Result:** Fresh profiles start in Simplified Chinese. Users can switch to English in Settings, and the selection survives a restart.

## User testing exposed release-quality usability gaps

**Problem:** The first public build worked end to end, but ordinary-user testing found that imports could replace data without warning, the collapsed task dialog wasted space, date entry required typing a strict format, some filters were cramped, and the Windows executable had no recognizable icon.

**Cause:** The first release focused on complete task flows and persistence. Destructive-action safeguards, platform packaging details, contrast, keyboard interaction, and compact states needed a second pass with the packaged application rather than only code-level tests.

**Solution:** Add an import confirmation and timestamped automatic JSON backup, a localized date picker with Today/Tomorrow shortcuts, responsive dialog sizing, explicit filter widths, stronger Midnight contrast, keyboard and accessibility metadata for custom controls, and a multi-resolution Windows icon embedded in the executable.

**Result:** v0.1.1 preserves current data before every import, exposes safer and faster date entry, renders the affected screens cleanly at the default window size, and presents TinyBloom with its own icon and Windows version metadata.

## Growth rewards needed both encouragement and consequence

**Problem:** A purely decorative garden could provide a pleasant reward but would not help users notice when they had stopped acting. At the same time, deleting XP or killing the plant would make returning feel punishing.

**Cause:** Long-term progress and current care are different signals. Treating both as one score either makes rest consequence-free or destroys earned progress.

**Solution:** Keep XP and levels permanent, while tracking a separate 0–100 vitality value. Completing a task or small step grants a one-time XP reward and restores vitality; each inactive calendar day removes 15 vitality. The plant changes from healthy to tired to wilted, and can always recover through another completed step.

**Result:** The garden creates visible accountability without allowing repeated completion toggles to farm rewards or making a lapse irreversible.

## Desktop release metadata needed a cross-device boundary

**Problem:** The v0.2 interface, Windows metadata, archive name, and JSON export described TinyBloom generically. That would make desktop and future mobile builds hard to distinguish, while duplicate imported task identifiers could attach small steps to the wrong task.

**Solution:** Brand this build as TinyBloom Desktop without changing the existing application data directory, add explicit application/platform/schema metadata to exports, and reject duplicate or orphaned identities before an import transaction starts.

**Result:** Users can identify the Windows desktop build immediately, future clients have a stable compatibility marker, and malformed cross-device data is rejected without touching existing data.
