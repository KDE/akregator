kde4_header()

add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

########### next target ###############

set(librss2local_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/category.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/cloud.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/document.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/enclosure.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/image.cpp
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/item.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/parser.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/source.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/textinput.cpp
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rss2/tools_p.cpp
	)


set(librdflocal_la_SRCS
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/parser.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/dublincorevocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/dublincore.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/syndicationvocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/syndication.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/contentvocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/item.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/image.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/textinput.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/document.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/rssvocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/node.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/property.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/literal.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/resource.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/model.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/modelmaker.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/rdfvocab.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/sequence.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/statement.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/resourcewrapper.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/rdf/nodevisitor.cpp
)

set(libatomlocal_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/category.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/constants.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/content.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/document.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/entry.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/generator.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/link.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/parser.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/person.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/source.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/atom/atomtools.cpp 
)


set(libmapperrss2local_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/feedrss2impl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/itemrss2impl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/enclosurerss2impl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/categoryrss2impl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/imagerss2impl.cpp
)

set(libmapperrdflocal_SRCS 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/feedrdfimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/itemrdfimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/imagerdfimpl.cpp
)

set(libmapperatomlocal_SRCS
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/feedatomimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/itematomimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/enclosureatomimpl.cpp 
	${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/mapper/categoryatomimpl.cpp
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
   personimpl.cpp 
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

install_files( /include/libsyndication FILES specificdocument.h abstractparser.h parsercollection.h
                documentvisitor.h documentsource.h elementwrapper.h item.h feed.h category.h image.h         
                enclosure.h person.h global.h dataretriever.h loader.h )

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
