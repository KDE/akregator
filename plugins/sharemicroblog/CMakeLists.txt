include_directories(
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/src
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
)

MESSAGE(STATUS "Akregator plasma plugin disabled")
#REACTIVATE plasma plugins
#set(akregator_sharemicroblog_plugin_SRCS sharemicroblogplugin.cpp)

#add_library(akregator_sharemicroblog_plugin MODULE ${akregator_sharemicroblog_plugin_SRCS})

#target_link_libraries(akregator_sharemicroblog_plugin
#  akregatorprivate
#  akregatorinterfaces
#  ${KDE4_PLASMA_LIBS}
#  KF5::KIOCore
#)

#install(FILES akregator_sharemicroblog_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(TARGETS akregator_sharemicroblog_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
#install(FILES akregator_sharemicroblog_plugin.rc DESTINATION ${DATA_INSTALL_DIR}/akregator_sharemicroblog_plugin)

########### settings target ###############

set(kcm_akrshareconfig_SRCS
  akregator_config_sharemicroblog.cpp
)

kconfig_add_kcfg_files(kcm_akrshareconfig_SRCS sharesettings.kcfgc)
ki18n_wrap_ui(kcm_akrshareconfig_SRCS configurationwidget.ui)
add_library(akregator_config_sharemicroblog MODULE ${kcm_akrshareconfig_SRCS})

target_link_libraries(akregator_config_sharemicroblog
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_sharemicroblog.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_sharemicroblog DESTINATION ${PLUGIN_INSTALL_DIR})
