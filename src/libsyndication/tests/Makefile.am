INCLUDES = -I$(top_srcdir)/akregator/src/libsyndication/src $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = testloader testlibsyndication testpersonimpl benchmark

# List of tests to run automatically from "make check"
TESTS = testpersonimpl testfeeds.rb

METASOURCES = AUTO

LDADD = $(top_builddir)/akregator/src/libsyndication/src/libsyndication.la -lQtTest_debug

testloader_SOURCES = testloader.cpp
testlibsyndication_SOURCES = testlibsyndication.cpp
testpersonimpl_SOURCES = testpersonimpl.cpp
benchmark_SOURCES = benchmark.cpp

