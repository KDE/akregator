include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${akregator_SOURCE_DIR}/interfaces
  ${akregator_SOURCE_DIR}/plugins/mk4storage/metakit/include
  ${akregator_BINARY_DIR}
)

set(libmetakitlocal_SRCS
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
)

########### next target ###############

set(akregator_mk4storage_plugin_PART_SRCS
  ${libmetakitlocal_SRCS}
  feedstoragemk4impl.cpp
  storagemk4impl.cpp
  storagefactorymk4impl.cpp
  mk4plugin.cpp
)

add_library(akregator_mk4storage_plugin MODULE ${akregator_mk4storage_plugin_PART_SRCS})

target_link_libraries(akregator_mk4storage_plugin
  KF5::Syndication
  akregatorinterfaces
  KF5::I18n
  KF5::CoreAddons
)

install(TARGETS akregator_mk4storage_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(FILES akregator_mk4storage_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
