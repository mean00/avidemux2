include(admAsNeeded)
include(admPluginLocation)
MACRO(INIT_AUDIO_DEVICE _lib)
ENDMACRO()

MACRO(INSTALL_AUDIO_DEVICE _lib)
	INSTALL(TARGETS ${_lib} 
                DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/audioDevices/"
                COMPONENT plugins
                )
ENDMACRO()


MACRO(ADD_AUDIO_DEVICE name)
        ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
        IF(NOT MSVC) 
                SET(EXTRALIB "m")
        ENDIF() 
        TARGET_LINK_LIBRARIES( ${name} PRIVATE ADM_coreAudioDevice6 ADM_core6 ${EXTRALIB} adm_pthread)
ENDMACRO()

