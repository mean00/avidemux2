#
#  Macro to declare an video encoder plugin
#
include(admCmakeParseArgument)
SET(VE_SETTINGS_DIR "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/pluginSettings/")

MACRO(INSTALL_VIDEO_ENCODER_SETTINGS PLUGIN)
    PARSE_ARGUMENTS(SETTINGS   
                        "VERSION;FILES"    
                        ""
                        ${ARGN})   
    MESSAGE(STATUS "Will install plugin ${PLUGIN} version ${SETTINGS_VERSION}")
    IF(SETTINGS_FILES)
        MESSAGE(STATUS "Will install ${SETTINGS_FILES}")
        INSTALL(FILES ${SETTINGS_FILES}
                DESTINATION "${VE_SETTINGS_DIR}/${PLUGIN}/${SETTINGS_VERSION}/"
                COMPONENT   settings
                )
    ENDIF(SETTINGS_FILES)
    MESSAGE("")
ENDMACRO(INSTALL_VIDEO_ENCODER_SETTINGS)


