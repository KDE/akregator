# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/akregator/src/librss $(all_includes)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/akregator.pot

KDE_ICON = akregator

# this Makefile creates both a KPart application and a KPart
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = akregator

# the application source, library search path, and link libraries
akregator_SOURCES = main.cpp akregator.cpp trayicon.cpp
akregator_LDFLAGS = $(KDE_RPATH) $(all_libraries)
akregator_LDADD = $(LIB_KPARTS)

# this is where the desktop file will go 
shelldesktopdir = $(kde_appsdir)/Internet
shelldesktop_DATA = akregator.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir = $(kde_datadir)/akregator
shellrc_DATA = akregator_shell.rc

#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libakregatorpart.la

# the Part's source, library search path, and link libraries
libakregatorpart_la_SOURCES = akregator_part.cpp feedstree.cpp feedscollection.cpp feed.cpp articlelist.cpp feediconmanager.cpp feediconmanager.skel articleviewer.cpp feedgroup.cpp addfeeddialog.cpp addfeeddialogbase.ui myarticle.cpp archive.cpp
libakregatorpart_la_LDFLAGS = $(KDE_RPATH) -version-info 1:0 -no-undefined $(all_libraries) 
libakregatorpart_la_LIBADD = $(top_builddir)/akregator/src/librss/librsslocal.la $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KHTML)

# this is where the desktop file will go 
partdesktopdir = $(kde_servicesdir)
partdesktop_DATA = akregator_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir = $(kde_datadir)/akregatorpart
partrc_DATA = akregator_part.rc welcome.html
SUBDIRS = librss

noinst_HEADERS =  archive.h
