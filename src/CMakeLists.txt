add_subdirectory( about ) 
add_subdirectory( icons ) 

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include_directories ( ${Boost_INCLUDE_DIR} )

########### next target ###############

set(akregator_SRCS main.cpp mainwindow.cpp )

kde4_automoc(${akregator_SRCS})

kde4_add_executable(akregator ${akregator_SRCS})

target_link_libraries(akregator ${KDE4_KPARTS_LIBS} akregatorprivate )

install(TARGETS akregator  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(akregatorprivate_LIB_SRCS
   plugin.cpp 
   aboutdata.cpp 
   trayicon.cpp 
   storagefactoryregistry.cpp 
   storage.cpp
)

kde4_automoc(${akregatorprivate_LIB_SRCS})


kde4_add_kcfg_files(akregatorprivate_LIB_SRCS akregatorconfig.kcfgc )

kde4_add_library(akregatorprivate SHARED ${akregatorprivate_LIB_SRCS})

target_link_libraries(akregatorprivate  kdepim  ${KDE4_KHTML_LIBS} ${KDE4_SYNDICATION_LIBS} )

set_target_properties(akregatorprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS akregatorprivate  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(akregatorpart_PART_SRCS 
   abstractselectioncontroller.cpp
   article.cpp 
   articlefilter.cpp 
   articlemodel.cpp
   feed.cpp 
   treenode.cpp 
   treenodevisitor.cpp 
   folder.cpp 
   nodelist.cpp 
   feedlist.cpp 
   fetchqueue.cpp 
   feediconmanager.cpp 
   articleinterceptor.cpp 
   pluginmanager.cpp
   selectioncontroller.cpp
   subscriptionlistmodel.cpp
   storagedummyimpl.cpp 
   storagefactorydummyimpl.cpp 
   feedstoragedummyimpl.cpp 
   dragobjects.cpp 
   utils.cpp
   searchbar.cpp 
   browserrun.cpp 
   articlelistview.cpp 
   actionmanager.cpp 
   actionmanagerimpl.cpp 
   frame.cpp 
   framemanager.cpp 
   browserframe.cpp
   browserframe_p.cpp
   openurlrequest.cpp
   articleviewer.cpp 
   articleformatter.cpp
   addfeeddialog.cpp 
   propertiesdialog.cpp 
   tabwidget.cpp 
   feedlistview.cpp 
   treenodeitem.cpp 
   folderitem.cpp 
   feeditem.cpp 
   progressmanager.cpp 
   kernel.cpp 
   configdialog.cpp 
   settings_advanced.cpp 
   akregator_part.cpp 
   mainwidget.cpp 
   notificationmanager.cpp 
   subscriptionlistview.cpp )
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
   settings_advancedbase.ui )



kde4_add_plugin(akregatorpart WITH_PREFIX ${akregatorpart_PART_SRCS})



target_link_libraries(akregatorpart  ${KDE4_KDECORE_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} akregatorprivate)

install(TARGETS akregatorpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES akregator.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES akregator.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES akregator_part.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES articleviewer.rc pageviewer.rc akregator_part.rc akregator.notifyrc  DESTINATION ${DATA_INSTALL_DIR}/akregator)
install( FILES akregator_plugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES akregator_shell.rc  DESTINATION ${DATA_INSTALL_DIR}/akregator)
install( FILES feed.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
#install( FILES storage.h feedstorage.h storagefactory.h storagefactoryregistry.h plugin.h articleinterceptor.h  DESTINATION ${INCLUDE_INSTALL_DIR}/akregator)
install( FILES org.kde.akregator.part.xml DESTINATION ${DBUS_INTERFACES_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )
