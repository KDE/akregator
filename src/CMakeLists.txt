include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/command/
    ${CMAKE_CURRENT_SOURCE_DIR}/formatter/
    ${CMAKE_CURRENT_SOURCE_DIR}/subscription/
    ${CMAKE_CURRENT_SOURCE_DIR}/feed/
    ${CMAKE_CURRENT_SOURCE_DIR}/urlhandler/
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/
    ${CMAKE_CURRENT_SOURCE_DIR}/frame/
    ${CMAKE_CURRENT_SOURCE_DIR}/searchbar/
    )


add_subdirectory(about)
add_subdirectory(icons)


########### next target ###############

set(akregator_common_SRCS)
ecm_qt_declare_logging_category(akregator_common_SRCS HEADER akregator_debug.h IDENTIFIER AKREGATOR_LOG CATEGORY_NAME log_akregator)

set(akregator_SRCS main.cpp mainwindow.cpp ${akregator_common_SRCS})

file(GLOB ICONS_AKREGATOR_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-akregator.png")
ecm_add_app_icon(akregator_SRCS ICONS ${ICONS_AKREGATOR_SRCS})

add_executable(akregator ${akregator_SRCS})

target_link_libraries(akregator
    KF5::Notifications
    KF5::KontactInterface
    KF5::Libkdepim
    akregatorprivate
    akregatorinterfaces
    KF5::PimCommon
    Qt5::PrintSupport
    )

install(TARGETS akregator ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(akregator_articleviewer_ng_SRCS
    articleviewer-ng/articleviewerng.cpp
    articleviewer-ng/articleviewerwidgetng.cpp
    articleviewer-ng/articlehtmlwriter.cpp

    )

set(akregatorprivate_formatter_SRCS
    formatter/articleformatter.cpp
    formatter/defaultnormalviewformatter.cpp
    formatter/defaultcombinedviewformatter.cpp
    formatter/grantleeviewformatter.cpp
    formatter/articlegrantleeobject.cpp
    )

set(akregatorprivate_frame_SRCS
    frame/webviewer.cpp
    frame/frame.cpp
    frame/mainframe.cpp
    frame/framemanager.cpp
    frame/webviewframe.cpp
    )

set(akregatorprivate_urlhandler_SRCS
    urlhandler/urlhandlermanager.cpp
    urlhandler/urlhandler.cpp
    )

set(akregatorprivate_LIB_SRCS
    akregratormigrateapplication.cpp
    ${akregatorprivate_frame_SRCS}
    ${akregatorprivate_urlhandler_SRCS}
    ${akregator_articleviewer_ng_SRCS}
    ${akregator_common_SRCS}
    ${akregatorprivate_formatter_SRCS}
    articleviewerwidget.cpp
    aboutdata.cpp
    trayicon.cpp
    article.cpp
    feed/feed.cpp
    feed/feedlist.cpp
    feed/feediconmanager.cpp
    treenode.cpp
    treenodevisitor.cpp
    utils.cpp
    notificationmanager.cpp
    articlejobs.cpp
    folder.cpp
    kernel.cpp
    subscription/subscriptionlistjobs.cpp
    fetchqueue.cpp
    openurlrequest.cpp
    actions/actionmanager.cpp
    actions/actions.cpp
    )

add_library(akregatorprivate ${akregatorprivate_LIB_SRCS})
generate_export_header(akregatorprivate BASE_NAME akregator)

target_link_libraries(akregatorprivate
    PRIVATE
    KF5::KDELibs4Support
    KF5::Parts
    KF5::Notifications
    KF5::Libkdepim
    KF5::Syndication
    akregatorinterfaces
    KF5::PimCommon
    KF5::WindowSystem
    KF5::IconThemes
    KF5::PimTextEdit
    KF5::GrantleeTheme
    Grantlee5::Templates
    KF5::WebKit
    KF5::MessageViewer
    Qt5::PrintSupport
    )

target_include_directories(akregatorprivate PUBLIC "$<BUILD_INTERFACE:${akregator_SOURCE_DIR}/src;${akregator_BINARY_DIR}/src>")

set_target_properties(akregatorprivate
    PROPERTIES VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
    )
install(TARGETS akregatorprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(akregator_utils_SRCS
    utils/filtercolumnsproxymodel.cpp
    )

set(akregatorpart_command_SRCS
    command/deletesubscriptioncommand.cpp
    command/createfeedcommand.cpp
    command/createfoldercommand.cpp
    command/expireitemscommand.cpp
    command/loadfeedlistcommand.cpp
    command/editsubscriptioncommand.cpp
    command/importfeedlistcommand.cpp
    )

set(akregatorpart_widgets_SRCS
    widgets/statussearchline.cpp
    widgets/searchbar.cpp
   )

set(akregatorpart_subscription_SRCS
    subscription/subscriptionlistview.cpp
    subscription/subscriptionlistdelegate.cpp
    subscription/subscriptionlistmodel.cpp
    )

set(akregatorpart_PART_SRCS
    ${akregatorpart_subscription_SRCS}
    ${akregatorpart_widgets_SRCS}
    ${akregatorpart_command_SRCS}
    ${akregator_utils_SRCS}
    ${akregator_common_SRCS}
    abstractselectioncontroller.cpp
    articlematcher.cpp
    articlemodel.cpp
    pluginmanager.cpp
    selectioncontroller.cpp
    articlelistview.cpp
    actions/actionmanagerimpl.cpp
    addfeeddialog.cpp
    feed/feedpropertiesdialog.cpp
    tabwidget.cpp
    progressmanager.cpp
    akregator_part.cpp
    mainwidget.cpp
    dummystorage/storagedummyimpl.cpp
    dummystorage/storagefactorydummyimpl.cpp
    dummystorage/feedstoragedummyimpl.cpp
    )

qt5_add_dbus_adaptor(akregatorpart_PART_SRCS org.kde.akregator.part.xml akregator_part.h Akregator::Part)

ki18n_wrap_ui(akregatorpart_PART_SRCS
    ui/addfeedwidgetbase.ui
    ui/feedpropertieswidgetbase.ui
    )

add_library(akregatorpart MODULE ${akregatorpart_PART_SRCS})
generate_export_header(akregatorpart BASE_NAME akregatorpart)
target_link_libraries(akregatorpart
    KF5::KDELibs4Support
    akregatorinterfaces
    akregatorprivate
    KF5::KCMUtils
    KF5::NotifyConfig
    KF5::Libkdepim
    KF5::Syndication
    KF5::PimCommon
    KF5::PimTextEdit
    KF5::GrantleeTheme
    KF5::WebKit
    Qt5::WebKitWidgets
    KF5::MessageViewer
    )

install(TARGETS akregatorpart DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(PROGRAMS data/org.kde.akregator.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/akregator_part.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES data/akregator.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/akregator_plugin.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})
install(FILES data/akregator_shell.rc data/akregator_part.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/akregator)
install(FILES feed.protocol DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES org.kde.akregator.part.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
install(FILES data/akregator.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR} )

add_subdirectory(formatter/html)
