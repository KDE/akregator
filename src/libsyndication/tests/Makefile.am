INCLUDES = -I$(top_srcdir)/akregator/src/libsyndication/src $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = testlibsyndication testpersonimpl

# List of tests to run automatically from "make check"
TESTS = testpersonimpl

METASOURCES = AUTO

LDADD = $(top_builddir)/akregator/src/libsyndication/src/libsyndication.la -lQtTest_debug

testlibsyndication_SOURCES = testlibsyndication.cpp
testpersonimpl_SOURCES = testpersonimpl.cpp

