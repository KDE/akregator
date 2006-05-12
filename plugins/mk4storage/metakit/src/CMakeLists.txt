
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../include
#
#noinst_LTLIBRARIES = libmetakitlocal.la
#
#noinst_HEADERS = borc.h column.h column.inl custom.h derived.h field.h field.inl format.h gnuc.h handler.h handler.inl header.h mfc.h \
#		msvc.h mwcw.h persist.h remap.h std.h store.h store.inl univ.h univ.inl win.h doxy.h
#
#libmetakitlocal_la_SOURCES = column.cpp custom.cpp derived.cpp field.cpp fileio.cpp format.cpp handler.cpp persist.cpp remap.cpp std.cpp store.cpp \
#			    string.cpp table.cpp univ.cpp view.cpp viewx.cpp
#
