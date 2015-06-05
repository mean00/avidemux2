include(admAsNeeded)

macro (ADD_SCRIPT_ENGINE name)
    ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
endmacro (ADD_SCRIPT_ENGINE name)

macro (INSTALL_SCRIPT_ENGINE _lib)
	install(TARGETS ${_lib} DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/scriptEngines/" COMPONENT plugins)
endmacro (INSTALL_SCRIPT_ENGINE)
macro (INSTALL_SCRIPT_ENGINE_QT4 _lib)
	install(TARGETS ${_lib} DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/scriptEngines/${QT_EXTENSION}" COMPONENT plugins)
endmacro (INSTALL_SCRIPT_ENGINE_QT4)

macro (INSTALL_SCRIPT_ENGINE_HELP _engineName _sourceDirectory)
if (WIN32)
    set(helpDir "${CMAKE_INSTALL_PREFIX}/help/${_engineName}")
else (WIN32)
    set(helpDir "${CMAKE_INSTALL_PREFIX}/share/avidemux6/help/${_engineName}")
endif (WIN32)

    install(DIRECTORY "${_sourceDirectory}" DESTINATION "${helpDir}" COMPONENT plugins)
endmacro (INSTALL_SCRIPT_ENGINE_HELP)
