cmake_minimum_required(VERSION 3.21)

project(TinyBloom VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Qml Sql QuickControls2 LinguistTools)
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

set(TINYBLOOM_SOURCES
    src/database/DatabaseManager.cpp
    src/database/DatabaseManager.h
    src/managers/SettingsManager.cpp
    src/managers/SettingsManager.h
    src/managers/TaskManager.cpp
    src/managers/TaskManager.h
    src/models/Task.cpp
    src/models/Task.h
    src/models/TaskListModel.cpp
    src/models/TaskListModel.h
    src/models/Subtask.h
    src/services/DataService.cpp
    src/services/DataService.h
)

qt_add_executable(TinyBloom src/main.cpp ${TINYBLOOM_SOURCES})

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
        qml/components/ProgressCard.qml
        qml/components/Sidebar.qml
        qml/components/TaskCard.qml
        qml/components/TaskDialog.qml
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
    Qt6::Quick Qt6::Qml Qt6::Sql Qt6::QuickControls2
)

if(WIN32)
    target_sources(TinyBloom PRIVATE resources/tinybloom.rc)
    set_target_properties(TinyBloom PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(TinyBloomTests
        tests/tst_core.cpp
        ${TINYBLOOM_SOURCES}
    )
    target_include_directories(TinyBloomTests PRIVATE src)
    target_link_libraries(TinyBloomTests PRIVATE Qt6::Test Qt6::Sql Qt6::Core)
    add_test(NAME TinyBloomCore COMMAND TinyBloomTests)
endif()
