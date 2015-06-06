##############################
# DEBIAN
##############################
set(CPACK_COMPONENTS_ALL             runtime dev )

SET(CPACK_DEBIAN_PACKAGE_NAME        "avidemux3-cli")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor,main program, command line version ")
SET(DEPS "libc6 (>=2.4),libglib2.0-0 (>=2.14.0),libstdc++6 (>=4.2.1),libx11-6,   zlib1g (>=1:1.1.4), avidemux3-core-runtime (>=${AVIDEMUX_VERSION})")
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
include(admCPack)

