project (ktp-filetransfer-handler)

cmake_minimum_required(VERSION 2.8.12)

# KDE Application Version, managed by release script
set(KDE_APPLICATIONS_VERSION_MAJOR "16")
set(KDE_APPLICATIONS_VERSION_MINOR "04")
set(KDE_APPLICATIONS_VERSION_MICRO "3")
set(KTP_FILETRANSFER_HANDLER_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH})

find_package (KF5 REQUIRED COMPONENTS CoreAddons I18n KIO Config)
find_package (Qt5 REQUIRED COMPONENTS Core DBus Widgets)
find_package (KTp REQUIRED)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMSetupVersion)
include(FeatureSummary)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
