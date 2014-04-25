include_directories(
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/src
  ${CMAKE_SOURCE_DIR}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
)

########### next target ###############

set(kcm_akronlinesyncconfig_SRCS
  akregator_config_onlinesync.cpp
  ui/configurationdialog.cpp
  ui/configurationwidget.cpp
  ui/configurationdialogadd.cpp
)

kconfig_add_kcfg_files(kcm_akronlinesyncconfig_SRCS onlinesyncsettings.kcfgc)

qt5_wrap_ui(kcm_akronlinesyncconfig_SRCS ui/configurationwidget.ui)
qt5_wrap_ui(kcm_akronlinesyncconfig_SRCS ui/configurationdialogadd.ui)

add_library(akregator_config_onlinesync MODULE ${kcm_akronlinesyncconfig_SRCS})

target_link_libraries(akregator_config_onlinesync
  akregatorinterfaces
  KF5::KIOCore
  ${KDE4_KPARTS_LIBS}
)

install(FILES akregator_config_onlinesync.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_onlinesync DESTINATION ${PLUGIN_INSTALL_DIR})

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

qt5_wrap_ui(akregator_onlinesync_plugin_SRCS
  ui/configurationwidget.ui
  ui/configurationdialogadd.ui
)

kconfig_add_kcfg_files(akregator_onlinesync_plugin_SRCS onlinesyncsettings.kcfgc)

add_library(akregator_onlinesync_plugin MODULE ${akregator_onlinesync_plugin_SRCS})

target_link_libraries(akregator_onlinesync_plugin
  akregatorinterfaces
  KF5::KIOCore
  ${KDE4_KPARTS_LIBS}
)

install(FILES akregator_onlinesync_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_onlinesync_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES akregator_onlinesync_plugin.rc DESTINATION ${DATA_INSTALL_DIR}/akregator_onlinesync_plugin)
