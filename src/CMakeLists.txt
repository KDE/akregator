set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory( about ) 
add_subdirectory( icons )

include_directories (
    ${Boost_INCLUDE_DIR} 
    ${CMAKE_SOURCE_DIR}/akregator/interfaces 
    ${CMAKE_BINARY_DIR}/akregator/interfaces 
)

########### next target ###############

set(akregator_SRCS main.cpp mainwindow.cpp )

kde4_add_app_icon(akregator_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/akregator.png")

kde4_add_executable(akregator ${akregator_SRCS})

target_link_libraries(akregator ${KDE4_KPARTS_LIBS} kdepim akregatorprivate
                      akregatorinterfaces)

install(TARGETS akregator  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(akregatorprivate_LIB_SRCS
   aboutdata.cpp 
   trayicon.cpp 
)

kde4_add_library(akregatorprivate SHARED ${akregatorprivate_LIB_SRCS})

target_link_libraries(akregatorprivate kdepim  ${KDE4_KHTML_LIBS} ${KDE4_SYNDICATION_LIBS} akregatorinterfaces)
target_link_libraries(akregatorprivate LINK_INTERFACE_LIBRARIES ${KDE4_KHTML_LIBS} )

set_target_properties(akregatorprivate
   PROPERTIES VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS akregatorprivate ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set( akregator_utils_SRCS 
     utils/filtercolumnsproxymodel.cpp
)

set(akregatorpart_PART_SRCS 
   ${akregator_utils_SRCS}
   abstractselectioncontroller.cpp
   article.cpp
   articlejobs.cpp
   articlematcher.cpp
   articlemodel.cpp
   deletesubscriptioncommand.cpp
   feed.cpp 
   treenode.cpp 
   treenodevisitor.cpp 
   folder.cpp 
   feedlist.cpp 
   fetchqueue.cpp 
   feediconmanager.cpp 
   pluginmanager.cpp
   selectioncontroller.cpp
   subscriptionlistmodel.cpp
   subscriptionlistjobs.cpp
   utils.cpp
   searchbar.cpp 
   browserrun.cpp 
   articlelistview.cpp 
   actionmanager.cpp 
   actions.cpp
   actionmanagerimpl.cpp 
   createfeedcommand.cpp
   createfoldercommand.cpp
   expireitemscommand.cpp
   frame.cpp 
   framemanager.cpp 
   browserframe.cpp
   browserframe_p.cpp
   openurlrequest.cpp
   articleviewer.cpp 
   articleformatter.cpp
   addfeeddialog.cpp 
   editsubscriptioncommand.cpp
   feedpropertiesdialog.cpp 
   tabwidget.cpp 
   progressmanager.cpp 
   kernel.cpp 
   akregator_part.cpp 
   mainwidget.cpp 
   notificationmanager.cpp 
   subscriptionlistview.cpp
   dummystorage/storagedummyimpl.cpp 
   dummystorage/storagefactorydummyimpl.cpp 
   dummystorage/feedstoragedummyimpl.cpp )
   #speechclient.cpp )
qt4_add_dbus_adaptor( akregatorpart_PART_SRCS org.kde.akregator.part.xml akregator_part.h Akregator::Part )


kde4_add_ui_files(akregatorpart_PART_SRCS 
   addfeedwidgetbase.ui 
   feedpropertieswidgetbase.ui 
)

kde4_add_plugin(akregatorpart ${akregatorpart_PART_SRCS})

target_link_libraries(akregatorpart  ${KDE4_KUTILS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_SOLID_LIBRARY} akregatorinterfaces akregatorprivate kdepim ${KDE4_SYNDICATION_LIBS} ${KDE4_KPIMUTILS_LIBS}  ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS akregatorpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES akregator.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES akregator_part.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES articleviewer.rc pageviewer.rc akregator_part.rc akregator.notifyrc  DESTINATION ${DATA_INSTALL_DIR}/akregator)
install( FILES akregator_plugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES akregator_shell.rc  DESTINATION ${DATA_INSTALL_DIR}/akregator)
install( FILES feed.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES org.kde.akregator.part.xml DESTINATION ${KDE4_DBUS_INTERFACES_DIR} )
