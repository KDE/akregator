# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
add_library(akregatorplugins STATIC)
set_target_properties(
    akregatorplugins
    PROPERTIES
        POSITION_INDEPENDENT_CODE
            ON
)

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
        addaccountwizard.h
        addaccountwizard.cpp
        pluginmanager.h
        pluginmanager.cpp
)

target_link_libraries(
    akregatorplugins
    akregatorinterfaces
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
    Qt::Widgets
)

target_include_directories(akregatorplugins PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

add_subdirectory(miniflux)
