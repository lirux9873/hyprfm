cmake_minimum_required(VERSION 3.20)
project(hyprfm VERSION 0.6.1 LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    message(FATAL_ERROR "This source tree targets FreeBSD. Configure and build on FreeBSD.")
endif()
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation prefix" FORCE)
endif()
include(GNUInstallDirs)
foreach(required_file src/qml/Quill/Theme.qml src/qml/icons/IconCheck.qml)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${required_file}")
        message(FATAL_ERROR "Missing QML submodules. Run: git submodule update --init --recursive")
    endif()
endforeach()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0 gio-unix-2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 DBus Svg Network Concurrent)
find_package(KF6WindowSystem QUIET)

add_subdirectory(src)

# Install themes
install(DIRECTORY themes/ DESTINATION ${CMAKE_INSTALL_DATADIR}/hyprfm/themes FILES_MATCHING PATTERN "*.toml")

# Install the QML fallback alongside the compiled resources.
install(DIRECTORY ${CMAKE_BINARY_DIR}/src/HyprFM/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/hyprfm/HyprFM
    FILES_MATCHING
        PATTERN "*.qml"
        PATTERN "qmldir"
        PATTERN "*.qmltypes")


option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

install(FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs
    FILES_MATCHING PATTERN "*.md" PATTERN "*.png" PATTERN "*.gif")
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/hyprfm)
