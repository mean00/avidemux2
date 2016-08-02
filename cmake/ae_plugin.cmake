#
#  Macro to declare an audio encoder plugin
#
include(admAsNeeded)
MACRO(INIT_AUDIO_ENCODER _lib)
    ADD_DEFINITIONS("-DADM_MINIMAL_UI_INTERFACE")
ENDMACRO(INIT_AUDIO_ENCODER)
MACRO(ADD_AUDIO_ENCODER name)
        ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
ENDMACRO(ADD_AUDIO_ENCODER name)

MACRO(INSTALL_AUDIOENCODER _lib)
	INSTALL(TARGETS ${_lib}                         
                                DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/audioEncoders/"
                                COMPONENT   plugins
                        )
	TARGET_LINK_LIBRARIES(${_lib} ADM_core6 ADM_coreUI6 ADM_coreAudio6 ADM_coreAudioEncoder6 ADM_coreUtils6 m)
ENDMACRO(INSTALL_AUDIOENCODER)
