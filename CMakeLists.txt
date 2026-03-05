cmake_minimum_required(VERSION 3.20)
project(kforge-gui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Find Qt6 (falls back to Qt5) ─────────────────────────────────
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Widgets REQUIRED)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5")
else()
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
endif()

# ── Sources ──────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SidebarWidget.cpp
    src/BuildJobWidget.cpp
    src/OtherWidgets.cpp
)

set(HEADERS
    src/MainWindow.h
    src/SidebarWidget.h
    src/BuildJobWidget.h
    src/OtherWidgets.h
)

# ── Resources ─────────────────────────────────────────────────────
if(EXISTS "${CMAKE_SOURCE_DIR}/resources/resources.qrc")
    list(APPEND SOURCES resources/resources.qrc)
endif()

# ── Executable ───────────────────────────────────────────────────
if(APPLE)
    add_executable(kforge-gui MACOSX_BUNDLE ${SOURCES} ${HEADERS})
    set_target_properties(kforge-gui PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "KForge GUI"
        MACOSX_BUNDLE_GUI_IDENTIFIER "dev.khmerstack.kforge-gui"
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
elseif(WIN32)
    add_executable(kforge-gui WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(kforge-gui ${SOURCES} ${HEADERS})
endif()

# ── Link Qt ──────────────────────────────────────────────────────
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(kforge-gui PRIVATE Qt6::Core Qt6::Widgets)
else()
    target_link_libraries(kforge-gui PRIVATE Qt5::Core Qt5::Widgets)
endif()

# ── Install ──────────────────────────────────────────────────────
install(TARGETS kforge-gui
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
