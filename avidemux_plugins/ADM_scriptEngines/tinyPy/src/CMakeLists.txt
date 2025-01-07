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

ADD_DEFINITIONS(-DCPYTHON_MOD)
FILE(STRINGS "version.txt" PY_VERSION)
ADD_DEFINITIONS(-DPY_VERSION_NUM=${PY_VERSION})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(modules/re)
ADD_CORE_INCLUDE(ADM_coreScript)
# This is the problematic part
SET(AVIDEMUX_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)
INCLUDE_DIRECTORIES(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common)
INCLUDE_DIRECTORIES(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_editor/include)

TARGET_LINK_LIBRARIES(ADM_script_tinyPy PRIVATE ADM_coreImage6 ADM_coreScript ADM_coreUtils6 ADM_core6 ADM_coreUI6 ADM_coreDemuxer6 adm_pthread)
IF(NOT MSVC)
  TARGET_LINK_LIBRARIES(ADM_script_tinyPy PRIVATE m)
ENDIF()

IF(NOT APPLE)
  TARGET_LINK_LIBRARIES(ADM_script_tinyPy PRIVATE -Wl,--version-script,"${CMAKE_CURRENT_SOURCE_DIR}/ADM_script_tinyPy.v")
ELSEIF()
  TARGET_LINK_LIBRARIES(ADM_script_tinyPy PRIVATE -Wl,-read_only_relocs,suppress)
ENDIF()
