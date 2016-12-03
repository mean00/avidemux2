include(admAsNeeded)
MACRO(INIT_AUDIO_DEVICE _lib)
ENDMACRO(INIT_AUDIO_DEVICE)

MACRO(INSTALL_AUDIO_DEVICE _lib)
	INSTALL(TARGETS ${_lib} 
                DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/audioDevices/"
                COMPONENT plugins
                )
ENDMACRO(INSTALL_AUDIO_DEVICE)


MACRO(ADD_AUDIO_DEVICE name)
        ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
        IF(NOT MSVC) 
                SET(EXTRALIB "m")
        ENDIF(NOT MSVC) 
        TARGET_LINK_LIBRARIES( ${name} ADM_coreAudioDevice6 ADM_core6 ${EXTRALIB})
ENDMACRO(ADD_AUDIO_DEVICE name)

