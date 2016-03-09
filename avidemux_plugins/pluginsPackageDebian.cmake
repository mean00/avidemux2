##############################
# DEBIAN
##############################
include(admDebianUtils)
SET(PLUGIN_EXT ${PLUGIN_UI})
IF(${PLUGIN_UI} MATCHES "QT4")
        SET(PLUGIN_EXT ${QT_EXTENSION})
ENDIF(${PLUGIN_UI} MATCHES "QT4")
#
IF(DO_SETTINGS)
        SET(CPACK_COMPONENTS_ALL settings)
        SET(CPACK_DEBIAN_PACKAGE_NAME "avidemux3-settings")
        SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor, settings ")
ELSE(DO_SETTINGS)
        SET(CPACK_COMPONENTS_ALL plugins)
        SET(CPACK_DEBIAN_PACKAGE_NAME "avidemux3-plugins-${PLUGIN_EXT}")
        SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple video editor, plugins (${PLUGIN_EXT} ")
ELSE(DO_SETTINGS)
ENDIF(DO_SETTINGS)
#
# Build our deps list
#
SET(DEPS "libc6 (>=2.4), libstdc++6 (>=4.2.1),libx11-6,  libxv1, zlib1g (>=1:1.1.4), avidemux3-core-runtime (>=${AVIDEMUX_VERSION})")
IF(${PLUGIN_UI} MATCHES "COMMON")
        # Audio decoder
        SETDEBIANDEPS(USE_FAAD libfaad2 DEPS)
        SETDEBIANDEPS(USE_VORBIS "libvorbis0a, libvorbisenc2, libogg0" DEPS)
        SETDEBIANDEPS(USE_LIBOPUS libopus0 DEPS)
        # Audio encoder
        SETDEBIANDEPS(USE_LAME libmp3lame0 DEPS)
        SETDEBIANDEPS(USE_FAAC libfaac0 DEPS)
        # Audio device
        SETDEBIANDEPS(USE_AFTEN libpulse0 DEPS)
ENDIF(${PLUGIN_UI} MATCHES "COMMON")
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
include(admCPack)
