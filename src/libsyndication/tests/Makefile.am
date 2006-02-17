INCLUDES = -I$(top_srcdir)/akregator/src/libsyndication/src $(all_includes)

METASOURCES = AUTO

# TODO: make proper test suite
bin_PROGRAMS = testlibsyndication testpersonfromstring
testlibsyndication_SOURCES = testlibsyndication.cpp
testlibsyndication_LDFLAGS = $(all_libraries)
testlibsyndication_LDADD = $(top_builddir)/akregator/src/libsyndication/src/libsyndication.la
testpersonfromstring_SOURCES = testpersonfromstring.cpp
testpersonfromstring_LDFLAGS = $(all_libraries)
testpersonfromstring_LDADD = $(top_builddir)/akregator/src/libsyndication/src/libsyndication.la

