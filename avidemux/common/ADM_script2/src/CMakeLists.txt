SET(ADM_script_SRCS
	ADM_script.cpp
)

IF (WIN32)
	ADD_DEFINITIONS(-DXP_WIN)
ELSE (WIN32)
	ADD_DEFINITIONS(-DXP_UNIX)
ENDIF (WIN32)

ADD_DEFINITIONS(-DJS_THREADSAFE)

ADD_LIBRARY(ADM_script6 STATIC ${ADM_script_SRCS})
include_directories(../include)
include_directories(../../ADM_editor)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreScript/include)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreScriptPython/include)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreScriptQt/include)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreScriptSpiderMonkey/include)

find_library(QT_SCRIPT_PLUGIN ADM_coreScriptQt PATHS "${AVIDEMUX_SEARCH_LIB_DIR}")
find_library(SPIDERMONKEY_SCRIPT_PLUGIN ADM_coreScriptSpiderMonkey PATHS "${AVIDEMUX_SEARCH_LIB_DIR}")
find_library(TINYPY_SCRIPT_PLUGIN ADM_coreScriptPython PATHS "${AVIDEMUX_SEARCH_LIB_DIR}")

if (QT_SCRIPT_PLUGIN)
	target_link_libraries(ADM_script6 ${QT_SCRIPT_PLUGIN})
endif (QT_SCRIPT_PLUGIN)

if (SPIDERMONKEY_SCRIPT_PLUGIN)
	target_link_libraries(ADM_script6 ${SPIDERMONKEY_SCRIPT_PLUGIN})
endif (SPIDERMONKEY_SCRIPT_PLUGIN)

if (TINYPY_SCRIPT_PLUGIN)
	target_link_libraries(ADM_script6 ${TINYPY_SCRIPT_PLUGIN})
endif (TINYPY_SCRIPT_PLUGIN)
