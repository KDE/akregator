project(akregator)

cmake_minimum_required(VERSION 3.1)
set(KF5_VERSION "5.35.0")
find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
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
include(ECMCoverageOption)
# Do NOT add quote
set(KDEPIM_DEV_VERSION beta1)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
    set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()


set(KDEPIM_VERSION_NUMBER "5.5.80")
set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")
set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
set(KDEPIM_LIB_SOVERSION "5")

set(QT_REQUIRED_VERSION "5.7.0")

set(KONTACTINTERFACE_LIB_VERSION "5.5.80")
set(KPIMTEXTEDIT_LIB_VERSION "5.5.80")
set(LIBGRANTLEETHEME_LIB_VERSION_LIB "5.5.80")
set(LIBKDEPIM_LIB_VERSION_LIB "5.5.80")
set(LIBKLEO_LIB_VERSION_LIB "5.5.80")
set(MESSAGELIB_LIB_VERSION_LIB "5.5.80")
set(PIMCOMMON_LIB_VERSION_LIB "5.5.80")
set(SYNDICATION_LIB_VERSION "5.5.80")

find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test WebEngine WebEngineWidgets PrintSupport)
find_package(Grantlee5 "5.1" CONFIG REQUIRED)

# Find KF5 package
find_package(KF5Crash ${KF5_VERSION} REQUIRED)
find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NotifyConfig ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Parts ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5TextEditor ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Notifications ${KF5_VERSION} CONFIG REQUIRED)

# Find KdepimLibs Package
find_package(KF5GrantleeTheme ${LIBGRANTLEETHEME_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5Libkleo ${LIBKLEO_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5MessageViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Syndication ${SYNDICATION_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5WebEngineViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)

# Fix plugin support for removing theses dependancies
find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
find_package(KF5PimCommonAkonadi ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
include_directories(${akregator_SOURCE_DIR} ${akregator_BINARY_DIR})
include(CheckTypeSize)
check_type_size("long" SIZEOF_LONG)
if (Qt5WebEngineWidgets_VERSION VERSION_GREATER "5.7.99")
    set(WEBENGINEVIEWER_PRINT_SUPPORT true)
endif()

configure_file(config-akregator.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-akregator.h)
add_definitions(-DTRANSLATION_DOMAIN=\"akregator\")


add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)


add_subdirectory(export)
add_subdirectory(interfaces)
add_subdirectory(plugins)
add_subdirectory(configuration)
add_subdirectory(src)
add_subdirectory(kontactplugin)
add_subdirectory(kconf_update)

install( FILES akregator.renamecategories akregator.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

add_subdirectory(doc)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (${Qt5WebEngine_VERSION} GREATER "5.7.99")
    MESSAGE(STATUS "QtWebEngine greater or equal 5.8.0 provides printing support and dnd fix. It's a good idea to use it")
else()
    if (${Qt5WebEngine_VERSION} GREATER "5.6.99")
        MESSAGE(STATUS "QtWebEngine 5.7.0 doesn't provide printing support. It has a lot of regression. Better to upgrade QtWebEngine to 5.8.0")
    endif()
endif()
