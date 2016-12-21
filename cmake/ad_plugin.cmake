include(admAsNeeded)
MACRO(INIT_AUDIO_PLUGIN _lib)
	INCLUDE_DIRECTORIES("${ADM_HEADER_DIR}/ADM_audioCodec")
ENDMACRO(INIT_AUDIO_PLUGIN)

MACRO(INSTALL_AUDIODECODER _lib)
        IF(NOT MSVC) 
                SET(EXTRALIB "m")
        ENDIF(NOT MSVC) 

	TARGET_LINK_LIBRARIES(${_lib} ADM_core6 ${EXTRALIB})
	INSTALL(TARGETS ${_lib} 
                        DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/audioDecoder/"
                        COMPONENT   plugins
                        )
ENDMACRO(INSTALL_AUDIODECODER)
############## ADD_VIDEO_FILTER ###################"
MACRO(ADD_AUDIO_DECODER name)
        ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
ENDMACRO(ADD_AUDIO_DECODER name)

