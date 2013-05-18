project (ktp-filetransfer-handler)

set(KTP_FILETRANSFER_HANDLER_VERSION "0.6.2")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
cmake_policy(SET CMP0002 OLD)

set(IS_KTP_INTERNAL_MODULE TRUE)
set(KDE_MIN_VERSION "4.6.0")

find_package(KDE4 ${KDE_MIN_VERSION} REQUIRED)
find_package(TelepathyQt4 0.9.1 REQUIRED)
find_package(KTp REQUIRED)

include(CheckIncludeFiles)
include(KDE4Defaults)
include(MacroLibrary)

add_definitions(${KDE4_DEFINITIONS}
                -DQT_NO_CAST_FROM_ASCII
                -DQT_NO_KEYWORDS
)

include_directories(${KDE4_INCLUDES}
                    ${TELEPATHY_QT4_INCLUDE_DIR}
                    ${KTP_INCLUDE_DIR}
)

add_subdirectory(src)
