
include_directories(
    ${akregator_BINARY_DIR}/interfaces
    ${akregator_SOURCE_DIR}/interfaces
    )

########### next target ###############

add_library(akregator_config_general MODULE)
target_sources(akregator_config_general PRIVATE akregator_config_general.cpp)

ki18n_wrap_ui(akregator_config_general ui/settings_general.ui)

target_link_libraries(akregator_config_general
    akregatorinterfaces
    KF5::TextWidgets
    KF5::KCMUtils
    KF5::I18n
    )

install(FILES akregator_config_general.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
kcoreaddons_desktop_to_json(akregator_config_general akregator_config_general.desktop SERVICE_TYPES kcmodule.desktop)
install(TARGETS akregator_config_general DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

add_library(akregator_config_appearance MODULE  )
target_sources(akregator_config_appearance PRIVATE akregator_config_appearance.cpp)

ki18n_wrap_ui(akregator_config_appearance ui/settings_appearance.ui)

target_link_libraries(akregator_config_appearance
    akregatorinterfaces
    KF5::TextWidgets
    KF5::KCMUtils
    KF5::I18n
    )

kcoreaddons_desktop_to_json(akregator_config_appearance akregator_config_appearance.desktop SERVICE_TYPES kcmodule.desktop)
install(FILES akregator_config_appearance.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_appearance DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

add_library(akregator_config_archive MODULE)
target_sources(akregator_config_archive PRIVATE akregator_config_archive.cpp)

ki18n_wrap_ui(akregator_config_archive ui/settings_archive.ui)

target_link_libraries(akregator_config_archive
    KF5::KCMUtils
    akregatorinterfaces
    KF5::TextWidgets
    KF5::I18n
    )
#kcoreaddons_desktop_to_json(akregator_config_archive akregator_config_archive.desktop DEFAULT_SERVICE_TYPE)
install(FILES akregator_config_archive.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_archive DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

add_library(akregator_config_browser MODULE )
target_sources(akregator_config_browser PRIVATE akregator_config_browser.cpp)

ki18n_wrap_ui(akregator_config_browser ui/settings_browser.ui)

target_link_libraries(akregator_config_browser
    akregatorinterfaces
    KF5::KCMUtils
    KF5::I18n

    )
kcoreaddons_desktop_to_json(akregator_config_browser akregator_config_browser.desktop SERVICE_TYPES kcmodule.desktop)

install(FILES akregator_config_browser.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_browser DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

add_library(akregator_config_advanced MODULE)
target_sources(akregator_config_advanced PRIVATE
    akregator_config_advanced.cpp
    settings_advanced.cpp
    )

ki18n_wrap_ui(akregator_config_advanced ui/settings_advancedbase.ui)

target_link_libraries(akregator_config_advanced
    akregatorinterfaces
    KF5::TextWidgets
    KF5::KCMUtils
    KF5::I18n
    )
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akregator_config_advanced PROPERTIES UNITY_BUILD ON)
endif()
kcoreaddons_desktop_to_json(akregator_config_advanced akregator_config_advanced.desktop SERVICE_TYPES kcmodule.desktop)
install(FILES akregator_config_advanced.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_advanced DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############
add_library(akregator_config_plugins MODULE)

target_sources(akregator_config_plugins PRIVATE
    akregator_config_plugins.cpp
    akregatorconfigurepluginlistwidget.cpp
    )

ecm_qt_declare_logging_category(akregator_config_plugins HEADER kcm_config_plugins_debug.h IDENTIFIER AKREGATOR_CONFIG_PLUGIN_LOG CATEGORY_NAME org.kde.pim.akregator_config_plugin
        DESCRIPTION "akregator config plugin (akregator)"
        EXPORT AKREGATOR
    )



target_link_libraries(akregator_config_plugins
    akregatorinterfaces
    KF5::KCMUtils
    KF5::I18n
    KF5::WebEngineViewer
    )
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akregator_config_plugins PROPERTIES UNITY_BUILD ON)
endif()

kcoreaddons_desktop_to_json(akregator_config_plugins akregator_config_plugins.desktop SERVICE_TYPES kcmodule.desktop)
install(FILES akregator_config_plugins.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_plugins DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############
if (TARGET KUserFeedbackWidgets)
    add_library(akregator_config_userfeedback MODULE)
    target_sources(akregator_config_userfeedback PRIVATE akregator_config_userfeedback.cpp)


    target_link_libraries(akregator_config_userfeedback
        akregatorinterfaces
        KF5::TextWidgets
        KF5::KCMUtils
        KF5::I18n
        KUserFeedbackWidgets
        )

    install(FILES akregator_config_userfeedback.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
    kcoreaddons_desktop_to_json(akregator_config_userfeedback akregator_config_userfeedback.desktop SERVICE_TYPES kcmodule.desktop)
    install(TARGETS akregator_config_userfeedback DESTINATION ${KDE_INSTALL_PLUGINDIR})
endif()


###########
add_library(akregator_config_security MODULE)

target_sources(akregator_config_security PRIVATE
    akregator_config_security.cpp
    )

target_link_libraries(akregator_config_security
    akregatorinterfaces
    KF5::KCMUtils
    KF5::I18n
    KF5::WebEngineViewer
    )
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akregator_config_security PROPERTIES UNITY_BUILD ON)
endif()

kcoreaddons_desktop_to_json(akregator_config_security akregator_config_security.desktop SERVICE_TYPES kcmodule.desktop)
install(FILES akregator_config_security.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS akregator_config_security DESTINATION ${KDE_INSTALL_PLUGINDIR})
