SET(admScriptPython_SRCS
	tinypy/tinypy.cpp
	ADM_pyAvidemux.cpp
	PythonEngine.cpp
	TinyParams.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreScriptPython ${admScriptPython_SRCS})
ADM_INSTALL_LIB(ADM_coreScriptPython)

add_definitions(-DCPYTHON_MOD)

include_directories(.)
include_directories(../include)
include_directories(../../ADM_coreScript/include)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_editor)

if (APPLE)
	target_link_libraries(ADM_coreScriptPython -Wl,-read_only_relocs,suppress)
endif (APPLE)

target_link_libraries(ADM_coreScriptPython ADM_coreScript ADM_coreUtils6 -Wl,--version-script,"${CMAKE_CURRENT_SOURCE_DIR}/ADM_coreScriptPython.v")
