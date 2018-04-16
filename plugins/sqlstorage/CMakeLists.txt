include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    ${akregator_SOURCE_DIR}/interfaces
    ${akregator_BINARY_DIR}
    )

find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Sql)

########### next target ###############

set(akregator_sqlstorage_plugin_PART_SRCS
    feedstoragesqlimpl.cpp
    storagesqlimpl.cpp
    storagefactorysqlimpl.cpp
    sqlplugin.cpp
    )

add_library(akregator_sqlstorage_plugin MODULE ${akregator_sqlstorage_plugin_PART_SRCS})

target_link_libraries(akregator_sqlstorage_plugin
    KF5::Syndication
    akregatorinterfaces
    KF5::I18n
    KF5::CoreAddons
    Qt5::Sql
    )

install(TARGETS akregator_sqlstorage_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(FILES akregator_sqlstorage_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

