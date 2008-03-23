########### next target ###############

include_directories( ${Boost_INCLUDE_DIR} )

set(akregatorinterfaces_LIB_SRCS
   command.cpp
   plugin.cpp 
   storagefactoryregistry.cpp 
)

kde4_add_kcfg_files(akregatorinterfaces_LIB_SRCS akregatorconfig.kcfgc)

kde4_add_library(akregatorinterfaces SHARED ${akregatorinterfaces_LIB_SRCS})

target_link_libraries(akregatorinterfaces kdepim ${KDE4_KHTML_LIBS} ${KDE4_SYNDICATION_LIBS} )

set_target_properties(akregatorinterfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS akregatorinterfaces DESTINATION ${LIB_INSTALL_DIR})

install( FILES akregator_export.h
               article.h
               command.h
               types.h
               storage.h
               feedstorage.h
               storagefactory.h
               storagefactoryregistry.h
               plugin.h 
               DESTINATION ${INCLUDE_INSTALL_DIR}/akregator )
               
install( FILES akregator.kcfg DESTINATION ${KCFG_INSTALL_DIR})
