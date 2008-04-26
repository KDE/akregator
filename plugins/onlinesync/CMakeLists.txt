include_directories( ${CMAKE_SOURCE_DIR}/akregator/interfaces )
include_directories( ${CMAKE_SOURCE_DIR} ) 
include_directories( ${KDE4_INCLUDE_DIR} )
include_directories( ${QT_INCLUDES} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/akregator/src )


########### next target ###############

set(kcm_akronlinesyncconfig_SRCS 
    akregator_config_onlinesync.cpp )

kde4_add_kcfg_files(kcm_akronlinesyncconfig_SRCS onlinesyncsettings.kcfgc)

kde4_add_ui_files( kcm_akronlinesyncconfig_SRCS settings_onlinesync.ui )
kde4_add_plugin(akregator_config_onlinesync ${kcm_akronlinesyncconfig_SRCS})

target_link_libraries( akregator_config_onlinesync akregatorinterfaces ${KDE4_KIO_LIBS} )

install( FILES akregator_config_onlinesync.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_onlinesync  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(akregator_onlinesync_plugin_SRCS 
    feedsync.cpp
    subscriptionlist.cpp 
    akregator.cpp 
    googlereader.cpp 
    opml.cpp 
    ui/configurationdialog.cpp 
    ui/configurationdialogadd.cpp 
    onlinesyncplugin.cpp )


kde4_add_ui_files(akregator_onlinesync_plugin_SRCS ui/configurationdialog.ui)
kde4_add_ui_files(akregator_onlinesync_plugin_SRCS ui/configurationdialogadd.ui)

kde4_add_library(akregator_onlinesync_plugin ${akregator_onlinesync_plugin_SRCS})

kde4_add_kcfg_files(akregator_onlinesync_plugin_SRCS onlinesyncsettings.kcfgc)

kde4_add_plugin(akregator_onlinesync_plugin ${akregator_onlinesync_plugin_SRCS})

target_link_libraries(akregator_onlinesync_plugin ${KDE4_KDE3SUPPORT_LIBS})
target_link_libraries(akregator_onlinesync_plugin ${KDE4_KDEUI_LIBS})
target_link_libraries( akregator_onlinesync_plugin akregatorinterfaces ${KDE4_KIO_LIBS} )

install( FILES akregator_onlinesync_plugin.desktop  DESTINATION ${DATA_INSTALL_DIR}/akregator/kpartplugins)
install(TARGETS akregator_onlinesync_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES akregator_onlinesync_plugin.rc DESTINATION ${DATA_INSTALL_DIR}/akregator/kpartplugins)


