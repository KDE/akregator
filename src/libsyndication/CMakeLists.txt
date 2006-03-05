kde4_header()

add_subdirectory( src ) 
add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = src tests
#
#include $(top_srcdir)/admin/Doxyfile.am
#DOXYGEN_EMPTY = YES
#
