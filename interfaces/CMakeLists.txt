########### next target ###############

include_directories(${Boost_INCLUDE_DIR})

set(akregatorinterfaces_LIB_SRCS
  command.cpp
  feedlistmanagementinterface.cpp
  plugin.cpp
  storagefactoryregistry.cpp
)

kconfig_add_kcfg_files(akregatorinterfaces_LIB_SRCS akregatorconfig.kcfgc)

add_library(akregatorinterfaces ${LIBRARY_TYPE} ${akregatorinterfaces_LIB_SRCS})
generate_export_header(akregatorinterfaces BASE_NAME akregatorinterfaces)

target_link_libraries(akregatorinterfaces
  kdepim
  KF5::KHtml
  KF5::Syndication
)

set_target_properties(akregatorinterfaces PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS akregatorinterfaces ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akregator.kcfg DESTINATION ${KCFG_INSTALL_DIR})

