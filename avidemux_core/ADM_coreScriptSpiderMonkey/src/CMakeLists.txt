include(admCheckSpiderMonkey)

if (NOT USE_SYSTEM_SPIDERMONKEY)
	ADD_SUBDIRECTORY(smjs)
endif (NOT USE_SYSTEM_SPIDERMONKEY)

SET(admScriptSpiderMonkey_SRCS
	ADM_jsAvidemux_js.c
	ADM_jsAvidemux.cpp
	ADM_jsEditor_js.c
	ADM_jsEditor.cpp
	ADM_jsIf_js.c
	ADM_jsIf.cpp
	ADM_jsTestFactory_js.c
	ADM_jsTestFactory.cpp
	ADM_jsUtils.cpp
	SpiderMonkeyEngine.cpp
	ADM_coreScriptSpiderMonkey.v
)

IF (WIN32)
	ADD_DEFINITIONS(-DXP_WIN)
ELSE (WIN32)
	ADD_DEFINITIONS(-DXP_UNIX)
ENDIF (WIN32)

ADD_DEFINITIONS(-DJS_THREADSAFE)
add_subdirectory(src_dialogFactory)

ADM_ADD_SHARED_LIBRARY(ADM_coreScriptSpiderMonkey ${admScriptSpiderMonkey_SRCS})
ADM_INSTALL_LIB(ADM_coreScriptSpiderMonkey)

include_directories(../include)
include_directories(../../ADM_coreScript/include)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common)

if (USE_SYSTEM_SPIDERMONKEY)
	include_directories(${SPIDERMONKEY_INCLUDE_DIR})
	target_link_libraries(ADM_coreScriptSpiderMonkey ${SPIDERMONKEY_LIBRARY_DIR})
else (USE_SYSTEM_SPIDERMONKEY)
	include_directories(./smjs)
	target_link_libraries(ADM_coreScriptSpiderMonkey ADM_smjs6)
endif (USE_SYSTEM_SPIDERMONKEY)

target_link_libraries(ADM_coreScriptSpiderMonkey ADM_coreUI6 ADM_coreUtils6 ADM_scriptDF ADM_coreScript -Wl,--version-script,"${CMAKE_CURRENT_SOURCE_DIR}/ADM_coreScriptSpiderMonkey.v")
