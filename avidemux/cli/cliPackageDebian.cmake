##############################
# DEBIAN
##############################
set(CPACK_COMPONENTS_ALL             runtime dev )

SET(CPACK_DEBIAN_PACKAGE_NAME        "avidemux3-cli")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor,main program, command line version ")
SET(DEPS "avidemux3-core-runtime (>=${AVIDEMUX_VERSION})")
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
include(admCPack)

