##############################
# RPM
##############################
SET(CPACK_RPM_PACKAGE_PROVIDES "avidemux3-${QT_EXTENSION} = ${AVIDEMUX_VERSION}")
SET(CPACK_RPM_PACKAGE_NAME "avidemux3-${QT_EXTENSION}")
SET(CPACK_RPM_PACKAGE_DESCRIPTION "Simple video editor,main program ${QT_EXTENSION} version ")
SET(CPACK_RPM_PACKAGE_SUMMARY "Qt interface for avidemux")
SET(CPACK_RPM_PACKAGE_REQUIRES "avidemux3-core")

include(admCPackRpm)
