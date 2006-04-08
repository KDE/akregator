SUBDIRS = rss2 rdf atom mapper

INCLUDES = $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libsyndication.la

libsyndication_la_SOURCES = documentsource.cpp \
                            documentvisitor.cpp elementwrapper.cpp constants.cpp \
                            feed.cpp item.cpp enclosure.cpp category.cpp \
                            image.cpp tools.cpp person.cpp global.cpp \
                            loader.cpp dataretriever.cpp specificitemvisitor.cpp \
                            specificitem.cpp specificdocument.cpp abstractparser.cpp \
                            personimpl.cpp

libsyndication_include_HEADERS = specificdocument.h abstractparser.h \
                                 parsercollection.h documentvisitor.h \
                                 documentsource.h elementwrapper.h \
                                 item.h feed.h category.h image.h \
                                 enclosure.h person.h global.h \
                                 dataretriever.h loader.h specificitemvisitor.h \
                                 specificitem.h mapper.h sharedptr.h

libsyndication_includedir = $(includedir)/libsyndication

libsyndication_la_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(LIB_KIO) -no-undefined -version-info 1:0:0
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

DOXYGEN_EXCLUDE = mapper
include $(top_srcdir)/admin/Doxyfile.am
