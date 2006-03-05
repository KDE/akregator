kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############


########### next target ###############


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#noinst_LTLIBRARIES = libmappercommonlocal.la libmapperrss2local.la \
#                     libmapperrdflocal.la libmapperatomlocal.la
#
#noinst_HEADERS = personimpl.h \
#                 feedrss2impl.h itemrss2impl.h enclosurerss2impl.h \
#                 categoryrss2impl.h imagerss2impl.h \
#                 feedrdfimpl.h itemrdfimpl.h imagerdfimpl.h \
#                 feedatomimpl.h itematomimpl.h enclosureatomimpl.h \
#                 categoryatomimpl.h
#
#libmappercommonlocal_la_SOURCES = personimpl.cpp
#
#libmapperrss2local_la_SOURCES = feedrss2impl.cpp itemrss2impl.cpp \
#                                enclosurerss2impl.cpp categoryrss2impl.cpp \
#                                imagerss2impl.cpp
#libmapperrdflocal_la_SOURCES = feedrdfimpl.cpp itemrdfimpl.cpp \
#                               imagerdfimpl.cpp
#libmapperatomlocal_la_SOURCES = feedatomimpl.cpp itematomimpl.cpp \
#                                enclosureatomimpl.cpp categoryatomimpl.cpp
