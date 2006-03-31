kde4_header()

add_subdirectory( rss2 ) 
add_subdirectory( rdf ) 
add_subdirectory( atom ) 
add_subdirectory( mapper ) 

include_directories(
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/
	${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(librss2local_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/category.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/cloud.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/document.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/enclosure.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/image.cpp
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/item.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/parser.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/source.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/textinput.cpp
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rss2/tools_p.cpp
	)


set(librdflocal_la_SRCS
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/parser.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/dublincorevocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/dublincore.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/syndicationvocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/syndication.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/contentvocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/item.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/image.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/textinput.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/document.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/rssvocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/node.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/property.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/literal.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/resource.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/model.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/modelmaker.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/rdfvocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/sequence.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/statement.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/resourcewrapper.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/rdf/nodevisitor.cpp
)

set(libatomlocal_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/category.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/constants.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/content.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/document.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/entry.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/generator.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/link.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/parser.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/person.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/source.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/atom/atomtools.cpp 
)


set(libmappercommonlocal_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/personimpl.cpp
)

set(libmapperrss2local_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/feedrss2impl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/itemrss2impl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/enclosurerss2impl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/categoryrss2impl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/imagerss2impl.cpp
)

set(libmapperrdflocal_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/feedrdfimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/itemrdfimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/imagerdfimpl.cpp
)

set(libmapperatomlocal_SRCS
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/feedatomimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/itematomimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/enclosureatomimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src/mapper/categoryatomimpl.cpp
)

set(syndication_LIB_SRCS 
	${librss2local_SRCS} 
	${librdflocal_la_SRCS} 
	${libatomlocal_SRCS} 
	${libmappercommonlocal_SRCS} 
	${libmapperrss2local_SRCS} 
	${libmapperrdflocal_SRCS} 
	${libmapperatomlocal_SRCS}
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
   global.cpp
   dataretriever.cpp 
   specificitemvisitor.cpp
   specificitem.cpp 
   specificdocument.cpp 
   abstractparser.cpp)

kde4_automoc(${syndication_LIB_SRCS})

kde4_add_library(syndication SHARED ${syndication_LIB_SRCS})

target_link_libraries(syndication  ${KDE4_KIO_LIBS}  )

set_target_properties(syndication PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} syndication )


########### install files ###############

install_files( /include/libsyndication FILES specificdocument.h abstractparser.h                                   parsercollection.h documentvisitor.h                                   documentsource.h elementwrapper.h                                   item.h feed.h category.h image.h                                   enclosure.h person.h global.h                                   dataretriever.h loader.h )

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
#libsyndication_include_HEADERS = specificdocument.h abstractparser.h \
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
