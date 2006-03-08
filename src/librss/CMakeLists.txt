kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/src ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(testlibrss_SRCS testlibrss.cpp ${librsslocal_SRCS} )

kde4_automoc(${testlibrss_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testlibrss ${testlibrss_SRCS})

target_link_libraries(testlibrss  ${LIB_KIO} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#    -I$(top_srcdir)/src \
#    $(all_includes)
#
#noinst_LTLIBRARIES = \
#    librsslocal.la
#
#noinst_HEADERS = article.h document.h global.h image.h textinput.h \
#  loader.h librss.h enclosure.h
#
#librsslocal_la_SOURCES = article.cpp document.cpp image.cpp textinput.cpp \
#  tools_p.cpp loader.cpp enclosure.cpp category.cpp
#
#librsslocal_la_METASOURCES = AUTO
#
#check_PROGRAMS = testlibrss
#testlibrss_SOURCES = testlibrss.cpp
#testlibrss_LDFLAGS = $(all_libraries)
#testlibrss_LDADD =  librsslocal.la $(LIB_KIO)
