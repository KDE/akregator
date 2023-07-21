# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.240.40")
project(akregator VERSION ${PIM_VERSION})

set(KF_MIN_VERSION "5.240.0")
set(RELEASE_SERVICE_VERSION "23.07.40")
set(QT_REQUIRED_VERSION "6.5.0")
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
set(KTEXTADDONS_MIN_VERSION "1.3.41")

# Do NOT add quote
set(KDEPIM_DEV_VERSION alpha)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "${PIM_VERSION}${KDEPIM_DEV_VERSION} (${RELEASE_SERVICE_VERSION})")
set(KDEPIM_LIB_VERSION "${PIM_VERSION}")
set(KDEPIM_LIB_SOVERSION "6")

set(KONTACTINTERFACE_LIB_VERSION "5.240.40")
set(KPIMTEXTEDIT_LIB_VERSION "5.240.40")
set(LIBGRANTLEETHEME_LIB_VERSION "5.240.40")
set(LIBKDEPIM_LIB_VERSION "5.240.40")
set(MESSAGELIB_LIB_VERSION "5.240.41")
set(PIMCOMMON_LIB_VERSION "5.240.40")

find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test WebEngineWidgets PrintSupport)
# Find KF6 package
find_package(KF6I18n ${KF_MIN_VERSION} REQUIRED)
find_package(KF6Crash ${KF_MIN_VERSION} REQUIRED)
find_package(KF6KCMUtils ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6NotifyConfig ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Parts ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6TextWidgets ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6XmlGui ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Notifications ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Syndication ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6DocTools ${KF_MIN_VERSION})
set_package_properties(KF6DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)


# Find KdepimLibs Package
find_package(KPim6GrantleeTheme ${LIBGRANTLEETHEME_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6Libkdepim ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6MessageViewer ${MESSAGELIB_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6TextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF6TextEditTextToSpeech ${KTEXTADDONS_MIN_VERSION} CONFIG)
if (TARGET KF6::TextEditTextToSpeech)
    add_definitions(-DHAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT)
endif()
find_package(KPim6WebEngineViewer ${MESSAGELIB_LIB_VERSION} CONFIG REQUIRED)
# Fix plugin support for removing theses dependencies
find_package(KPim6AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
find_package(KPim6PimCommonAkonadi ${PIMCOMMON_LIB_VERSION} CONFIG REQUIRED)


find_package(KUserFeedbackQt6 1.2.1 CONFIG)
set_package_properties(KUserFeedbackQt6 PROPERTIES DESCRIPTION "User Feedback lib" TYPE OPTIONAL PURPOSE "Allow to send Telemetry Information (optional). It can be disable in apps.")


include_directories(${akregator_SOURCE_DIR} ${akregator_BINARY_DIR})
add_definitions(-DTRANSLATION_DOMAIN=\"akregator\")
ecm_set_disabled_deprecation_versions(QT 5.15.2
     KF 5.240.0
)
if (TARGET KUserFeedbackWidgetsQt6)
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
if(KF6DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
