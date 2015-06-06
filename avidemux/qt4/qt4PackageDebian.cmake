##############################
# DEBIAN
##############################
set(CPACK_COMPONENTS_ALL runtime dev )
include(debianArch)
SET(CPACK_SET_DESTDIR "ON")
SET(CPACK_DEBIAN_PACKAGE_NAME "avidemux3-${QT_EXTENSION}")
SET (CPACK_GENERATOR "DEB")
# ARCH
SET_DEBIAN_ARCH()
# Mandatory
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "mean")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor,main program qt4 version ")

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
SET(CPACK_PACKAGE_NAME "avidemux3-${QT_EXTENSION}")
#

include(CPack)
