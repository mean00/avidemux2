##############################
# DEBIAN
##############################
set(CPACK_COMPONENTS_ALL             runtime dev )

SET(CPACK_DEBIAN_PACKAGE_NAME        "avidemux3-core")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor, core libraries")
SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(admCPack)
