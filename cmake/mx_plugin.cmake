include(admAsNeeded)
MACRO(INIT_MUXER _lib)
ENDMACRO(INIT_MUXER)

MACRO(INSTALL_MUXER _lib)
        IF(NOT MSVC) 
                SET(EXTRALIB "m")
        ENDIF(NOT MSVC) 
        TARGET_LINK_LIBRARIES(${_lib} ADM_core6 ADM_coreUtils6 ADM_coreAudio6 ADM_coreImage6 ADM_coreUI6 ADM_core6 ADM_coreMuxer6 ${EXTRALIB})
	INSTALL(TARGETS ${_lib} 
                DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/muxers/"
                COMPONENT plugins
                )
ENDMACRO(INSTALL_MUXER)

MACRO(ADD_MUXER name)
        ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
ENDMACRO(ADD_MUXER name)

