kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############

install_files( /include/libsyndication/rss2 FILES category.h cloud.h document.h enclosure.h                   image.h item.h parser.h rss2.h source.h textinput.h )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#noinst_LTLIBRARIES = librss2local.la
#
#librss2local_include_HEADERS = category.h cloud.h constants.h document.h enclosure.h \
#                 image.h item.h parser.h rss2.h source.h textinput.h tools.h
#
#librss2local_la_SOURCES = category.cpp cloud.cpp constants.cpp \
#                                 document.cpp enclosure.cpp image.cpp \
#                                 item.cpp parser.cpp source.cpp textinput.cpp \
#                                 tools.cpp
#
#librss2local_includedir = $(includedir)/libsyndication/rss2
#
## librss2local_la_METASOURCES = AUTO
#
#include $(top_srcdir)/admin/Doxyfile.am
