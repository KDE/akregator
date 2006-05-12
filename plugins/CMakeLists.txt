
add_subdirectory( mk4storage ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )





#original Makefile.am contents follow:

#SUBDIRS = src
#EXTRA_DIST = AUTHORS COPYING ChangeLog INSTALL NEWS README TODO akregator.lsm
#AUTOMAKE_OPTIONS = foreign
#
## build apidox
#DOXYGEN_EMPTY = YES
#include $(top_srcdir)/admin/Doxyfile.am
#
#
#
