#set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/akregator/src/librss -I$(top_srcdir) $(all_includes)

# let automoc handle all of the meta source files (moc).
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/akregator.pot

KDE_ICON = AUTO

# this Makefile creates both a KPart application and a KPart
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = akregator

# the application source, library search path, and link libraries
akregator_SOURCES = main.cpp app.cpp akregator.cpp 
akregator_LDFLAGS = $(KDE_RPATH) $(all_libraries)
akregator_LDADD = $(LIB_KPARTS) libakregatorprivate.la $(top_builddir)/libkdepim/libkdepim.la

# this is where the desktop file will go
shelldesktopdir = $(kde_appsdir)/Internet
shelldesktop_DATA = akregator.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir = $(kde_datadir)/akregator
shellrc_DATA = akregator_shell.rc

kdelnk_DATA = feed.protocol
kdelnkdir = $(kde_servicesdir)

#########################################################################
# PRIVATE SECTION
#########################################################################

# this is a lib shared by both the KPart and the main application
lib_LTLIBRARIES = libakregatorprivate.la
libakregatorprivate_la_SOURCES = akregatorconfig.kcfgc aboutdata.cpp
libakregatorprivate_la_LDFLAGS = $(all_libraries) -avoid-version -no-undefined
libakregatorprivate_la_LIBADD = $(top_builddir)/akregator/src/librss/librsslocal.la $(LIB_KHTML)

#########################################################################
# KPART SECTION
#########################################################################

kde_kcfg_DATA = akregator.kcfg
kde_module_LTLIBRARIES = libakregatorpart.la

archive.lo: akregatorconfig.h
akregator.lo: akregatorconfig.h
akregator_part.lo: akregatorconfig.h
akregator_view.lo: akregatorconfig.h
akregatorconfig.lo: akregatorconfig.h
articlelist.lo: akregatorconfig.h
articleviewer.lo: akregatorconfig.h
feed.lo: akregatorconfig.h
fetchtransaction.lo: akregatorconfig.h
trayicon.lo: akregatorconfig.h
viewer.lo: akregatorconfig.h

# the Part's source, library search path, and link libraries
libakregatorpart_la_SOURCES = \
	akregator_run.cpp \
	articlesequence.cpp \
	intervalmanager.cpp \
	feedstree.cpp \
	feed.cpp \
	fetchtransaction.cpp \
	articlelist.cpp \
	frame.cpp \
	feediconmanager.cpp \
	viewer.cpp \
	articleviewer.cpp \
	feedgroup.cpp \
	exportwidgetbase.ui \
	addfeeddialog.cpp \
	addfeedwidgetbase.ui \
	propertiesdialog.cpp \
	propertieswidgetbase.ui \
	myarticle.cpp \
	articlefilter.cpp \
	archive.cpp \
	pageviewer.cpp \
	tabwidget.cpp \
	treenode.cpp \
	treenodeitem.cpp \
	feedgroupitem.cpp \
	feeditem.cpp \
	feedlist.cpp \
	settings_appearance.ui \
	settings_general.ui \
	settings_archive.ui \
	settings_browser.ui \
	trayicon.cpp \
	feediconmanager.skel \
	akregator_partiface.skel \
	akregator_part.cpp \
	akregator_view.cpp \
        notificationmanager.cpp

libakregatorpart_la_LDFLAGS = $(KDE_RPATH) $(KDE_PLUGIN) -avoid-version -no-undefined $(all_libraries)
libakregatorpart_la_LIBADD = libakregatorprivate.la $(top_builddir)/libkdepim/libkdepim.la

# this is where the desktop file will go
partdesktopdir = $(kde_servicesdir)
partdesktop_DATA = akregator_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir = $(kde_datadir)/akregator
partrc_DATA = articleviewer.rc pageviewer.rc akregator_part.rc welcome.html eventsrc

# this is where the icons go
iconsdir = $(kde_datadir)/akregator/pics
icons_DATA = akregator_flag.png

SUBDIRS = librss

noinst_HEADERS = archive.h
