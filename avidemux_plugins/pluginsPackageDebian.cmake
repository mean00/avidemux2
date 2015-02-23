##############################
# DEBIAN
##############################

SET(PLUGIN_EXT ${PLUGIN_UI})
IF(${PLUGIN_UI} MATCHES "QT4")
        SET(PLUGIN_EXT ${QT_EXTENSION})
ENDIF(${PLUGIN_UI} MATCHES "QT4")
include(debianArch)
SET(CPACK_SET_DESTDIR "ON")
SET(CPACK_DEBIAN_PACKAGE_NAME "avidemux3-plugins-${PLUGIN_EXT}")
SET (CPACK_GENERATOR "DEB")
# ARCH
SET_DEBIAN_ARCH()
# Mandatory
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "mean")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor, plugins ")

# Build our deps list
# Build deps SET(DEPS "debhelper (>> 4), libfreetype6-dev, libxml2-dev,  libxv-dev, dpatch , cmake, desktop-file-utils")
SET(DEPS "libc6 (>=2.4),libglib2.0-0 (>=2.14.0),libstdc++6 (>=4.2.1),libx11-6,  libxv1, zlib1g (>=1:1.1.4), avidemux3-core (>=${AVIDEMUX_VERSION})")
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
SET(CPACK_DEBIAN_PACKAGE_SECTION "extra")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
# Some more infos
#
SET(CPACK_PACKAGE_NAME "avidemux3-plugins-${PLUGIN_EXT}")
#

include(CPack)
