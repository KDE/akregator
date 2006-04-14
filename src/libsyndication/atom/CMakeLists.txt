kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############

install_files( /include/libsyndication/atom FILES category.h constants.h content.h document.h entry.h                   generator.h link.h parser.h person.h source.h atom.h )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#noinst_LTLIBRARIES = libatomlocal.la
#
#libatomlocal_include_HEADERS = category.h constants.h content.h document.h entry.h \
#                 generator.h link.h parser.h person.h source.h atom.h
#
#noinst_HEADERS = tools.h
#
#libatomlocal_la_SOURCES = category.cpp constants.cpp content.cpp document.cpp \
#                          entry.cpp generator.cpp link.cpp parser.cpp \
#                          person.cpp source.cpp tools.cpp
#
#
#libatomlocal_includedir = $(includedir)/libsyndication/atom
#
## libatomlocal_la_METASOURCES = AUTO
#
#include $(top_srcdir)/admin/Doxyfile.am
