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
	SpiderMonkeyScriptWriter.cpp
	ADM_script_spiderMonkey.v
)

IF (WIN32)
	ADD_DEFINITIONS(-DXP_WIN)
ELSE (WIN32)
	ADD_DEFINITIONS(-DXP_UNIX)
ENDIF (WIN32)

ADD_DEFINITIONS(-DJS_THREADSAFE)
add_subdirectory(src_dialogFactory)

ADD_SCRIPT_ENGINE(ADM_script_spiderMonkey ${admScriptSpiderMonkey_SRCS})
INSTALL_SCRIPT_ENGINE(ADM_script_spiderMonkey)

include_directories(../include)
include_directories(../../ADM_coreScript/include)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common)

if (USE_SYSTEM_SPIDERMONKEY)
	include_directories(${SPIDERMONKEY_INCLUDE_DIR})
	target_link_libraries(ADM_script_spiderMonkey ${SPIDERMONKEY_LIBRARY_DIR})
else (USE_SYSTEM_SPIDERMONKEY)
	include_directories(./smjs)
	target_link_libraries(ADM_script_spiderMonkey ADM_smjs6)
endif (USE_SYSTEM_SPIDERMONKEY)

target_link_libraries(ADM_script_spiderMonkey ADM_coreUI6 ADM_coreUtils6 ADM_scriptDF ADM_coreScript ADM_core6)

if (NOT APPLE)
    target_link_libraries(ADM_script_spiderMonkey -Wl,--version-script,"${CMAKE_CURRENT_SOURCE_DIR}/ADM_script_spiderMonkey.v")
endif (NOT APPLE)
