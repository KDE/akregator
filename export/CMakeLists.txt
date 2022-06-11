# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

add_executable(akregatorstorageexporter)
target_sources(akregatorstorageexporter PRIVATE akregatorstorageexporter.cpp)
target_link_libraries(akregatorstorageexporter
    KF5::Syndication
    akregatorprivate
    KF5::Service
    KF5::I18n
    KF5::CoreAddons
    )

install(TARGETS akregatorstorageexporter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

