include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set(akregatorstoragemigrator_SRCS akregatorstoragemigrator.cpp)

add_executable(akregatorstoragemigrator ${akregatorstoragemigrator_SRCS})

target_link_libraries(akregatorstoragemigrator
    KF5::Syndication
    akregatorinterfaces
    KF5::Service
    KF5::I18n
    )

install(TARGETS akregatorstoragemigrator ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

