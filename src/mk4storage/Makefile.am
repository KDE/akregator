kde_module_LTLIBRARIES = libakregator_mk4storage_plugin.la

SUBDIRS = metakit

INCLUDES = \
    -I$(top_srcdir)/akregator/src \
    -I$(top_srcdir)/akregator/src/mk4storage/metakit/include \
    $(all_includes)

libakregator_mk4storage_plugin_la_LIBADD = \
    $(top_builddir)/akregator/src/mk4storage/metakit/src/libmetakitlocal.la \
    $(top_builddir)/akregator/src/librss/librsslocal.la \
    ../libakregatorprivate.la \
    $(LIB_KFILE) \
    $(LIB_KDECORE)

libakregator_mk4storage_plugin_la_LDFLAGS = \
    $(KDE_PLUGIN) \
    $(all_libraries)

libakregator_mk4storage_plugin_la_SOURCES = feedstoragemk4impl.cpp \
                                            storagemk4impl.cpp \
                                            storagefactorymk4impl.cpp \
                                            mk4confwidgetbase.ui \
                                            mk4plugin.cpp \
                                            mk4config.kcfgc

METASOURCES = AUTO

kde_services_DATA = akregator_mk4storage_plugin.desktop

kde_kcfg_DATA = mk4config.kcfg
