include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

add_executable(akregatorstorageexporter)
target_sources(akregatorstorageexporter PRIVATE akregatorstorageexporter.cpp)
target_link_libraries(akregatorstorageexporter
    KF5::Syndication
    akregatorinterfaces
    KF5::Service
    KF5::I18n
    )

install(TARGETS akregatorstorageexporter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

