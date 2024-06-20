# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
include_directories(
    ${akregator_BINARY_DIR}/interfaces
    ${akregator_SOURCE_DIR}/interfaces
)

########### next target ###############

add_library(akregator_config_general MODULE)
target_sources(akregator_config_general PRIVATE akregator_config_general.cpp akregator_config_general.h)

ki18n_wrap_ui(akregator_config_general ui/settings_general.ui)

target_link_libraries(akregator_config_general
    akregatorinterfaces
    KF6::TextWidgets
    KF6::KCMUtils
    KF6::I18n
)

install(TARGETS akregator_config_general DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/akregator)

########### next target ###############

add_library(akregator_config_appearance MODULE)
target_sources(akregator_config_appearance PRIVATE akregator_config_appearance.cpp akregator_config_appearance.h)

ki18n_wrap_ui(akregator_config_appearance ui/settings_appearance.ui)

target_link_libraries(akregator_config_appearance
    akregatorinterfaces
    KF6::TextWidgets
    KF6::KCMUtils
    KF6::I18n
)

install(TARGETS akregator_config_appearance DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/akregator)

########### next target ###############

add_library(akregator_config_archive MODULE)
target_sources(akregator_config_archive PRIVATE akregator_config_archive.cpp akregator_config_archive.h)

ki18n_wrap_ui(akregator_config_archive ui/settings_archive.ui)

target_link_libraries(akregator_config_archive
    KF6::KCMUtils
    akregatorinterfaces
    KF6::TextWidgets
    KF6::I18n
)
install(TARGETS akregator_config_archive DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/akregator)

########### next target ###############

add_library(akregator_config_browser MODULE)
target_sources(akregator_config_browser PRIVATE akregator_config_browser.cpp akregator_config_browser.h)

ki18n_wrap_ui(akregator_config_browser ui/settings_browser.ui)

target_link_libraries(akregator_config_browser
    akregatorinterfaces
    KF6::KCMUtils
    KF6::I18n

)

install(TARGETS akregator_config_browser DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/akregator)

########### next target ###############

add_library(akregator_config_advanced MODULE)
target_sources(akregator_config_advanced PRIVATE
    akregator_config_advanced.cpp
    settings_advanced.cpp
    akregator_config_advanced.h
    settings_advanced.h
)

ki18n_wrap_ui(akregator_config_advanced ui/settings_advancedbase.ui)

target_link_libraries(akregator_config_advanced
    akregatorinterfaces
    KF6::TextWidgets
    KF6::KCMUtils
    KF6::I18n
)
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akregator_config_advanced PROPERTIES UNITY_BUILD ON)
endif()
install(TARGETS akregator_config_advanced DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/akregator)


########### next target ###############
add_library(akregator_config_plugins MODULE)

target_sources(akregator_config_plugins PRIVATE
    akregator_config_plugins.cpp
    akregatorconfigurepluginlistwidget.cpp
    akregator_config_plugins.h
    akregatorconfigurepluginlistwidget.h
)

ecm_qt_declare_logging_category(akregator_config_plugins HEADER kcm_config_plugins_debug.h IDENTIFIER AKREGATOR_CONFIG_PLUGIN_LOG CATEGORY_NAME org.kde.pim.akregator_config_plugin
        DESCRIPTION "akregator config plugin (akregator)"
        EXPORT AKREGATOR
)



target_link_libraries(akregator_config_plugins
    akregatorinterfaces
    KF6::KCMUtils
    KF6::I18n
    KPim6::WebEngineViewer
)
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akregator_config_plugins PROPERTIES UNITY_BUILD ON)
endif()

install(TARGETS akregator_config_plugins DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/akregator)


########### next target ###############
if(TARGET KF6::UserFeedbackWidgets)
    add_library(akregator_config_userfeedback MODULE)
    target_sources(akregator_config_userfeedback PRIVATE akregator_config_userfeedback.cpp akregator_config_userfeedback.h)


    target_link_libraries(akregator_config_userfeedback
        akregatorinterfaces
        KF6::TextWidgets
        KF6::KCMUtils
        KF6::I18n
        KF6::UserFeedbackWidgets
)

    install(TARGETS akregator_config_userfeedback DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/akregator)
endif()


###########
add_library(akregator_config_security MODULE)

target_sources(akregator_config_security PRIVATE
    akregator_config_security.cpp
    akregator_config_security.h
)

ki18n_wrap_ui(akregator_config_security ui/settings_security.ui)

target_link_libraries(akregator_config_security
    akregatorinterfaces
    KF6::KCMUtils
    KF6::I18n
    KPim6::WebEngineViewer
)
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akregator_config_security PROPERTIES UNITY_BUILD ON)
endif()

install(TARGETS akregator_config_security DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/akregator)
