include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/include
  ${CMAKE_BINARY_DIR}/akregator
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES}
  ${Boost_INCLUDE_DIR}
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

kde4_add_plugin(akregator_mk4storage_plugin ${akregator_mk4storage_plugin_PART_SRCS})

target_link_libraries(akregator_mk4storage_plugin
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_SYNDICATION_LIBS}
  akregatorinterfaces
)

install(TARGETS akregator_mk4storage_plugin DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES akregator_mk4storage_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
