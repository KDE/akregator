# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/akregator/src/librss $(all_includes)

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
akregator_SOURCES = main.cpp app.cpp akregator.cpp trayicon.cpp balloon.cpp akregatorconfig.kcfgc settings_general.ui settings_archive.ui settings_browser.ui
akregator_LDFLAGS = $(KDE_RPATH) $(all_libraries)
akregator_LDADD = $(LIB_KPARTS)

# this is where the desktop file will go
shelldesktopdir = $(kde_appsdir)/Internet
shelldesktop_DATA = akregator.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir = $(kde_datadir)/akregator
shellrc_DATA = akregator_shell.rc

kdelnk_DATA = feed.protocol
kdelnkdir = $(kde_servicesdir)

#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libakregatorpart.la

# the Part's source, library search path, and link libraries
libakregatorpart_la_SOURCES = akregatorconfig.kcfgc akregator_part.cpp akregator_view.cpp akregator_run.cpp articlesequence.cpp intervalmanager.cpp feedstree.cpp feedscollection.cpp feed.cpp fetchtransaction.cpp articlelist.cpp frame.cpp feediconmanager.cpp feediconmanager.skel viewer.cpp articleviewer.cpp feedgroup.cpp exportwidgetbase.ui addfeeddialog.cpp addfeedwidgetbase.ui propertiesdialog.cpp propertieswidgetbase.ui myarticle.cpp articlefilter.cpp archive.cpp pageviewer.cpp tabwidget.cpp akregator_partiface.skel
libakregatorpart_la_LDFLAGS = $(KDE_RPATH) -version-info 1:0 -no-undefined $(all_libraries)
libakregatorpart_la_LIBADD = $(top_builddir)/akregator/src/librss/librsslocal.la $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KHTML)

kde_kcfg_DATA = akregator.kcfg

# this is where the desktop file will go
partdesktopdir = $(kde_servicesdir)
partdesktop_DATA = akregator_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir = $(kde_datadir)/akregatorpart
partrc_DATA = akregator_part.rc welcome.html

if include_kontact
KONTACT_SUBDIR = kontact_plugin
endif

if include_konqplugin
KONQPLUGIN_SUBDIR = konq_plugin
endif

SUBDIRS = librss $(KONTACT_SUBDIR) $(KONQPLUGIN_SUBDIR)

noinst_HEADERS = archive.h
