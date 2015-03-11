

add_subdirectory(about)
add_subdirectory(icons)

include_directories(
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
  ${CMAKE_BINARY_DIR}/akregator/interfaces
)

########### next target ###############

set(akregator_SRCS main.cpp mainwindow.cpp akregator_debug.cpp)

file(GLOB ICONS_AKREGATOR_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-akregator.png")
ecm_add_app_icon(akregator_SRCS ICONS ${ICONS_AKREGATOR_SRCS})

add_executable(akregator ${akregator_SRCS})

target_link_libraries(akregator
  KF5::KontactInterface
  KF5::KontactInterface
  kdepim
  akregatorprivate
  akregatorinterfaces
)

install(TARGETS akregator ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(akregatorprivate_LIB_SRCS
  akregator_debug.cpp
  aboutdata.cpp
  trayicon.cpp
  article.cpp
  feed.cpp
  feedlist.cpp
  treenode.cpp
  treenodevisitor.cpp
  utils.cpp
  feediconmanager.cpp
  notificationmanager.cpp
  articlejobs.cpp
  folder.cpp
  kernel.cpp
  subscriptionlistjobs.cpp
  fetchqueue.cpp
  frame.cpp
  framemanager.cpp
  browserrun.cpp
  openurlrequest.cpp
  actionmanager.cpp
  browserframe.cpp
  browserframe_p.cpp
  actions.cpp
)

add_library(akregatorprivate ${akregatorprivate_LIB_SRCS})
generate_export_header(akregatorprivate BASE_NAME akregator)

target_link_libraries(akregatorprivate
PUBLIC
  KF5::KHtml
PRIVATE
  kdepim
  KF5::Syndication
  akregatorinterfaces
)

set_target_properties(akregatorprivate
  PROPERTIES VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)
install(TARGETS akregatorprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(akregator_utils_SRCS
  utils/filtercolumnsproxymodel.cpp
)

set(akregatorpart_PART_SRCS
  ${akregator_utils_SRCS}
  akregator_debug.cpp
  deletesubscriptioncommand.cpp
  abstractselectioncontroller.cpp
  articlematcher.cpp
  articlemodel.cpp
  pluginmanager.cpp
  selectioncontroller.cpp
  subscriptionlistmodel.cpp
  searchbar.cpp
  articlelistview.cpp
  actionmanagerimpl.cpp
  createfeedcommand.cpp
  createfoldercommand.cpp
  expireitemscommand.cpp
  articleviewer.cpp
  articleformatter.cpp
  addfeeddialog.cpp
  loadfeedlistcommand.cpp
  editsubscriptioncommand.cpp
  importfeedlistcommand.cpp
  feedpropertiesdialog.cpp
  tabwidget.cpp
  progressmanager.cpp
  akregator_part.cpp
  mainwidget.cpp
  subscriptionlistview.cpp
  subscriptionlistdelegate.cpp
  dummystorage/storagedummyimpl.cpp
  dummystorage/storagefactorydummyimpl.cpp
  dummystorage/feedstoragedummyimpl.cpp
)

qt5_add_dbus_adaptor(akregatorpart_PART_SRCS org.kde.akregator.part.xml akregator_part.h Akregator::Part)

ki18n_wrap_ui(akregatorpart_PART_SRCS
  addfeedwidgetbase.ui
  feedpropertieswidgetbase.ui
)

add_library(akregatorpart MODULE ${akregatorpart_PART_SRCS})
generate_export_header(akregatorpart BASE_NAME akregatorpart)
target_link_libraries(akregatorpart
  akregatorinterfaces
  akregatorprivate
  KF5::KCMUtils
  KF5::NotifyConfig
  kdepim
  KF5::Syndication
)

install(TARGETS akregatorpart DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(PROGRAMS org.kde.akregator.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES akregator_part.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES akregator.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES akregator_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})
install(FILES akregator_shell.rc articleviewer.rc pageviewer.rc akregator_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/akregator)
install(FILES feed.protocol DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES org.kde.akregator.part.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
install(FILES akregator.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR} )
