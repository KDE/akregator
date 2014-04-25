
include_directories(
  ${CMAKE_BINARY_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
)

########### next target ###############

set(kcm_akrgeneralconfig_SRCS akregator_config_general.cpp)

qt5_wrap_ui(kcm_akrgeneralconfig_SRCS settings_general.ui)
add_library(akregator_config_general MODULE ${kcm_akrgeneralconfig_SRCS})

target_link_libraries(akregator_config_general
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_general.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_general DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrappearanceconfig_SRCS akregator_config_appearance.cpp)

qt5_wrap_ui(kcm_akrappearanceconfig_SRCS settings_appearance.ui)
add_library(akregator_config_appearance MODULE ${kcm_akrappearanceconfig_SRCS})

target_link_libraries(akregator_config_appearance
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_appearance.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_appearance DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrarchiveconfig_SRCS akregator_config_archive.cpp)

qt5_wrap_ui(kcm_akrarchiveconfig_SRCS settings_archive.ui)
add_library(akregator_config_archive MODULE ${kcm_akrarchiveconfig_SRCS})

target_link_libraries(akregator_config_archive
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_archive.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_archive DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrbrowserconfig_SRCS akregator_config_browser.cpp)

qt5_wrap_ui(kcm_akrbrowserconfig_SRCS settings_browser.ui)
add_library(akregator_config_browser MODULE ${kcm_akrbrowserconfig_SRCS})

target_link_libraries(akregator_config_browser
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_browser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_browser DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akradvancedconfig_SRCS
  akregator_config_advanced.cpp
  settings_advanced.cpp
)

qt5_wrap_ui(kcm_akradvancedconfig_SRCS settings_advancedbase.ui)
add_library(akregator_config_advanced MODULE ${kcm_akradvancedconfig_SRCS})

target_link_libraries(akregator_config_advanced
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_advanced.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_advanced DESTINATION ${PLUGIN_INSTALL_DIR})
