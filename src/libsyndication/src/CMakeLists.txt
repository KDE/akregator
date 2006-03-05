kde4_header()

add_subdirectory( rss2 ) 
add_subdirectory( rdf ) 
add_subdirectory( atom ) 
add_subdirectory( mapper ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(syndication_LIB_SRCS 
   parsercollection.cpp 
   documentsource.cpp 
   documentvisitor.cpp 
   elementwrapper.cpp 
   constants.cpp 
   feed.cpp 
   item.cpp 
   enclosure.cpp 
   category.cpp 
   image.cpp 
   tools.cpp 
   person.cpp 
   loader.cpp 
   dataretriever.cpp )

kde4_automoc(${syndication_LIB_SRCS})

kde4_add_library(syndication SHARED ${syndication_LIB_SRCS})

target_link_libraries(syndication  ${QT_AND_KDECORE_LIBS} kio rss2local rdflocal atomlocal mapperrss2local mapperrdflocal mapperatomlocal mappercommonlocal )

set_target_properties(syndication PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} syndication )


########### install files ###############

install_files( /include/libsyndication FILES abstractdocument.h abstractparser.h                                   parsercollection.h documentvisitor.h                                   documentsource.h elementwrapper.h                                   item.h feed.h category.h image.h                                   enclosure.h person.h global.h                                   dataretriever.h loader.h )

kde4_create_doxygen_docs( EXCLUDE mapper  )

kde4_footer()



#original Makefile.am contents follow:

##SUBDIRS = rss2 rdf atom
#SUBDIRS = rss2 rdf atom mapper
#
#INCLUDES = $(all_includes)
#
#METASOURCES = AUTO
#
#lib_LTLIBRARIES = libsyndication.la
#
#libsyndication_la_SOURCES = parsercollection.cpp documentsource.cpp \
#                            documentvisitor.cpp elementwrapper.cpp constants.cpp \
#                            feed.cpp item.cpp enclosure.cpp category.cpp \
#                            image.cpp tools.cpp person.cpp \
#                            loader.cpp dataretriever.cpp
#
#libsyndication_include_HEADERS = abstractdocument.h abstractparser.h \
#                                 parsercollection.h documentvisitor.h \
#                                 documentsource.h elementwrapper.h \
#                                 item.h feed.h category.h image.h \
#                                 enclosure.h person.h global.h \
#                                 dataretriever.h loader.h
#
#libsyndication_includedir = $(includedir)/libsyndication
#
#libsyndication_la_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(LIB_KIO) -no-undefined -version-info 1:0:0
#libsyndication_la_LIBADD = rss2/librss2local.la \
#                           rdf/librdflocal.la \
#                           atom/libatomlocal.la \
#                           mapper/libmapperrss2local.la \
#                           mapper/libmapperrdflocal.la \
#                           mapper/libmapperatomlocal.la \
#                           mapper/libmappercommonlocal.la
#
##bin_SCRIPTS = libsyndication-config
#
##pkgconfigdir = $(libdir)/pkgconfig
##pkgconfig_DATA = libsyndication.pc
#
## EXTRA_DIST = $(libsyndication_la_SOURCES) $(libsyndication_include_HEADERS) 
## $(pkgconfig_DATA)
#
#DOXYGEN_EXCLUDE = mapper
#include $(top_srcdir)/admin/Doxyfile.am
