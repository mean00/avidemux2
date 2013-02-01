##############################
# DEBIAN
##############################
SET(CPACK_SET_DESTDIR "ON")
SET (CPACK_GENERATOR "TGZ")
# Some more infos
#
SET(CPACK_PACKAGE_NAME "avidemux3-plugins-${PLUGIN_UI}")
#

include(CPack)
