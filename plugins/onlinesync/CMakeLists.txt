include_directories(
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/src
  ${CMAKE_SOURCE_DIR}
  
  
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(kcm_akronlinesyncconfig_SRCS
  akregator_config_onlinesync.cpp
  ui/configurationdialog.cpp
  ui/configurationwidget.cpp
  ui/configurationdialogadd.cpp
)

kconfig_add_kcfg_files(kcm_akronlinesyncconfig_SRCS onlinesyncsettings.kcfgc)

ki18n_wrap_ui(kcm_akronlinesyncconfig_SRCS ui/configurationwidget.ui)
ki18n_wrap_ui(kcm_akronlinesyncconfig_SRCS ui/configurationdialogadd.ui)

add_library(akregator_config_onlinesync MODULE ${kcm_akronlinesyncconfig_SRCS})

target_link_libraries(akregator_config_onlinesync
  akregatorinterfaces
  KF5::KIOCore
  KF5::Parts
)

install(FILES akregator_config_onlinesync.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_onlinesync DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(akregator_onlinesync_plugin_SRCS
  sync/feedsync.cpp
  sync/subscriptionlist.cpp
  sync/akregator.cpp
  sync/googlereader.cpp
  sync/opml.cpp
  ui/configurationdialog.cpp
  ui/configurationwidget.cpp
  ui/configurationdialogadd.cpp
  onlinesyncplugin.cpp
)

ki18n_wrap_ui(akregator_onlinesync_plugin_SRCS
  ui/configurationwidget.ui
  ui/configurationdialogadd.ui
)

kconfig_add_kcfg_files(akregator_onlinesync_plugin_SRCS onlinesyncsettings.kcfgc)

add_library(akregator_onlinesync_plugin MODULE ${akregator_onlinesync_plugin_SRCS})

target_link_libraries(akregator_onlinesync_plugin
  akregatorinterfaces
  KF5::KIOCore
  KF5::Parts
)

install(FILES akregator_onlinesync_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_onlinesync_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES akregator_onlinesync_plugin.rc DESTINATION ${KDE_INSTALL_DATADIR}/akregator_onlinesync_plugin)
