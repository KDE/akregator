#set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/akregator/src/librss -I$(top_srcdir) $(all_includes)

# let automoc handle all of the meta source files (moc).
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp
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
akregator_SOURCES = main.cpp mainwindow.cpp
akregator_LDFLAGS = $(KDE_RPATH) $(all_libraries)
akregator_LDADD = $(LIB_KPARTS) libakregatorprivate.la
#akregator_LDADD = $(LIB_KPARTS) libakregatorprivate.la $(top_builddir)/libkdepim/libkdepim.la

# this is where the desktop file will go
xdg_apps_DATA = akregator.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir = $(kde_datadir)/akregator
shellrc_DATA = akregator_shell.rc

kdelnk_DATA = feed.protocol
kdelnkdir = $(kde_servicesdir)

#########################################################################
# PRIVATE SECTION
#########################################################################

# install headers (for storage plugins)
akregatorinclude_HEADERS = storage.h feedstorage.h storagefactory.h storagefactoryregistry.h plugin.h articleinterceptor.h
akregatorincludedir = $(includedir)/akregator

# this is a lib shared by both the KPart and the main application
lib_LTLIBRARIES = libakregatorprivate.la
libakregatorprivate_la_SOURCES = akregatorconfig.kcfgc \
                                 articlefilter.cpp \
				 aboutdata.cpp \
                                 trayicon.cpp \
				 article.cpp \
				 feed.cpp \
				 treenode.cpp \
                                 treenodevisitor.cpp \
                                 tagnode.cpp \
				 folder.cpp \
                                 tagfolder.cpp \
                                 nodelist.cpp \
				 feedlist.cpp \
                                 tagnodelist.cpp \
			         fetchqueue.cpp \
			  	 feediconmanager.cpp \
				feediconmanager.skel \
                                 articleinterceptor.cpp \
                                plugin.cpp \
 				pluginmanager.cpp \
				storagefactoryregistry.cpp \
				storage.cpp \
                                tag.cpp \
                                tagset.cpp \
                                storagedummyimpl.cpp \
                                storagefactorydummyimpl.cpp \
                                simplenodeselector.cpp \
                                feedstoragedummyimpl.cpp \
                                dragobjects.cpp \
                                utils.cpp
				
libakregatorprivate_la_LDFLAGS = $(all_libraries) -avoid-version -no-undefined 
libakregatorprivate_la_LIBADD =	$(top_builddir)/libkdepim/libkdepim.la \
	$(top_builddir)/akregator/src/librss/librsslocal.la $(LIB_KHTML)

#########################################################################
# KPART SECTION
#########################################################################

kde_kcfg_DATA = akregator.kcfg
kde_module_LTLIBRARIES = libakregatorpart.la

mainwindow.lo: akregatorconfig.h
akregator_part.lo: akregatorconfig.h
akregator_view.lo: akregatorconfig.h
akregatorconfig.lo: akregatorconfig.h
articlelistview.lo: akregatorconfig.h
articleviewer.lo: akregatorconfig.h
feed.lo: akregatorconfig.h
fetchqueue.lo: akregatorconfig.h
trayicon.lo: akregatorconfig.h
viewer.lo: akregatorconfig.h
searchbar.lo: akregatorconfig.h

kspeech_DIR = $(kde_includes)
kspeechsink_DIR = $(kde_includes)

# the Part's source, library search path, and link libraries
libakregatorpart_la_SOURCES = \
	searchbar.cpp \
	akregator_run.cpp \
	articlelistview.cpp \
	actionmanager.cpp \
        actionmanagerimpl.cpp \
	frame.cpp \
	viewer.cpp \
	articleviewer.cpp \
	addfeeddialog.cpp \
	addfeedwidgetbase.ui \
	propertiesdialog.cpp \
	propertieswidgetbase.ui \
	pageviewer.cpp \
	tabwidget.cpp \
	feedlistview.cpp \
	treenodeitem.cpp \
	folderitem.cpp \
	feeditem.cpp \
        progressmanager.cpp \
        kernel.cpp \
        listtabwidget.cpp \
        settings_appearance.ui \
	settings_general.ui \
	settings_archive.ui \
	settings_browser.ui \
	settings_advancedbase.ui \
        tagnodeitem.cpp \
	configdialog.cpp \
	settings_advanced.cpp \
	akregator_partiface.skel \
	akregator_part.cpp \
	akregator_view.cpp \
        notificationmanager.cpp \
        tagaction.cpp \
        tagpropertieswidgetbase.ui \
        tagpropertiesdialog.cpp \
        tagfolderitem.cpp \
        speechclient.cpp \
	kspeechsink.skel \
	kspeech.stub

libakregatorpart_la_LDFLAGS = $(KDE_RPATH) $(KDE_PLUGIN) $(LIB_KUTILS) -avoid-version -no-undefined $(all_libraries)
libakregatorpart_la_LIBADD = libakregatorprivate.la 

# this is where the desktop file will go
partdesktopdir = $(kde_servicesdir)
partdesktop_DATA = akregator_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir = $(kde_datadir)/akregator
partrc_DATA = articleviewer.rc pageviewer.rc akregator_part.rc eventsrc

kde_servicetypes_DATA = akregator_plugin.desktop


SUBDIRS = librss about . mk4storage pics


