SUBDIRS = rss2 rdf

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libsyndication.la

libsyndication_la_SOURCES = parserregistry.cpp documentsource.cpp documentvisitor.cpp
libsyndication_include_HEADERS = abstractparser.h parserregistry.h document.h documentvisitor.h documentsource.h
libsyndication_includedir = $(includedir)/libsyndication

libsyndication_la_LDFLAGS = $(all_libraries) $(LIB_KDECORE) -no-undefined -version-info 1:0:0
libsyndication_la_LIBADD = rss2/librss2local.la

#bin_SCRIPTS = libsyndication-config

#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = libsyndication.pc

# EXTRA_DIST = $(libsyndication_la_SOURCES) $(libsyndication_include_HEADERS) 
# $(pkgconfig_DATA)

