########### next target ###############

set(akregatorinterfaces_LIB_SRCS
    command.cpp
    feedlistmanagementinterface.cpp
    plugin.cpp
    storagefactoryregistry.cpp
    )

set(akregatorinterfaces_userfeedback_LIB_SRCS)
if (TARGET KUserFeedbackWidgets)
    set(akregatorinterfaces_userfeedback_LIB_SRCS ${akregatorinterfaces_userfeedback_LIB_SRCS}
        userfeedback/userfeedbackmanager.cpp
        userfeedback/akregatoruserfeedbackprovider.cpp
        )
endif()


kconfig_add_kcfg_files(akregatorinterfaces_LIB_SRCS akregatorconfig.kcfgc)

add_library(akregatorinterfaces ${akregatorinterfaces_LIB_SRCS} ${akregatorinterfaces_userfeedback_LIB_SRCS})
generate_export_header(akregatorinterfaces BASE_NAME akregatorinterfaces)

set(akregator_userfeedback_LIB)
if (TARGET KUserFeedbackWidgets)
    set(akregator_userfeedback_LIB KUserFeedbackWidgets)
endif()

target_link_libraries(akregatorinterfaces
    KF5::ConfigGui
    Qt5::Widgets
    ${akregator_userfeedback_LIB}
    )

target_include_directories(akregatorinterfaces PUBLIC "$<BUILD_INTERFACE:${akregator_SOURCE_DIR}/interfaces;${akregator_BINARY_DIR}/interfaces>")

set_target_properties(akregatorinterfaces PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION}
    )

install(TARGETS akregatorinterfaces ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akregator.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

