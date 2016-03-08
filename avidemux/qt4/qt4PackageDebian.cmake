##############################
# DEBIAN
##############################
set(CPACK_COMPONENTS_ALL             runtime dev )

SET(CPACK_DEBIAN_PACKAGE_NAME        "avidemux3-${QT_EXTENSION}")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor,main program qt4 version ")
SET(DEPS "libc6 (>=2.4),libstdc++6 (>=4.2.1),libx11-6,  libxv1, zlib1g (>=1:1.1.4), libglib2.0-0, libpng12-0, avidemux3-core-runtime (>=${AVIDEMUX_VERSION})")
# QT5
SET(DEPS "${DEPS}, libqt5opengl5, libqt5widgets5, libqt5gui5, libqt5core5a")
IF(USE_XV)
        SET(DEPS "${DEPS}, libxv1")
ENDIF(USE_XV)
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
include(admCPack)
##
