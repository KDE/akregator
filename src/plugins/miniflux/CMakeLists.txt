# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none

add_library(akregator_miniflux_plugin MODULE)

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
        Qt::Network
        Qt::Xml
        Qt::Widgets
        KF6::Syndication
)

if(TARGET KF6::Wallet)
    target_link_libraries(akregator_miniflux_plugin PRIVATE KF6::Wallet)
endif()

install(TARGETS akregator_miniflux_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/akregator/accountplugins)
