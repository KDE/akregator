INCLUDES = -I$(top_srcdir)/akregator/src/libsyndication/src \
           $(all_includes)
 

noinst_LTLIBRARIES = librss2local.la

librss2local_include_HEADERS = category.h cloud.h document.h enclosure.h \
                 image.h item.h parser.h rss2.h source.h textinput.h

librss2local_la_SOURCES = category.cpp cloud.cpp \
                                 document.cpp enclosure.cpp image.cpp \
                                 item.cpp parser.cpp source.cpp textinput.cpp \
                                 tools_p.cpp

librss2local_includedir = $(includedir)/libsyndication/rss2

# librss2local_la_METASOURCES = AUTO

include $(top_srcdir)/admin/Doxyfile.am
