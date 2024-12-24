include(admAsNeeded)
include(admPluginLocation)

macro(ADD_SCRIPT_ENGINE name)
  ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
ENDMACRO()

MACRO(INSTALL_SCRIPT_ENGINE _lib)
  install(TARGETS ${_lib} DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/scriptEngines/" COMPONENT plugins)
ENDMACRO()
macro(INSTALL_SCRIPT_ENGINE_QT4 _lib)
  install(TARGETS ${_lib} DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/scriptEngines/${QT_EXTENSION}" COMPONENT plugins)
ENDMACRO()

macro(INSTALL_SCRIPT_ENGINE_HELP _engineName _sourceDirectory)
  IF(WIN32)
    SET(helpDir "${CMAKE_INSTALL_PREFIX}/help/${_engineName}")
  ELSE()
    SET(helpDir "${CMAKE_INSTALL_PREFIX}/share/avidemux6/help/${_engineName}")
  ENDIF()

  install(DIRECTORY "${_sourceDirectory}" DESTINATION "${helpDir}" COMPONENT plugins)
ENDMACRO()
