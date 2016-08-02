#
#  Macro to declare an video decoder plugin
#
SET(VD_PLUGIN_DIR "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/videoDecoders/")
MACRO(INIT_VIDEO_DECODER _lib)
    ADD_DEFINITIONS("-DADM_MINIMAL_UI_INTERFACE")
ENDMACRO(INIT_VIDEO_DECODER)

MACRO(INSTALL_VIDEO_DECODER _lib)
	INSTALL(TARGETS ${_lib} 
                DESTINATION "${VD_PLUGIN_DIR}"
                COMPONENT plugins
                )
	TARGET_LINK_LIBRARIES(${_lib} ADM_core6 ADM_coreUI6 ADM_coreVideoCodec6 ADM_coreImage6 ADM_coreUtils6)
ENDMACRO(INSTALL_VIDEO_DECODER)

