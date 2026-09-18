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
