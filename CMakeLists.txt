project (telepathy-kde-filetransfer-handler)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

set(KDE_MIN_VERSION "4.6.0")
set(TELEPATHY_QT4_MIN_VERSION "0.7.3")

find_package (KDE4 ${KDE_MIN_VERSION} REQUIRED)
find_package (TelepathyQt4 ${TELEPATHY_QT4_MIN_VERSION} REQUIRED)

include (CheckIncludeFiles)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${KDE4_DEFINITIONS}
                 -DQT_NO_CAST_FROM_ASCII
                 -DQT_NO_KEYWORDS
)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.gitmodules)
execute_process(COMMAND git submodule init
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

execute_process(COMMAND git submodule update
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.gitmodules)


add_subdirectory(src)
