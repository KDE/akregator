# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
include_directories(
    ${akregator_SOURCE_DIR}/src
    )

########### next target ###############
add_library(kontact_akregatorplugin MODULE)

configure_file(akregatorplugin.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/akregatorplugin.json )

qt_add_dbus_interfaces(kontact_akregator_PART_SRCS ${akregator_SOURCE_DIR}/src/org.kde.akregator.part.xml)
target_sources(kontact_akregatorplugin PRIVATE akregator_plugin.cpp akregator_plugin.h ${kontact_akregator_PART_SRCS})


target_link_libraries(kontact_akregatorplugin KF${KF_MAJOR_VERSION}::I18n KF5::KontactInterface)

########### install files ###############

install(TARGETS kontact_akregatorplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kontact)
