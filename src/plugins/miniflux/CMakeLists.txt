# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none

add_library(akregator_miniflux_plugin MODULE)

# Place the plugin in the namespaced subdirectory inside the build tree so that
# KPluginMetaData::findPlugins() locates it when running uninstalled from build-*/bin.
set_target_properties(
    akregator_miniflux_plugin
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY
            "${CMAKE_BINARY_DIR}/bin/pim6/akregator/accountplugins"
)

ecm_qt_declare_logging_category(akregator_miniflux_plugin
    HEADER miniflux_debug.h
    IDENTIFIER MINIFLUX_LOG
    CATEGORY_NAME org.kde.pim.akregator.miniflux
    DESCRIPTION "akregator miniflux plugin (akregator)"
    EXPORT AKREGATOR
)

target_sources(
    akregator_miniflux_plugin
    PRIVATE
        # Networking layer
        minifluxdata.h
        minifluxclient.h
        minifluxclient.cpp
        minifluxretriever.h
        minifluxretriever.cpp
        minifluxstatussync.h
        minifluxstatussync.cpp
        # Plugin & UI
        minifluxfeed.h
        minifluxfeed.cpp
        minifluxaccount.h
        minifluxaccount.cpp
        minifluxsyncjob.h
        minifluxsyncjob.cpp
        minifluxplugin.h
        minifluxplugin.cpp
        minifluxaccountdialog.h
        minifluxaccountdialog.cpp
)

ki18n_wrap_ui(akregator_miniflux_plugin minifluxaccountdialog.ui)

target_link_libraries(
    akregator_miniflux_plugin
    PRIVATE
        akregatorinterfaces
        akregatorprivate
        akregatorplugins
        KF6::CoreAddons
        KF6::I18n
        KF6::ConfigGui
        KF6::WidgetsAddons
        Qt::Network
        Qt::Xml
        Qt::Widgets
        KF6::Syndication
        qt6keychain
)

install(TARGETS akregator_miniflux_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/akregator/accountplugins)
