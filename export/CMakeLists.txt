include_directories( ${CMAKE_SOURCE_DIR}/akregator/interfaces ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set(akregatorstorageexporter_SRCS 
    akregatorstorageexporter.cpp
)

kde4_add_executable(akregatorstorageexporter NOGUI ${akregatorstorageexporter_SRCS})

target_link_libraries(akregatorstorageexporter ${KDE4_KDECORE_LIBS} ${KDE4_SYNDICATION_LIBS} akregatorinterfaces)

install(TARGETS akregatorstorageexporter ${INSTALL_TARGETS_DEFAULT_ARGS})

