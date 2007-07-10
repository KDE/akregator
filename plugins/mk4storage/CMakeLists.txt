include_directories( ${CMAKE_SOURCE_DIR}/akregator/interfaces ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/include ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIR} )

set(libmetakitlocal_SRCS 
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/column.cpp 
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/custom.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/derived.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/field.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/fileio.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/format.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/handler.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/persist.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/remap.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/std.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/store.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/string.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/table.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/univ.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/view.cpp
                ${CMAKE_SOURCE_DIR}/akregator/plugins/mk4storage/metakit/src/viewx.cpp
                )

add_subdirectory( metakit ) 

########### next target ###############

set(akregator_mk4storage_plugin_PART_SRCS ${libmetakitlocal_SRCS}
   feedstoragemk4impl.cpp 
   storagemk4impl.cpp 
   storagefactorymk4impl.cpp 
   mk4plugin.cpp )


kde4_add_plugin(akregator_mk4storage_plugin WITH_PREFIX ${akregator_mk4storage_plugin_PART_SRCS})

target_link_libraries(akregator_mk4storage_plugin  ${KDE4_KDECORE_LIBS} ${KDE4_SYNDICATION_LIBS} akregatorinterfaces )

install(TARGETS akregator_mk4storage_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES akregator_mk4storage_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
