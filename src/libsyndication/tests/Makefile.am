INCLUDES = -I$(top_srcdir)/akregator/src/libsyndication/src $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = testlibsyndication testlibsyndication2 testpersonimpl 

# List of tests to run automatically from "make check"
TESTS = testpersonimpl testfeeds.rb

METASOURCES = AUTO

LDADD = $(top_builddir)/akregator/src/libsyndication/src/libsyndication.la -lQtTest_debug

testlibsyndication_SOURCES = testlibsyndication.cpp
testlibsyndication2_SOURCES = testlibsyndication2.cpp
testpersonimpl_SOURCES = testpersonimpl.cpp

