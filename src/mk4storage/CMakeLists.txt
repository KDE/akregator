kde4_header()

add_subdirectory( metakit ) 

include_directories( ${CMAKE_SOURCE_DIR}/akregator/src ${CMAKE_SOURCE_DIR}/akregator/src/mk4storage/metakit/include ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(akregator_mk4storage_plugin_PART_SRCS 
   feedstoragemk4impl.cpp 
   storagemk4impl.cpp 
   storagefactorymk4impl.cpp 
   mk4confwidget.cpp 
   mk4plugin.cpp )

kde4_automoc(${akregator_mk4storage_plugin_PART_SRCS})

kde4_add_ui3_files(akregator_mk4storage_plugin_PART_SRCS mk4confwidgetbase.ui )

kde4_add_kcfg_files(akregator_mk4storage_plugin_PART_SRCS mk4config.kcfgc )

kde4_add_plugin(akregator_mk4storage_plugin WITH_PREFIX ${akregator_mk4storage_plugin_PART_SRCS})

target_link_libraries(akregator_mk4storage_plugin  ${KDE4_KDECORE_LIBS} syndication akregatorprivate )

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} akregator_mk4storage_plugin )
install_targets(${PLUGIN_INSTALL_DIR} akregator_mk4storage_plugin )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES mk4config.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES akregator_mk4storage_plugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = libakregator_mk4storage_plugin.la
#
#SUBDIRS = metakit
#
#INCLUDES = \
#    -I$(top_srcdir)/akregator/src \
#    -I$(top_srcdir)/akregator/src/mk4storage/metakit/include \
#    $(all_includes)
#
#libakregator_mk4storage_plugin_la_LIBADD = \
#    $(top_builddir)/akregator/src/mk4storage/metakit/src/libmetakitlocal.la \
#    $(top_builddir)/akregator/src/librss/librsslocal.la \
#    ../libakregatorprivate.la \
#    $(LIB_KFILE) \
#    $(LIB_KDECORE)
#
#libakregator_mk4storage_plugin_la_LDFLAGS = \
#    $(KDE_PLUGIN) \
#    $(all_libraries)
#
#libakregator_mk4storage_plugin_la_SOURCES = feedstoragemk4impl.cpp \
#                                            storagemk4impl.cpp \
#                                            storagefactorymk4impl.cpp \
#                                            mk4confwidgetbase.ui \
#                                            mk4confwidget.cpp \
#                                            mk4plugin.cpp \
#                                            mk4config.kcfgc
#
#METASOURCES = AUTO
#
#kde_services_DATA = akregator_mk4storage_plugin.desktop
#
#kde_kcfg_DATA = mk4config.kcfg
