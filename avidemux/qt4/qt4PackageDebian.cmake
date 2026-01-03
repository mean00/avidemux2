##############################
# DEBIAN
##############################
set(CPACK_COMPONENTS_ALL             runtime dev )

SET(CPACK_DEBIAN_PACKAGE_NAME        "avidemux3-${QT_EXTENSION}")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor, main program Qt version ")
SET(DEPS "avidemux3-core-runtime (>=${AVIDEMUX_VERSION})")
IF("${QT_EXTENSION}" STREQUAL "qt6")
  SET(DEPS "${DEPS}, libqt6core6, libqt6gui6, libqt6network6, libqt6widgets6, libqt6openglwidgets6")
ELSE()
  IF("${QT_EXTENSION}" STREQUAL "qt5")
    SET(DEPS "${DEPS}, libqt5core5a, libqt5gui5, libqt5network5, libqt5widgets5, libqt5opengl5")
  ENDIF()
ENDIF()
IF(USE_XV)
        SET(DEPS "${DEPS}, libxv1")
ENDIF(USE_XV)
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
include(admCPack)
##
