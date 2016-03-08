##############################
# DEBIAN
##############################

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
SET(DEPS "libc6 (>=2.4), libstdc++6 (>=4.2.1),libx11-6,  libxv1, zlib1g (>=1:1.1.4), avidemux3-core-runtime (>=${AVIDEMUX_VERSION})")
# Audio decoder
IF(USE_FAAD)
    SET(DEPS "${DEPS},libfaad2")
ENDIF(USE_FAAD)
IF(USE_VORBIS)
    SET(DEPS "${DEPS},libvorbis0a, libvorbisenc2, libogg")
ENDIF(USE_VORBIS)
IF(USE_OPUS)
    SET(DEPS "${DEPS},libopus0")
ENDIF(USE_OPUS)
# Audio encoder
IF(USE_LAME)
    SET(DEPS "${DEPS},libmp3lame0")
ENDIF(USE_LAME)
IF(USE_FAAC)
    SET(DEPS "${DEPS},libfaac0")
ENDIF(USE_FAAC)
IF(USE_AFTEN)
    SET(DEPS "${DEPS},libaften0")
ENDIF(USE_AFTEN)
# Audio device
IF(USE_PULSE_SIMPLE)
    SET(DEPS "${DEPS},libpulse0")
ENDIF(USE_PULSE_SIMPLE)
# Add optional DEPS here
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")
#
include(admCPack)
