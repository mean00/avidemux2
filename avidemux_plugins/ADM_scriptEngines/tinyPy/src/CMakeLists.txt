IF(MSVC)
     SET(PYTOOLS ADM_pyTools_win32.cpp)
ELSE()
     SET(PYTOOLS ADM_pyTools.cpp)
ENDIF()             
SET(admScriptPython_SRCS
	tinypy/tinypy.cpp
	ADM_pyAvidemux.cpp
	ADM_pyAvidemuxAudio.cpp
	PythonEngine.cpp
	PythonScriptWriter.cpp
	TinyParams.cpp
	ADM_script_tinyPy.v
    ADM_pyTests.cpp
    ${PYTOOLS}
    modules/re/init.cpp
)

ADD_SCRIPT_ENGINE(ADM_script_tinyPy ${admScriptPython_SRCS})
INSTALL_SCRIPT_ENGINE(ADM_script_tinyPy)

add_definitions(-DCPYTHON_MOD)
file (STRINGS "version.txt" PY_VERSION)
ADD_DEFINITIONS(-DPY_VERSION_NUM=${PY_VERSION})

include_directories(.)
include_directories(../include)
include_directories(modules/re)
ADD_CORE_INCLUDE(ADM_coreScript)
# This is the problematic part
SET(AVIDEMUX_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_editor/include)

target_link_libraries(ADM_script_tinyPy ADM_coreImage6 ADM_coreScript ADM_coreUtils6 ADM_core6 ADM_coreUI6 ADM_coreDemuxer6)
IF(NOT MSVC)
        target_link_libraries(ADM_script_tinyPy m )
ENDIF(NOT MSVC)

if (NOT APPLE)
	target_link_libraries(ADM_script_tinyPy -Wl,--version-script,"${CMAKE_CURRENT_SOURCE_DIR}/ADM_script_tinyPy.v")
elseif (NOT ADM_CPU_X86_64)
	target_link_libraries(ADM_script_tinyPy -Wl,-read_only_relocs,suppress)
endif (NOT APPLE)
