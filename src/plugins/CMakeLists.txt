# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
add_library(akregatorplugins STATIC)

target_sources(
    akregatorplugins
    PRIVATE
        pluginmanager.h
        pluginmanager.cpp
)

target_link_libraries(
    akregatorplugins
    akregatorinterfaces
    KF6::CoreAddons
)

target_include_directories(akregatorplugins PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

add_subdirectory(miniflux)
