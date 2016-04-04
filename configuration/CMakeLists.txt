
include_directories(
  ${CMAKE_BINARY_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
)

########### next target ###############

set(kcm_akrgeneralconfig_SRCS akregator_config_general.cpp)

ki18n_wrap_ui(kcm_akrgeneralconfig_SRCS ui/settings_general.ui)
add_library(akregator_config_general MODULE ${kcm_akrgeneralconfig_SRCS})

target_link_libraries(akregator_config_general
  akregatorinterfaces
  KF5::KIOCore
  KF5::TextWidgets
  KF5::KCMUtils
  KF5::Completion
  KF5::I18n
)

install(FILES akregator_config_general.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
kcoreaddons_desktop_to_json(akregator_config_general akregator_config_general.desktop DEFAULT_SERVICE_TYPE)
install(TARGETS akregator_config_general DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_akrappearanceconfig_SRCS akregator_config_appearance.cpp)

ki18n_wrap_ui(kcm_akrappearanceconfig_SRCS ui/settings_appearance.ui)
add_library(akregator_config_appearance MODULE ${kcm_akrappearanceconfig_SRCS})

target_link_libraries(akregator_config_appearance
  akregatorinterfaces
  KF5::KIOCore
  KF5::TextWidgets
  KF5::KCMUtils
  KF5::Completion
  KF5::I18n
)

kcoreaddons_desktop_to_json(akregator_config_appearance akregator_config_appearance.desktop DEFAULT_SERVICE_TYPE)
install(FILES akregator_config_appearance.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_appearance DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_akrarchiveconfig_SRCS akregator_config_archive.cpp)

ki18n_wrap_ui(kcm_akrarchiveconfig_SRCS ui/settings_archive.ui)
add_library(akregator_config_archive MODULE ${kcm_akrarchiveconfig_SRCS})

target_link_libraries(akregator_config_archive
  KF5::KCMUtils
  akregatorinterfaces
  KF5::KIOCore
  KF5::TextWidgets
  KF5::Completion
  KF5::I18n   
)
#kcoreaddons_desktop_to_json(akregator_config_archive akregator_config_archive.desktop DEFAULT_SERVICE_TYPE)
install(FILES akregator_config_archive.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_archive DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_akrbrowserconfig_SRCS akregator_config_browser.cpp)

ki18n_wrap_ui(kcm_akrbrowserconfig_SRCS ui/settings_browser.ui)
add_library(akregator_config_browser MODULE ${kcm_akrbrowserconfig_SRCS})

target_link_libraries(akregator_config_browser
  akregatorinterfaces
  KF5::KIOCore
  KF5::KCMUtils
  KF5::Completion
  KF5::I18n

)
kcoreaddons_desktop_to_json(akregator_config_browser akregator_config_browser.desktop DEFAULT_SERVICE_TYPE)

install(FILES akregator_config_browser.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_browser DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_akradvancedconfig_SRCS
  akregator_config_advanced.cpp
  settings_advanced.cpp
)

ki18n_wrap_ui(kcm_akradvancedconfig_SRCS ui/settings_advancedbase.ui)
add_library(akregator_config_advanced MODULE ${kcm_akradvancedconfig_SRCS})

target_link_libraries(akregator_config_advanced
  akregatorinterfaces
  KF5::KIOCore
  KF5::TextWidgets
  KF5::KCMUtils
  KF5::Completion
  KF5::I18n
)

kcoreaddons_desktop_to_json(akregator_config_advanced akregator_config_advanced.desktop DEFAULT_SERVICE_TYPE)
install(FILES akregator_config_advanced.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_advanced DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############
if (NOT QTWEBENGINE_SUPPORT_OPTION)

    set(kcm_akradblockconfig_SRCS
        akregator_config_adblock.cpp
        )

    add_library(akregator_config_adblock MODULE ${kcm_akradblockconfig_SRCS})

    target_link_libraries(akregator_config_adblock
        akregatorinterfaces
        KF5::KCMUtils
        KF5::I18n
        KF5::MessageViewer
        KF5::ItemViews
        KF5::IconThemes
        )

    kcoreaddons_desktop_to_json(akregator_config_adblock akregator_config_adblock.desktop DEFAULT_SERVICE_TYPE)
    install(FILES akregator_config_adblock.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
    install(TARGETS akregator_config_adblock DESTINATION ${KDE_INSTALL_PLUGINDIR})
else()
     #TODO add support for webengineurl interceptor plugin
endif()
