##############################
# RPM
##############################
SET(CPACK_RPM_PACKAGE_PROVIDES "avidemux3-core = ${AVIDEMUX_VERSION}")
SET(CPACK_RPM_PACKAGE_NAME "avidemux3-core")
SET(CPACK_RPM_PACKAGE_DESCRIPTION "Simple video editor,core libraries and development files.")
SET(CPACK_RPM_PACKAGE_SUMMARY "Graphical video editing and transcoding tool and its development files.")

include(admCPackRpm)
