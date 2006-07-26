
add_subdirectory( about ) 
add_subdirectory( pics ) 

include_directories( ${CMAKE_SOURCE_DIR}/libsyndication/ )


########### next target ###############

set(akregator_SRCS main.cpp mainwindow.cpp )

kde4_automoc(${akregator_SRCS})

kde4_add_executable(akregator ${akregator_SRCS})

target_link_libraries(akregator  ${KDE4_KPARTS_LIBS} akregatorprivate)

install(TARGETS akregator  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(akregatorprivate_LIB_SRCS
   articlefilter.cpp 
   aboutdata.cpp 
   trayicon.cpp 
   article.cpp 
   feed.cpp 
   treenode.cpp 
   treenodevisitor.cpp 
   tagnode.cpp 
   folder.cpp 
   tagfolder.cpp 
   nodelist.cpp 
   feedlist.cpp 
   tagnodelist.cpp 
   fetchqueue.cpp 
   feediconmanager.cpp 
   articleinterceptor.cpp 
   plugin.cpp 
   pluginmanager.cpp 
   storagefactoryregistry.cpp 
   storage.cpp 
   tag.cpp 
   tagset.cpp 
   storagedummyimpl.cpp 
   storagefactorydummyimpl.cpp 
   simplenodeselector.cpp 
   feedstoragedummyimpl.cpp 
   dragobjects.cpp 
   utils.cpp )

kde4_automoc(${akregatorprivate_LIB_SRCS})


kde4_add_kcfg_files(akregatorprivate_LIB_SRCS akregatorconfig.kcfgc )

kde4_add_library(akregatorprivate SHARED ${akregatorprivate_LIB_SRCS})

target_link_libraries(akregatorprivate  ${KDE4_KDEUI_LIBS} kdepim  ${KDE4_KHTML_LIBS} syndication )

set_target_properties(akregatorprivate PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS akregatorprivate  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(akregatorpart_PART_SRCS 
   searchbar.cpp 
   browserrun.cpp 
   articlelistview.cpp 
   actionmanager.cpp 
   actionmanagerimpl.cpp 
   frame.cpp 
   framemanager.cpp 
   browserframe.cpp 
   viewer.cpp 
   articleviewer.cpp 
   addfeeddialog.cpp 
   propertiesdialog.cpp 
   tabwidget.cpp 
   feedlistview.cpp 
   treenodeitem.cpp 
   folderitem.cpp 
   feeditem.cpp 
   progressmanager.cpp 
   kernel.cpp 
   listtabwidget.cpp 
   tagnodeitem.cpp 
   configdialog.cpp 
   settings_advanced.cpp 
   akregator_part.cpp 
   mainwidget.cpp 
   notificationmanager.cpp 
   tagaction.cpp 
   tagpropertiesdialog.cpp 
   tagfolderitem.cpp )
   #speechclient.cpp )
qt4_add_dbus_adaptor( akregatorpart_PART_SRCS org.kde.akregator.part.xml akregator_part.h Akregator::Part )

kde4_automoc(${akregatorpart_PART_SRCS})

kde4_add_ui_files(akregatorpart_PART_SRCS 
   addfeedwidgetbase.ui 
   propertieswidgetbase.ui 
   settings_appearance.ui 
   settings_general.ui 
   settings_archive.ui 
   settings_browser.ui 
   settings_advancedbase.ui 
   tagpropertieswidgetbase.ui )



kde4_add_plugin(akregatorpart WITH_PREFIX ${akregatorpart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} akregatorpart )

target_link_libraries(akregatorpart  ${KDE4_KDECORE_LIBS} ${KDE4_KUTILS_LIBS} akregatorprivate )

install(TARGETS akregatorpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES akregator.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES akregator.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES akregator_part.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES articleviewer.rc pageviewer.rc akregator_part.rc eventsrc  DESTINATION ${DATA_INSTALL_DIR}/akregator)
install( FILES akregator_plugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES akregator_shell.rc  DESTINATION ${DATA_INSTALL_DIR}/akregator)
install( FILES feed.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES storage.h feedstorage.h storagefactory.h storagefactoryregistry.h plugin.h articleinterceptor.h  DESTINATION ${INCLUDE_INSTALL_DIR}/akregator)

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )
kde4_install_icons( ${ICON_INSTALL_DIR}  crystalsvg )



#original Makefile.am contents follow:

##set the include path for X, qt and KDE
#INCLUDES = -I$(top_srcdir)/akregator/src/librss -I$(top_srcdir) $(all_includes)
#
## let automoc handle all of the meta source files (moc).
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/akregator.pot
#
#KDE_ICON = AUTO
#
## this Makefile creates both a KPart application and a KPart
##########################################################################
## APPLICATION SECTION
##########################################################################
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#bin_PROGRAMS = akregator
#
## the application source, library search path, and link libraries
#akregator_SOURCES = main.cpp mainwindow.cpp
#akregator_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#akregator_LDADD = $(LIB_KPARTS) libakregatorprivate.la
##akregator_LDADD = $(LIB_KPARTS) libakregatorprivate.la $(top_builddir)/libkdepim/libkdepim.la
#
## this is where the desktop file will go
#xdg_apps_DATA = akregator.desktop
#
## this is where the shell's XML-GUI resource file goes
#shellrcdir = $(kde_datadir)/akregator
#shellrc_DATA = akregator_shell.rc
#
#kdelnk_DATA = feed.protocol
#kdelnkdir = $(kde_servicesdir)
#
##########################################################################
## PRIVATE SECTION
##########################################################################
#
## install headers (for storage plugins)
#akregatorinclude_HEADERS = storage.h feedstorage.h storagefactory.h storagefactoryregistry.h plugin.h articleinterceptor.h
#akregatorincludedir = $(includedir)/akregator
#
## this is a lib shared by both the KPart and the main application
#lib_LTLIBRARIES = libakregatorprivate.la
#libakregatorprivate_la_SOURCES = akregatorconfig.kcfgc \
#                                 articlefilter.cpp \
#				 aboutdata.cpp \
#                                 trayicon.cpp \
#				 article.cpp \
#				 feed.cpp \
#				 treenode.cpp \
#                                 treenodevisitor.cpp \
#                                 tagnode.cpp \
#				 folder.cpp \
#                                 tagfolder.cpp \
#                                 nodelist.cpp \
#				 feedlist.cpp \
#                                 tagnodelist.cpp \
#			         fetchqueue.cpp \
#			  	 feediconmanager.cpp \
#				feediconmanager.skel \
#                                 articleinterceptor.cpp \
#                                plugin.cpp \
# 				pluginmanager.cpp \
#				storagefactoryregistry.cpp \
#				storage.cpp \
#                                tag.cpp \
#                                tagset.cpp \
#                                storagedummyimpl.cpp \
#                                storagefactorydummyimpl.cpp \
#                                simplenodeselector.cpp \
#                                feedstoragedummyimpl.cpp \
#                                dragobjects.cpp \
#                                utils.cpp
#				
#libakregatorprivate_la_LDFLAGS = $(all_libraries) -avoid-version -no-undefined 
#libakregatorprivate_la_LIBADD =	$(top_builddir)/libkdepim/libkdepim.la \
#	$(top_builddir)/akregator/src/librss/librsslocal.la $(LIB_KHTML)
#
##########################################################################
## KPART SECTION
##########################################################################
#
#kde_kcfg_DATA = akregator.kcfg
#kde_module_LTLIBRARIES = libakregatorpart.la
#
#mainwindow.lo: akregatorconfig.h
#akregator_part.lo: akregatorconfig.h
#mainwidget.lo: akregatorconfig.h
#akregatorconfig.lo: akregatorconfig.h
#articlelistview.lo: akregatorconfig.h
#articleviewer.lo: akregatorconfig.h
#feed.lo: akregatorconfig.h
#fetchqueue.lo: akregatorconfig.h
#trayicon.lo: akregatorconfig.h
#viewer.lo: akregatorconfig.h
#searchbar.lo: akregatorconfig.h
#
#kspeech_DIR = $(kde_includes)
#kspeechsink_DIR = $(kde_includes)
#
## the Part's source, library search path, and link libraries
#libakregatorpart_la_SOURCES = \
#	searchbar.cpp \
#	browserrun.cpp \
#	articlelistview.cpp \
#	actionmanager.cpp \
#        actionmanagerimpl.cpp \
#	frame.cpp \
#        framemanager.cpp \
#        browserframe.cpp \
#	viewer.cpp \
#	articleviewer.cpp \
#	addfeeddialog.cpp \
#	addfeedwidgetbase.ui \
#	propertiesdialog.cpp \
#	propertieswidgetbase.ui \
#	tabwidget.cpp \
#	feedlistview.cpp \
#	treenodeitem.cpp \
#	folderitem.cpp \
#	feeditem.cpp \
#        progressmanager.cpp \
#        kernel.cpp \
#        listtabwidget.cpp \
#        settings_appearance.ui \
#	settings_general.ui \
#	settings_archive.ui \
#	settings_browser.ui \
#	settings_advancedbase.ui \
#        tagnodeitem.cpp \
#	configdialog.cpp \
#	settings_advanced.cpp \
#	akregator_partiface.skel \
#	akregator_part.cpp \
#	mainwidget.cpp \
#        notificationmanager.cpp \
#        tagaction.cpp \
#        tagpropertieswidgetbase.ui \
#        tagpropertiesdialog.cpp \
#        tagfolderitem.cpp \
#        speechclient.cpp \
#	kspeechsink.skel \
#	kspeech.stub
#
#libakregatorpart_la_LDFLAGS = $(KDE_RPATH) $(KDE_PLUGIN) $(LIB_KUTILS) -avoid-version -no-undefined $(all_libraries)
#libakregatorpart_la_LIBADD = libakregatorprivate.la 
#
## this is where the desktop file will go
#partdesktopdir = $(kde_servicesdir)
#partdesktop_DATA = akregator_part.desktop
#
## this is where the part's XML-GUI resource file goes
#partrcdir = $(kde_datadir)/akregator
#partrc_DATA = articleviewer.rc pageviewer.rc akregator_part.rc eventsrc
#
#kde_servicetypes_DATA = akregator_plugin.desktop
#
#SUBDIRS = librss libsyndication about . mk4storage pics
#
##build apidox in subdirs, disable it for this dir for now
#
#DOXYGEN_EMPTY = YES
#include $(top_srcdir)/admin/Doxyfile.am
#
#
