INCLUDES = \
    -I$(top_srcdir)/src \
    $(all_includes)

noinst_LIBRARIES = \
    librsslocal.a

noinst_HEADERS = article.h document.h global.h image.h textinput.h \
  loader.h librss.h

librsslocal_a_SOURCES = article.cpp document.cpp image.cpp textinput.cpp \
  tools_p.cpp loader.cpp

librsslocal_a_METASOURCES = AUTO

check_PROGRAMS = testlibrss
testlibrss_SOURCES = testlibrss.cpp
testlibrss_LDFLAGS = $(all_libraries)
testlibrss_LDADD = librsslocal.a
