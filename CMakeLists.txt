cmake_minimum_required(VERSION 3.5)
set(PIM_VERSION "5.15.40")
project(akregator VERSION ${PIM_VERSION})

set(KF5_MIN_VERSION "5.72.0")
set(RELEASE_SERVICE_VERSION "20.11.80")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMInstallIcons)


include(ECMSetupVersion)
include(ECMAddTests)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(FeatureSummary)
include(CheckFunctionExists)
include(ECMGeneratePriFile)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)


# Do NOT add quote
set(KDEPIM_DEV_VERSION alpha)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "${PIM_VERSION}${KDEPIM_DEV_VERSION} (${RELEASE_SERVICE_VERSION})")
set(KDEPIM_LIB_VERSION "${PIM_VERSION}")
set(KDEPIM_LIB_SOVERSION "5")

set(QT_REQUIRED_VERSION "5.13.0")

set(KONTACTINTERFACE_LIB_VERSION "5.15.40")
set(KPIMTEXTEDIT_LIB_VERSION "5.15.40")
set(LIBGRANTLEETHEME_LIB_VERSION_LIB "5.15.40")
set(LIBKDEPIM_LIB_VERSION_LIB "5.15.40")
set(LIBKLEO_LIB_VERSION_LIB "5.15.40")
set(MESSAGELIB_LIB_VERSION_LIB "5.15.40")
set(PIMCOMMON_LIB_VERSION_LIB "5.15.40")

option(KDEPIM_KUSERFEEDBACK "Enable user feedback support (experimental)" FALSE)


find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test WebEngine WebEngineWidgets PrintSupport)
find_package(Grantlee5 "5.2" CONFIG REQUIRED)

# Find KF5 package
find_package(KF5Crash ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5DocTools ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5KCMUtils ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5NotifyConfig ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Parts ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5TextEditor ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Notifications ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Syndication ${KF5_MIN_VERSION} CONFIG REQUIRED)

# Find KdepimLibs Package
find_package(KF5GrantleeTheme ${LIBGRANTLEETHEME_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5Libkleo ${LIBKLEO_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5MessageViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5WebEngineViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)

# Fix plugin support for removing theses dependancies
find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
find_package(KF5PimCommonAkonadi ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)

find_package(KUserFeedback 0.9.90 CONFIG) # Needs Provider::describeDataSources()
set_package_properties(KUserFeedback PROPERTIES DESCRIPTION "User Feedback lib" TYPE OPTIONAL PURPOSE "Allow to send Telemetry Information (optional). It can be disable in apps.")


include_directories(${akregator_SOURCE_DIR} ${akregator_BINARY_DIR})
add_definitions(-DTRANSLATION_DOMAIN=\"akregator\")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050e00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054800)
if (TARGET KUserFeedbackWidgets)
    add_definitions(-DWITH_KUSERFEEDBACK)
endif()
add_subdirectory(export)
add_subdirectory(interfaces)
add_subdirectory(plugins)
add_subdirectory(configuration)
add_subdirectory(src)
add_subdirectory(kontactplugin)
add_subdirectory(kconf_update)

ecm_qt_install_logging_categories(
        EXPORT AKREGATOR
        FILE akregator.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )
add_subdirectory(doc)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
