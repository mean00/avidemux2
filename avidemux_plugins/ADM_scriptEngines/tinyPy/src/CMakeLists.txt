SET(admScriptPython_SRCS
	tinypy/tinypy.cpp
	ADM_pyAvidemux.cpp
	ADM_pyAvidemuxAudio.cpp
	PythonEngine.cpp
	PythonScriptWriter.cpp
	TinyParams.cpp
	ADM_script_tinyPy.v
        ADM_pyTests.cpp
        modules/re/init.cpp
)

ADD_SCRIPT_ENGINE(ADM_script_tinyPy ${admScriptPython_SRCS})
INSTALL_SCRIPT_ENGINE(ADM_script_tinyPy)

add_definitions(-DCPYTHON_MOD)

include_directories(.)
include_directories(../include)
include_directories(modules/re)
include_directories(../../ADM_coreScript/include)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_editor/include)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreSubtitles/include)

target_link_libraries(ADM_script_tinyPy ADM_coreScript ADM_coreUtils6 ADM_core6 ADM_coreUI6 ADM_coreSubtitle m)

if (NOT APPLE)
	target_link_libraries(ADM_script_tinyPy -Wl,--version-script,"${CMAKE_CURRENT_SOURCE_DIR}/ADM_script_tinyPy.v")
elseif (NOT ADM_CPU_X86_64)
	target_link_libraries(ADM_script_tinyPy -Wl,-read_only_relocs,suppress)
endif (NOT APPLE)
