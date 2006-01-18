#SUBDIRS = rss2 rdf atom
SUBDIRS = rss2 rdf atom mapper

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libsyndication.la

libsyndication_la_SOURCES = parserregistry.cpp documentsource.cpp \
                            documentvisitor.cpp elementwrapper.cpp constants.cpp \
                            feed.cpp item.cpp

libsyndication_include_HEADERS = abstractdocument.h abstractparser.h \
                                 parserregistry.h documentvisitor.h \
                                 documentsource.h elementwrapper.h \
                                 item.h feed.h

libsyndication_includedir = $(includedir)/libsyndication

libsyndication_la_LDFLAGS = $(all_libraries) $(LIB_KDECORE) -no-undefined -version-info 1:0:0
libsyndication_la_LIBADD = rss2/librss2local.la \
                           rdf/librdflocal.la \
                           atom/libatomlocal.la \
                           mapper/libmapperrss2local.la \
                           mapper/libmapperrdflocal.la \
                           mapper/libmapperatomlocal.la

#bin_SCRIPTS = libsyndication-config

#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = libsyndication.pc

# EXTRA_DIST = $(libsyndication_la_SOURCES) $(libsyndication_include_HEADERS) 
# $(pkgconfig_DATA)

