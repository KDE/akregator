project(akregator)

if(${CMAKE_SOURCE_DIR} STREQUAL ${akregator_SOURCE_DIR})
    cmake_minimum_required(VERSION 2.8.12)
    find_package(ECM 5.20.0 REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${akregator_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
    include(ECMInstallIcons)

    include(ECMOptionalAddSubdirectory)
    include(ECMSetupVersion)
    include(ECMAddTests)
    include(ECMMarkNonGuiExecutable)
    include(GenerateExportHeader)
    include(ECMGenerateHeaders)
    include(CMakePackageConfigHelpers)
    include(FeatureSummary)
    include(CheckFunctionExists)
    include(ECMGeneratePriFile)

    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings NO_POLICY_SCOPE)
    include(ECMAddAppIcon)
    include(ECMQtDeclareLoggingCategory)


    set(KF5_VERSION "5.20.0")

    set(KDEPIM_VERSION_NUMBER "5.2.42")
    set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")


    set(KDEPIMLIBS_LIB_VERSION "5.2.40")
    set(KPIMTEXTEDIT_LIB_VERSION "5.2.40")
    set(KBLOG_LIB_VERSION "5.2.40")
    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")
    set(KDEPIM_APPS_LIB_VERSION_LIB "5.2.42")

    set(QT_REQUIRED_VERSION "5.6.0")
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test Sql)
    find_package(Qt5X11Extras NO_MODULE)
    set(MAILCOMMON_LIB_VERSION_LIB "5.2.40")
    set(PIMCOMMON_LIB_VERSION_LIB "5.2.40")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.2.40")
    # Find KF5 package
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5TextEditor ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5PimCommon ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)


    # Find KdepimLibs Package
    find_package(KF5Blog ${KBLOG_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)

    option(QTWEBENGINE_SUPPORT_OPTION "Enable support for QtWebEngine. False by default." FALSE)
    if (QTWEBENGINE_SUPPORT_OPTION)
        find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED WebEngine WebEngineWidgets)
        find_package(KF5WebEngineViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
        set(WEBENGINE_TYPE "QtWebEngine")
    else()
        find_package(KF5ComposerEditorNG ${KDEPIM_APPS_LIB_VERSION_LIB} CONFIG REQUIRED)
        find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED WebKitWidgets)
        find_package(KF5WebKit ${KF5_VERSION} CONFIG REQUIRED)
        set(WEBENGINE_TYPE "QtWebKit")
    endif()


    include_directories(${akregator_SOURCE_DIR} ${akregator_BINARY_DIR})
endif()

include(CheckTypeSize)
check_type_size("long" SIZEOF_LONG)
configure_file(config-akregator.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-akregator.h)
add_definitions(-DTRANSLATION_DOMAIN=\"akregator\")


add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_subdirectory(export)
add_subdirectory(interfaces)
add_subdirectory(plugins)
add_subdirectory(configuration)
add_subdirectory(src)
add_subdirectory(kontactplugin)
add_subdirectory(kconf_update)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
