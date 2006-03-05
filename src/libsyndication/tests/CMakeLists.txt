kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/akregator/src/libsyndication/src ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(testlibsyndication_SRCS testlibsyndication.cpp )

kde4_automoc(${testlibsyndication_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testlibsyndication ${testlibsyndication_SRCS})

target_link_libraries(testlibsyndication  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testlibsyndication2_SRCS testlibsyndication2.cpp )

kde4_automoc(${testlibsyndication2_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testlibsyndication2 ${testlibsyndication2_SRCS})

target_link_libraries(testlibsyndication2  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testpersonimpl_SRCS testpersonimpl.cpp )

kde4_automoc(${testpersonimpl_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testpersonimpl ${testpersonimpl_SRCS})

target_link_libraries(testpersonimpl  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akregator/src/libsyndication/src $(all_includes)
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#
#check_PROGRAMS = testlibsyndication testlibsyndication2 testpersonimpl 
#
## List of tests to run automatically from "make check"
#TESTS = testpersonimpl testfeeds.rb
#
#METASOURCES = AUTO
#
#LDADD = $(top_builddir)/akregator/src/libsyndication/src/libsyndication.la -lQtTest_debug
#
#testlibsyndication_SOURCES = testlibsyndication.cpp
#testlibsyndication2_SOURCES = testlibsyndication2.cpp
#testpersonimpl_SOURCES = testpersonimpl.cpp
#
