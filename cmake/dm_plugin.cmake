MACRO(INIT_DEMUXER _lib)
ENDMACRO(INIT_DEMUXER)

MACRO(INSTALL_DEMUXER _lib)
        TARGET_LINK_LIBRARIES(${_lib} ADM_core6 ADM_coreUtils6 ADM_coreAudio6 ADM_coreImage6 ADM_coreUI6 ADM_core6 ADM_coreDemuxer6 m)
	INSTALL(TARGETS ${_lib} 
                DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/demuxers/"
                COMPONENT plugins
                )
ENDMACRO(INSTALL_DEMUXER)

MACRO(ADD_DEMUXER name)
        ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
ENDMACRO(ADD_DEMUXER name)

