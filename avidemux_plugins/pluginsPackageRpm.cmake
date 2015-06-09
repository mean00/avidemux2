##############################
# RPM
##############################
SET(PLUGIN_EXT ${PLUGIN_UI})
IF(${PLUGIN_UI} MATCHES "QT4")
        SET(PLUGIN_EXT ${QT_EXTENSION})
ENDIF(${PLUGIN_UI} MATCHES "QT4")
IF(DO_SETTINGS)
        SET(CPACK_COMPONENTS_ALL settings)
        SET(CPACK_RPM_PACKAGE_NAME "avidemux3-settings")
        SET(CPACK_RPM_PACKAGE_DESCRIPTION "Simple video editor, settings ")
        SET(CPACK_RPM_PACKAGE_PROVIDES "avidemux3-settings = ${AVIDEMUX_VERSION}")
ELSE(DO_SETTINGS)
        SET(CPACK_COMPONENTS_ALL plugins)
        SET(CPACK_RPM_PACKAGE_NAME "avidemux3-plugins-${PLUGIN_EXT}")
        SET(CPACK_RPM_PACKAGE_DESCRIPTION "Simple video editor, plugins (${PLUGIN_EXT} ")
        SET(CPACK_RPM_PACKAGE_PROVIDES "avidemux3-plugins-${PLUGIN_EXT} = ${AVIDEMUX_VERSION}")
ENDIF(DO_SETTINGS)

SET(CPACK_RPM_PACKAGE_SUMMARY "${CPACK_RPM_PACKAGE_DESCRIPTION}")

include(admCPackRpm)
