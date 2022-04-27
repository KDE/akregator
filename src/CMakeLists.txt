# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
configure_file(akregator-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/akregator-version.h @ONLY)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/command/
    ${CMAKE_CURRENT_SOURCE_DIR}/formatter/
    ${CMAKE_CURRENT_SOURCE_DIR}/subscription/
    ${CMAKE_CURRENT_SOURCE_DIR}/feed/
    ${CMAKE_CURRENT_SOURCE_DIR}/urlhandler/
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/
    ${CMAKE_CURRENT_SOURCE_DIR}/frame/
    )

add_subdirectory(icons)


########### next target ###############

set(akregator_common_SRCS)
ecm_qt_declare_logging_category(akregator_common_SRCS HEADER akregator_debug.h IDENTIFIER AKREGATOR_LOG CATEGORY_NAME org.kde.pim.akregator
        DESCRIPTION "akregator (akregator)"
        OLD_CATEGORY_NAMES log_akregator
        EXPORT AKREGATOR
    )

set(akregator_SRCS main.cpp mainwindow.cpp mainwindow.h ${akregator_common_SRCS})

file(GLOB ICONS_AKREGATOR_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-akregator.png")
ecm_add_app_icon(akregator_SRCS ICONS ${ICONS_AKREGATOR_SRCS})

add_executable(akregator ${akregator_SRCS})

target_link_libraries(akregator
    KF5::I18n
    KF5::Crash
    KF5::Notifications
    KF5::KontactInterface
    KF5::Libkdepim
    akregatorprivate
    akregatorinterfaces
    KF5::PimCommon
    )

install(TARGETS akregator ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

add_library(akregatorprivate)

target_sources(akregatorprivate PRIVATE
    ${akregator_common_SRCS}
    articleviewer-ng/webengine/articleviewerwebengine.cpp
    articleviewer-ng/webengine/articleviewerwebenginepage.cpp
    articleviewer-ng/webengine/articlehtmlwebenginewriter.cpp
    articleviewer-ng/webengine/articleviewerwebenginewidgetng.cpp

    job/downloadarticlejob.cpp
    job/downloadfeediconjob.cpp

    formatter/articleformatter.cpp
    formatter/defaultnormalviewformatter.cpp
    formatter/defaultcombinedviewformatter.cpp
    formatter/grantleeviewformatter.cpp
    formatter/articlegrantleeobject.cpp
    formatter/grantleeutil.cpp

    frame/webengine/webengineframe.cpp
    frame/webengine/akrwebengineviewer.cpp
    frame/frame.cpp
    frame/mainframe.cpp
    frame/framemanager.cpp

    urlhandler/webengine/urlhandlerwebengine.cpp
    urlhandler/webengine/urlhandlerwebenginemanager.cpp
    akregratormigrateapplication.cpp
    articleviewerwidget.cpp
    aboutdata.cpp
    trayicon.cpp
    unityservicemanager.cpp
    article.cpp
    feed/feed.cpp
    feed/feedlist.cpp
    feed/feedretriever.cpp
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
    akregator.qrc

    articleviewer-ng/webengine/articleviewerwebengine.h
    articleviewer-ng/webengine/articleviewerwebenginepage.h
    articleviewer-ng/webengine/articlehtmlwebenginewriter.h
    articleviewer-ng/webengine/articleviewerwebenginewidgetng.h

    job/downloadarticlejob.h
    job/downloadfeediconjob.h

    formatter/articleformatter.h
    formatter/defaultnormalviewformatter.h
    formatter/defaultcombinedviewformatter.h
    formatter/grantleeviewformatter.h
    formatter/articlegrantleeobject.h
    formatter/grantleeutil.h

    frame/webengine/webengineframe.h
    frame/webengine/akrwebengineviewer.h
    frame/frame.h
    frame/mainframe.h
    frame/framemanager.h

    storage/metakit/src/column.cpp
    storage/metakit/src/custom.cpp
    storage/metakit/src/derived.cpp
    storage/metakit/src/field.cpp
    storage/metakit/src/fileio.cpp
    storage/metakit/src/format.cpp
    storage/metakit/src/handler.cpp
    storage/metakit/src/persist.cpp
    storage/metakit/src/remap.cpp
    storage/metakit/src/std.cpp
    storage/metakit/src/store.cpp
    storage/metakit/src/string.cpp
    storage/metakit/src/table.cpp
    storage/metakit/src/univ.cpp
    storage/metakit/src/view.cpp
    storage/metakit/src/viewx.cpp
    storage/feedstorage.cpp
    storage/storage.cpp


    urlhandler/webengine/urlhandlerwebengine.h
    urlhandler/webengine/urlhandlerwebenginemanager.h
    akregratormigrateapplication.h
    articleviewerwidget.h
    aboutdata.h
    trayicon.h
    unityservicemanager.h
    feed/feed.h
    feed/feedlist.h
    feed/feedretriever.h
    treenode.h
    treenodevisitor.h
    utils.h
    notificationmanager.h
    articlejobs.h
    folder.h
    kernel.h
    subscription/subscriptionlistjobs.h
    fetchqueue.h
    openurlrequest.h
    actions/actionmanager.h
    actions/actions.h

    )
target_include_directories(akregatorprivate PRIVATE storage/metakit/include)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akregatorprivate PROPERTIES UNITY_BUILD ON)
endif()
generate_export_header(akregatorprivate BASE_NAME akregator)

if (TARGET KUserFeedbackWidgets)
    target_link_libraries(akregatorprivate PRIVATE KUserFeedbackWidgets)
endif()


target_link_libraries(akregatorprivate
    PRIVATE
    KF5::I18n
    KF5::Parts
    KF5::Notifications
    KF5::Libkdepim
    KF5::Syndication
    akregatorinterfaces
    KF5::PimCommon
    KF5::PimTextEdit
    KF5::GrantleeTheme
    KF5::KIOGui
    KF5::MessageViewer
    Qt${QT_MAJOR_VERSION}::PrintSupport
    KF5::WebEngineViewer
    ${akregator_userfeedback_LIB}
    )

target_include_directories(akregatorprivate PUBLIC "$<BUILD_INTERFACE:${akregator_SOURCE_DIR}/src;${akregator_BINARY_DIR}/src>")

set_target_properties(akregatorprivate
    PROPERTIES VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
    )
install(TARGETS akregatorprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############
add_library(akregatorpart MODULE)
set(akregator_adaptator_SRCS)
qt_add_dbus_adaptor(akregator_adaptator_SRCS org.kde.akregator.part.xml akregator_part.h Akregator::Part)

target_sources(akregatorpart PRIVATE
    ${akregator_common_SRCS}
    ${akregator_adaptator_SRCS}

    crashwidget/crashwidget.cpp

    utils/filtercolumnsproxymodel.cpp
    utils/textutil.cpp
    utils/textutil.h

    command/deletesubscriptioncommand.cpp
    command/createfeedcommand.cpp
    command/createfoldercommand.cpp
    command/expireitemscommand.cpp
    command/loadfeedlistcommand.cpp
    command/editsubscriptioncommand.cpp
    command/importfeedlistcommand.cpp

    widgets/statussearchline.cpp
    widgets/searchbar.cpp
    widgets/akregatorcentralwidget.cpp

    subscription/subscriptionlistview.cpp
    subscription/subscriptionlistdelegate.cpp
    subscription/subscriptionlistmodel.cpp

    abstractselectioncontroller.cpp
    articlematcher.cpp
    articlemodel.cpp
    selectioncontroller.cpp
    articlelistview.cpp
    actions/actionmanagerimpl.cpp
    addfeeddialog.cpp
    feed/feedpropertiesdialog.cpp
    tabwidget.cpp
    progressmanager.cpp
    akregator_part.cpp
    mainwidget.cpp

    crashwidget/crashwidget.h

    utils/filtercolumnsproxymodel.h

    command/deletesubscriptioncommand.h
    command/createfeedcommand.h
    command/createfoldercommand.h
    command/expireitemscommand.h
    command/loadfeedlistcommand.h
    command/editsubscriptioncommand.h
    command/importfeedlistcommand.h

    widgets/statussearchline.h
    widgets/searchbar.h
    widgets/akregatorcentralwidget.h

    subscription/subscriptionlistview.h
    subscription/subscriptionlistdelegate.h
    subscription/subscriptionlistmodel.h

    abstractselectioncontroller.h
    articlematcher.h
    articlemodel.h
    selectioncontroller.h
    articlelistview.h
    actions/actionmanagerimpl.h
    addfeeddialog.h
    feed/feedpropertiesdialog.h
    tabwidget.h
    progressmanager.h
    akregator_part.h
    mainwidget.h

    )

ki18n_wrap_ui(akregatorpart
    ui/addfeedwidgetbase.ui
    ui/feedpropertieswidgetbase.ui
    )

generate_export_header(akregatorpart BASE_NAME akregatorpart)
target_link_libraries(akregatorpart
    akregatorinterfaces
    akregatorprivate
    KF5::I18n
    KF5::KCMUtils
    KF5::NotifyConfig
    KF5::Libkdepim
    KF5::Syndication
    KF5::PimCommon
    KF5::PimTextEdit
    KF5::GrantleeTheme
    KF5::MessageViewer
    KF5::Parts
    KF5::Notifications
    KF5::TextWidgets
    Qt${QT_MAJOR_VERSION}::WebEngineWidgets
    KF5::WebEngineViewer
    KF5::KIOGui
    )
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akregatorpart PROPERTIES UNITY_BUILD ON)
endif()

install(TARGETS akregatorpart DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(PROGRAMS data/org.kde.akregator.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/org.kde.akregator.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.akregator.part.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
install(FILES data/akregator.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR} )

add_subdirectory(formatter/html)
if(BUILD_TESTING)
   add_subdirectory(job/autotests)
endif()
