INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libatomlocal.la

noinst_HEADERS = category.h constants.h content.h document.h entry.h \
                 generator.h link.h parser.h person.h source.h tools.h

libatomlocal_la_SOURCES = category.cpp constants.cpp content.cpp document.cpp \
                          entry.cpp generator.cpp link.cpp parser.cpp \
                          person.cpp source.cpp tools.cpp

libatomlocal_la_METASOURCES = AUTO

include $(top_srcdir)/admin/Doxyfile.am
