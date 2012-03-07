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
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreScriptSpiderMonkey/include)

if (USE_SYSTEM_SPIDERMONKEY)
	include_directories(${SPIDERMONKEY_INCLUDE_DIR})
else (USE_SYSTEM_SPIDERMONKEY)
	include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreScriptSpiderMonkey/src/smjs)
endif (USE_SYSTEM_SPIDERMONKEY)

target_link_libraries(ADM_script6 ADM_coreScriptPython ADM_coreScriptSpiderMonkey)
