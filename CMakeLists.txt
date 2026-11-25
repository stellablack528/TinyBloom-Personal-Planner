cmake_minimum_required(VERSION 3.21)

project(TinyBloom VERSION 0.4.1 LANGUAGES CXX)
set(TINYBLOOM_RELEASE_VERSION "${PROJECT_VERSION}-beta.1")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Qml Sql Network QuickControls2 LinguistTools)
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0002 NEW)
qt_policy(SET QTP0004 NEW)

set(TINYBLOOM_SOURCES
    src/database/DatabaseManager.cpp
    src/database/DatabaseManager.h
    src/managers/SettingsManager.cpp
    src/managers/SettingsManager.h
    src/managers/GrowthManager.cpp
    src/managers/GrowthManager.h
    src/managers/FocusManager.cpp
    src/managers/FocusManager.h
    src/managers/TaskManager.cpp
    src/managers/TaskManager.h
    src/models/Task.cpp
    src/models/Task.h
    src/models/TaskListModel.cpp
    src/models/TaskListModel.h
    src/models/Subtask.h
    src/services/DataService.cpp
    src/services/DataService.h
    src/services/UpdateManager.cpp
    src/services/UpdateManager.h
)

qt_add_executable(TinyBloom src/main.cpp ${TINYBLOOM_SOURCES})
target_compile_definitions(TinyBloom PRIVATE TINYBLOOM_RELEASE_VERSION="${TINYBLOOM_RELEASE_VERSION}")

qt_add_resources(TinyBloom "tinybloom_app_assets"
    PREFIX "/"
    FILES
        resources/icons/app-icon.svg
)

qt_add_qml_module(TinyBloom
    URI TinyBloom
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/themes/Theme.qml
        qml/components/AppButton.qml
        qml/components/AppTextField.qml
        qml/components/ConfirmDialog.qml
        qml/components/DatePickerPopup.qml
        qml/components/EmptyState.qml
        qml/components/ImportConfirmDialog.qml
        qml/components/LongTermNodeDialog.qml
        qml/components/LongTermTaskDialog.qml
        qml/components/GardenPlant.qml
        qml/components/GrowthSummaryCard.qml
        qml/components/FocusCalendar.qml
        qml/components/DurationDialog.qml
        qml/components/ProgressCard.qml
        qml/components/SeedPickerDialog.qml
        qml/components/Sidebar.qml
        qml/components/MobileNavigationBar.qml
        qml/components/TaskCard.qml
        qml/components/TaskDialog.qml
        qml/components/XpToast.qml
        qml/components/UpdateDialog.qml
        qml/pages/GardenPage.qml
        qml/pages/LongTermPage.qml
        qml/pages/MobileLongTermPage.qml
        qml/pages/TodayPage.qml
        qml/pages/TasksPage.qml
        qml/pages/SettingsPage.qml
    RESOURCES
        resources/icons/leaf.svg
)

qt_add_translations(TinyBloom
    TS_FILES translations/tinybloom_zh_CN.ts
    RESOURCE_PREFIX "/i18n"
)

target_include_directories(TinyBloom PRIVATE src)
target_link_libraries(TinyBloom PRIVATE
    Qt6::Quick Qt6::Qml Qt6::Sql Qt6::Network Qt6::QuickControls2
)

if(ANDROID)
    # NDK r27 needs these flags explicitly for native libraries to run on
    # Android devices that use a 16 KiB memory page size.
    target_link_options(TinyBloom PRIVATE
        "-Wl,-z,max-page-size=16384"
        "-Wl,-z,common-page-size=16384"
    )
    set_target_properties(TinyBloom PROPERTIES
        QT_ANDROID_PACKAGE_NAME "app.tinybloom.mobile"
        QT_ANDROID_APP_NAME "TinyBloom"
        QT_ANDROID_APP_ICON "@drawable/icon"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packaging/android"
        QT_ANDROID_VERSION_CODE 40101
        QT_ANDROID_VERSION_NAME "${TINYBLOOM_RELEASE_VERSION}"
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 36
    )
endif()

if(IOS)
    set_target_properties(TinyBloom PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "app.tinybloom.mobile"
        MACOSX_BUNDLE_BUNDLE_VERSION "40101"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

if(WIN32)
    set(TINYBLOOM_DESKTOP_VERSION "${TINYBLOOM_RELEASE_VERSION}" CACHE STRING "Windows application version")
    set(TINYBLOOM_DESKTOP_FILE_VERSION "0,4,1,10" CACHE STRING "Windows numeric file version")
    configure_file(resources/DesktopVersion.h.in generated/DesktopVersion.h @ONLY)
    target_include_directories(TinyBloom PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set_property(SOURCE resources/tinybloom.rc APPEND PROPERTY OBJECT_DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/generated/DesktopVersion.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/sounds/focus-wind.wav"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/sounds/focus-glass.wav")
    target_sources(TinyBloom PRIVATE
        resources/tinybloom.rc
        src/platform/windows/DesktopIntegration.cpp
        src/platform/windows/DesktopIntegration.h
        src/platform/windows/FocusClip.cpp
        src/platform/windows/FocusClip.h
    )
    target_link_libraries(TinyBloom PRIVATE comctl32 shell32 user32 winmm ole32 mfuuid)
    set_target_properties(TinyBloom PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

include(CTest)
if(BUILD_TESTING AND NOT ANDROID AND NOT IOS)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(TinyBloomTests
        tests/tst_core.cpp
        ${TINYBLOOM_SOURCES}
    )
    target_include_directories(TinyBloomTests PRIVATE src)
    target_link_libraries(TinyBloomTests PRIVATE Qt6::Test Qt6::Sql Qt6::Network Qt6::Gui Qt6::Core)
    add_test(NAME TinyBloomCore COMMAND TinyBloomTests)
endif()
