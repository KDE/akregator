# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.23.40")
project(akregator VERSION ${PIM_VERSION})

set(KF_MIN_VERSION "5.105.0")
set(RELEASE_SERVICE_VERSION "23.07.40")

find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMInstallIcons)


include(ECMSetupVersion)
include(ECMAddTests)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
if (QT_MAJOR_VERSION STREQUAL "6")
    set(KTEXTADDONS_MIN_VERSION "1.1.0")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
    set(KTEXTADDONS_MIN_VERSION "1.0.0")
endif()

# Do NOT add quote
set(KDEPIM_DEV_VERSION alpha)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "${PIM_VERSION}${KDEPIM_DEV_VERSION} (${RELEASE_SERVICE_VERSION})")
set(KDEPIM_LIB_VERSION "${PIM_VERSION}")
set(KDEPIM_LIB_SOVERSION "5")

set(QT_REQUIRED_VERSION "5.15.2")
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
endif()
set(KONTACTINTERFACE_LIB_VERSION "5.23.40")
set(KPIMTEXTEDIT_LIB_VERSION "5.23.40")
set(LIBGRANTLEETHEME_LIB_VERSION "5.23.40")
set(LIBKDEPIM_LIB_VERSION "5.23.40")
set(MESSAGELIB_LIB_VERSION "5.23.40")
set(PIMCOMMON_LIB_VERSION "5.23.40")

find_package(Qt${QT_MAJOR_VERSION} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test WebEngineWidgets PrintSupport)
# Find KF5 package
find_package(KF${KF_MAJOR_VERSION}I18n ${KF_MIN_VERSION} REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Crash ${KF_MIN_VERSION} REQUIRED)
find_package(KF${KF_MAJOR_VERSION}KCMUtils ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}NotifyConfig ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Parts ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}TextWidgets ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}XmlGui ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Notifications ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Syndication ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}DocTools ${KF_MIN_VERSION})
set_package_properties(KF${KF_MAJOR_VERSION}DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)


# Find KdepimLibs Package
find_package(KPim${KF_MAJOR_VERSION}GrantleeTheme ${LIBGRANTLEETHEME_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}Libkdepim ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}MessageViewer ${MESSAGELIB_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}TextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}TextEditTextToSpeech ${KTEXTADDONS_MIN_VERSION} CONFIG)
if (TARGET KF${KF_MAJOR_VERSION}::TextEditTextToSpeech)
    add_definitions(-DHAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT)
endif()
find_package(KPim${KF_MAJOR_VERSION}WebEngineViewer ${MESSAGELIB_LIB_VERSION} CONFIG REQUIRED)
# Fix plugin support for removing theses dependencies
find_package(KPim${KF_MAJOR_VERSION}AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}PimCommonAkonadi ${PIMCOMMON_LIB_VERSION} CONFIG REQUIRED)


set(KUSERFEEDBACK_SUFFIX)
if (QT_MAJOR_VERSION STREQUAL "6")
    set(KUSERFEEDBACK_SUFFIX "Qt6")
    find_package(KUserFeedback${KUSERFEEDBACK_SUFFIX} 1.2.1 CONFIG)
    add_definitions(-DUSE_KUSERFEEDBACK_QT6)
else()
    find_package(KUserFeedback${KUSERFEEDBACK_SUFFIX} 1.2.0 CONFIG)
endif()
set_package_properties(KUserFeedback${KUSERFEEDBACK_SUFFIX} PROPERTIES DESCRIPTION "User Feedback lib" TYPE OPTIONAL PURPOSE "Allow to send Telemetry Information (optional). It can be disable in apps.")


include_directories(${akregator_SOURCE_DIR} ${akregator_BINARY_DIR})
add_definitions(-DTRANSLATION_DOMAIN=\"akregator\")
ecm_set_disabled_deprecation_versions(QT 5.15.2
     KF 5.105.0
)
if (TARGET KUserFeedbackWidgets${KUSERFEEDBACK_SUFFIX})
    add_definitions(-DWITH_KUSERFEEDBACK)
endif()

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()
add_subdirectory(export)
add_subdirectory(interfaces)
add_subdirectory(configuration)
add_subdirectory(src)
add_subdirectory(kontactplugin)

ecm_qt_install_logging_categories(
        EXPORT AKREGATOR
        FILE akregator.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
if(KF${KF_MAJOR_VERSION}DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
