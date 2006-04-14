kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############

install_files( /include/libsyndication/rdf FILES parser.h dublincorevocab.h dublincore.h                           syndicationvocab.h syndication.h                           contentvocab.h item.h image.h                           textinput.h document.h rssvocab.h                           node.h property.h literal.h resource.h                           model.h modelmaker.h rdfvocab.h sequence.h                           statement.h resourcewrapper.h nodevisitor.h rdf.h )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#noinst_LTLIBRARIES = librdflocal.la
#
#librdflocal_include_HEADERS = parser.h dublincorevocab.h dublincore.h \
#                         syndicationvocab.h syndication.h \
#                         contentvocab.h item.h image.h \
#                         textinput.h document.h rssvocab.h \
#                         node.h property.h literal.h resource.h \
#                         model.h modelmaker.h rdfvocab.h sequence.h \
#                         statement.h resourcewrapper.h nodevisitor.h rdf.h
#                 
#
#librdflocal_la_SOURCES = parser.cpp dublincorevocab.cpp dublincore.cpp \
#                         syndicationvocab.cpp syndication.cpp \
#                         contentvocab.cpp item.cpp image.cpp \
#                         textinput.cpp document.cpp rssvocab.cpp \
#                         node.cpp property.cpp literal.cpp resource.cpp \
#                         model.cpp modelmaker.cpp rdfvocab.cpp sequence.cpp \
#                         statement.cpp resourcewrapper.cpp nodevisitor.cpp
#
#librdflocal_includedir = $(includedir)/libsyndication/rdf
#
## librdflocal_la_METASOURCES = AUTO
#
#include $(top_srcdir)/admin/Doxyfile.am
