########### next target ###############

include_directories( ${Boost_INCLUDE_DIR} )

set(akregatorinterfaces_LIB_SRCS
   articleinterceptor.cpp 
   plugin.cpp 
   storagefactoryregistry.cpp 
)

kde4_automoc(${akregatorinterfaces_LIB_SRCS})

kde4_add_library(akregatorinterfaces SHARED ${akregatorinterfaces_LIB_SRCS})

target_link_libraries(akregatorinterfaces kdepim  ${KDE4_KHTML_LIBS} ${KDE4_SYNDICATION_LIBS} )

set_target_properties(akregatorinterfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS akregatorinterfaces DESTINATION ${LIB_INSTALL_DIR})
install( FILES akregator_export.h article.h types.h storage.h feedstorage.h storagefactory.h storagefactoryregistry.h plugin.h articleinterceptor.h  DESTINATION ${INCLUDE_INSTALL_DIR}/akregator )
