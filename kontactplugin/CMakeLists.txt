
include_directories(
  ${CMAKE_SOURCE_DIR}/akregator/src
  ${Boost_INCLUDE_DIRS}
)

########### next target ###############

set(kontact_akregator_PART_SRCS akregator_plugin.cpp)

qt5_add_dbus_interfaces(kontact_akregator_PART_SRCS ${CMAKE_SOURCE_DIR}/akregator/src/org.kde.akregator.part.xml)

add_library(kontact_akregatorplugin MODULE ${kontact_akregator_PART_SRCS})

target_link_libraries(kontact_akregatorplugin  KF5::KontactInterface)

########### install files ###############

install(TARGETS kontact_akregatorplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES akregatorplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kontact)

install(FILES akregator.setdlg DESTINATION ${KDE_INSTALL_DATADIR}/kontact/ksettingsdialog)

