##############################
# DEBIAN
##############################
SET(CPACK_SET_DESTDIR "ON")
SET(CPACK_DEBIAN_PACKAGE_NAME "avidemux2-core")
SET (CPACK_GENERATOR "DEB")
# ARCH
IF (X86_64_SUPPORTED)
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
ELSE (X86_64_SUPPORTED)
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
ENDIF (X86_64_SUPPORTED)
# Mandatory
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "mean")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editori,core libraries")

# Build our deps list
# Build deps SET(DEPS "debhelper (>> 4), libfreetype6-dev, libxml2-dev,  libxv-dev, dpatch , cmake, desktop-file-utils")
SET(DEPS "libc6 (>=2.4),libglib2.0-0 (>=2.14.0),libstdc++6 (>=4.2.1),libx11-6,  libxml2 (>=2.6.27), libxv1, zlib1g (>=1:1.1.4), pkg-config, libpng12-0, libxml2")
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
SET(CPACK_DEBIAN_PACKAGE_SECTION "extra")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
# Some more infos
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "6")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
if(Subversion_FOUND)
        SET(CPACK_PACKAGE_VERSION_PATCH "0-r${ADM_SUBVERSION}")
endif(Subversion_FOUND)
#
SET(CPACK_PACKAGE_NAME "avidemux2-core")
#

include(CPack)
