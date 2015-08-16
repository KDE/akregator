project(akregator)
include(CheckTypeSize)
check_type_size("long" SIZEOF_LONG)
configure_file(config-akregator.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-akregator.h)
add_definitions(-DTRANSLATION_DOMAIN=\"akregator\")


add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_subdirectory(export)
add_subdirectory(interfaces)
add_subdirectory(plugins)
add_subdirectory(configuration)
add_subdirectory(src)
add_subdirectory(kontactplugin)
