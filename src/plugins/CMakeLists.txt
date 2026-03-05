# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
add_library(akregatorplugins STATIC)

ecm_qt_declare_logging_category(akregatorplugins
    HEADER akregator_plugin_debug.h
    IDENTIFIER AKREGATOR_PLUGIN_LOG
    CATEGORY_NAME org.kde.pim.akregator.plugins
    DESCRIPTION "akregator plugins (akregator)"
    EXPORT AKREGATOR
)

target_sources(
    akregatorplugins
    PRIVATE
        pluginmanager.h
        pluginmanager.cpp
)

target_link_libraries(
    akregatorplugins
    akregatorinterfaces
    KF6::CoreAddons
)

target_include_directories(akregatorplugins PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

add_subdirectory(miniflux)
