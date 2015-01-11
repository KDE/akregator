
include_directories(
  ${CMAKE_BINARY_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
)

########### next target ###############

set(kcm_akrgeneralconfig_SRCS akregator_config_general.cpp)

ki18n_wrap_ui(kcm_akrgeneralconfig_SRCS settings_general.ui)
add_library(akregator_config_general MODULE ${kcm_akrgeneralconfig_SRCS})

target_link_libraries(akregator_config_general
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_general.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_general DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_akrappearanceconfig_SRCS akregator_config_appearance.cpp)

ki18n_wrap_ui(kcm_akrappearanceconfig_SRCS settings_appearance.ui)
add_library(akregator_config_appearance MODULE ${kcm_akrappearanceconfig_SRCS})

target_link_libraries(akregator_config_appearance
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_appearance.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_appearance DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_akrarchiveconfig_SRCS akregator_config_archive.cpp)

ki18n_wrap_ui(kcm_akrarchiveconfig_SRCS settings_archive.ui)
add_library(akregator_config_archive MODULE ${kcm_akrarchiveconfig_SRCS})

target_link_libraries(akregator_config_archive
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_archive.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_archive DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_akrbrowserconfig_SRCS akregator_config_browser.cpp)

ki18n_wrap_ui(kcm_akrbrowserconfig_SRCS settings_browser.ui)
add_library(akregator_config_browser MODULE ${kcm_akrbrowserconfig_SRCS})

target_link_libraries(akregator_config_browser
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_browser.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_browser DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_akradvancedconfig_SRCS
  akregator_config_advanced.cpp
  settings_advanced.cpp
)

ki18n_wrap_ui(kcm_akradvancedconfig_SRCS settings_advancedbase.ui)
add_library(akregator_config_advanced MODULE ${kcm_akradvancedconfig_SRCS})

target_link_libraries(akregator_config_advanced
  akregatorinterfaces
  KF5::KIOCore
)

install(FILES akregator_config_advanced.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_advanced DESTINATION ${KDE_INSTALL_PLUGINDIR})
