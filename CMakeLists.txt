include(CheckTypeSize)
check_type_size("long" SIZEOF_LONG)
configure_file(config-akregator.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-akregator.h)

add_subdirectory( interfaces ) 
add_subdirectory( plugins ) 
add_subdirectory( configuration ) 
add_subdirectory( src ) 
