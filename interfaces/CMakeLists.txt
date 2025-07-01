# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
########### next target ###############
add_library(akregatorinterfaces)

target_sources(
    akregatorinterfaces
    PRIVATE
        command.cpp
        feedlistmanagementinterface.cpp
        command.h
        feedlistmanagementinterface.h
)

set(akregatorinterfaces_userfeedback_LIB_SRCS)
if(TARGET KF6::UserFeedbackWidgets)
    target_sources(
        akregatorinterfaces
        PRIVATE
            userfeedback/userfeedbackmanager.cpp
            userfeedback/akregatoruserfeedbackprovider.cpp
            userfeedback/userfeedbackmanager.h
            userfeedback/akregatoruserfeedbackprovider.h
    )
endif()

kconfig_add_kcfg_files(akregatorinterfaces akregatorconfig.kcfgc)

generate_export_header(akregatorinterfaces BASE_NAME akregatorinterfaces)

if(TARGET KF6::UserFeedbackWidgets)
    target_link_libraries(akregatorinterfaces KF6::UserFeedbackWidgets)
endif()

target_link_libraries(
    akregatorinterfaces
    KF6::ConfigGui
    Qt::Widgets
)
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        akregatorinterfaces
        PROPERTIES
            UNITY_BUILD
                ON
    )
endif()
target_include_directories(
    akregatorinterfaces
    PUBLIC
        "$<BUILD_INTERFACE:${akregator_SOURCE_DIR}/interfaces;${akregator_BINARY_DIR}/interfaces>"
)

set_target_properties(
    akregatorinterfaces
    PROPERTIES
        VERSION
            ${KDEPIM_LIB_VERSION}
        SOVERSION
            ${KDEPIM_LIB_SOVERSION}
)

install(
    TARGETS
        akregatorinterfaces
        ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    LIBRARY
        NAMELINK_SKIP
)
install(FILES akregator.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
