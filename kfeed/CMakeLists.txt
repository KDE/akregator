add_subdirectory( tests )

########### next target ###############

include_directories( )
set(kfeed_LIB_SRCS
   item.cpp
   enclosure.cpp
   person.cpp
   category.cpp
   serializer.cpp
)

kde4_automoc(${kfeed_LIB_SRCS})

kde4_add_library(kfeed SHARED ${kfeed_LIB_SRCS})

target_link_libraries(kfeed ${KDE4_SYNDICATION_LIBS} ${KDE4_KDECORE_LIBS} )

set_target_properties(kfeed PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kfeed DESTINATION ${LIB_INSTALL_DIR})
#install( FILES kfeed_export.h item.h enclosure.h person.h category.h DESTINATION ${INCLUDE_INSTALL_DIR}/kfeed )
