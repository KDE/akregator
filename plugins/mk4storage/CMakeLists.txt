# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    ${akregator_SOURCE_DIR}/interfaces
    ${akregator_SOURCE_DIR}/plugins/mk4storage/metakit/include
    ${akregator_BINARY_DIR}
    )

########### next target ###############

add_library(akregator_mk4storage_plugin MODULE)
target_sources(akregator_mk4storage_plugin PRIVATE
    metakit/src/column.cpp
    metakit/src/custom.cpp
    metakit/src/derived.cpp
    metakit/src/field.cpp
    metakit/src/fileio.cpp
    metakit/src/format.cpp
    metakit/src/handler.cpp
    metakit/src/persist.cpp
    metakit/src/remap.cpp
    metakit/src/std.cpp
    metakit/src/store.cpp
    metakit/src/string.cpp
    metakit/src/table.cpp
    metakit/src/univ.cpp
    metakit/src/view.cpp
    metakit/src/viewx.cpp
    feedstoragemk4impl.cpp
    storagemk4impl.cpp
    storagefactorymk4impl.cpp
    mk4plugin.cpp
    )


target_link_libraries(akregator_mk4storage_plugin
    KF5::Syndication
    akregatorinterfaces
    KF5::I18n
    KF5::CoreAddons
    )

install(TARGETS akregator_mk4storage_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(FILES akregator_mk4storage_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
