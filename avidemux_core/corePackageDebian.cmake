##############################
# DEBIAN
##############################
set(CPACK_COMPONENTS_ALL             runtime dev )

SET(CPACK_DEBIAN_PACKAGE_NAME        "avidemux3-core")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor,core libraries")
SET(DEPS "libc6 (>=2.4),libstdc++6 (>=4.2.1),libx11-6,  libxv1, zlib1g (>=1:1.1.4), pkg-config, libpng12-0, libsqlite3-0 (>=3.8.0) ")
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
include(admCPack)
