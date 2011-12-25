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

set(akregator_sharemicroblog_plugin_SRCS sharemicroblogplugin.cpp)

kde4_add_plugin(akregator_sharemicroblog_plugin ${akregator_sharemicroblog_plugin_SRCS})

target_link_libraries(akregator_sharemicroblog_plugin
  akregatorprivate
  akregatorinterfaces
  ${KDE4_PLASMA_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_KPARTS_LIBS}
)

install(FILES akregator_sharemicroblog_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_sharemicroblog_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES akregator_sharemicroblog_plugin.rc DESTINATION ${DATA_INSTALL_DIR}/akregator_sharemicroblog_plugin)

########### settings target ###############

set(kcm_akrshareconfig_SRCS
  akregator_config_sharemicroblog.cpp
)

kde4_add_kcfg_files(kcm_akrshareconfig_SRCS sharesettings.kcfgc)
kde4_add_ui_files(kcm_akrshareconfig_SRCS configurationwidget.ui)
kde4_add_plugin(akregator_config_sharemicroblog ${kcm_akrshareconfig_SRCS})

target_link_libraries(akregator_config_sharemicroblog
  akregatorinterfaces
  ${KDE4_KIO_LIBS}
  ${KDE4_KPARTS_LIBS}
)

install(FILES akregator_config_sharemicroblog.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_sharemicroblog DESTINATION ${PLUGIN_INSTALL_DIR})
