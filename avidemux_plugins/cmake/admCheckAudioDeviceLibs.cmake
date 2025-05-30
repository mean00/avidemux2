########################################
# ALSA
########################################
IF(UNIX AND NOT APPLE)
	OPTION(ALSA "" ON)

	MESSAGE(STATUS "Checking for ALSA")
	MESSAGE(STATUS "*****************")

	IF(ALSA)
		FIND_PACKAGE(Alsa)

		IF(ALSA_FOUND)
			ALSA_VERSION_STRING(alsaVersion)

			MESSAGE(STATUS "  version: ${alsaVersion}")
			PRINT_LIBRARY_INFO("ALSA" ALSA_FOUND "${_ALSA_INCLUDE_DIR}" "${ASOUND_LIBRARY}")

			SET(USE_ALSA True CACHE BOOL "")
		ENDIF()
	ELSE()
		MESSAGE("${MSG_DISABLE_OPTION}")
	ENDIF()

	MESSAGE("")
	APPEND_SUMMARY_LIST("Audio Device" "ALSA" "${USE_ALSA}")
ELSE()
	SET(ALSA_CAPABLE FALSE)
ENDIF()

########################################
# aRts
########################################
IF(UNIX)
	OPTION(ARTS "" ON)

	MESSAGE(STATUS "Checking for aRts")
	MESSAGE(STATUS "*****************")

	IF(ARTS)
		FIND_PACKAGE(Arts)
		PRINT_LIBRARY_INFO("aRts" ARTS_FOUND "${ARTS_CFLAGS}" "${ARTS_LDFLAGS}")

		IF(ARTS_FOUND)
			SET(USE_ARTS True CACHE BOOL "")
		ENDIF()
	ELSE()
		MESSAGE("${MSG_DISABLE_OPTION}")
	ENDIF()

	MESSAGE("")
	APPEND_SUMMARY_LIST("Audio Device" "aRts" "${USE_ARTS}")
ELSE()
	SET(ARTS_CAPABLE FALSE)
ENDIF()

########################################
# ESD
########################################
IF(UNIX)
	OPTION(ESD "" ON)

	MESSAGE(STATUS "Checking for ESD")
	MESSAGE(STATUS "****************")

	IF(ESD)
		FIND_HEADER_AND_LIB(ESD esd.h esd esd_close)
		PRINT_LIBRARY_INFO("ESD" ESD_FOUND "${ESD_INCLUDE_DIR}" "${ESD_LIBRARY_DIR}")

		IF(ESD_FOUND)
			SET(USE_ESD True CACHE BOOL "")
		ENDIF()
	ELSE()
		MESSAGE("${MSG_DISABLE_OPTION}")
	ENDIF()

	MESSAGE("")
	APPEND_SUMMARY_LIST("Audio Device" "ESD" "${USE_ESD}")
ELSE()
	SET(ESD_CAPABLE FALSE)
ENDIF()

########################################
# JACK
########################################
IF(UNIX)
	OPTION(JACK "" ON)

	MESSAGE(STATUS "Checking for JACK")
	MESSAGE(STATUS "*****************")

	IF(JACK)
		FIND_HEADER_AND_LIB(JACK jack/jack.h jack jack_client_close)
		PRINT_LIBRARY_INFO("JACK" JACK_FOUND "${JACK_INCLUDE_DIR}" "${JACK_LIBRARY_DIR}")

		IF(JACK_FOUND)
			SET(USE_JACK 1)
		ENDIF()
	ELSE()
		MESSAGE("${MSG_DISABLE_OPTION}")
	ENDIF()

	MESSAGE("")
	APPEND_SUMMARY_LIST("Audio Device" "JACK" "${USE_JACK}")
ELSE()
	SET(JACK_CAPABLE FALSE)
ENDIF()

########################################
# Secret Rabbit Code
########################################
IF(JACK_FOUND)
	MESSAGE(STATUS "Checking for Secret Rabbit Code")
	MESSAGE(STATUS "*******************************")

	FIND_HEADER_AND_LIB(SRC samplerate.h samplerate src_get_version)
	PRINT_LIBRARY_INFO("Secret Rabbit Code" SRC_FOUND "${SRC_INCLUDE_DIR}" "${SRC_LIBRARY_DIR}")

	IF(SRC_FOUND)
		SET(USE_SRC 1)
	ENDIF()

	MESSAGE("")
	APPEND_SUMMARY_LIST("Audio Device" "SRC" "${USE_SRC}")
ENDIF()

########################################
# OSS
########################################
IF(UNIX AND NOT APPLE)
	OPTION(OSS "" ON)

	MESSAGE(STATUS "Checking for OSS")
	MESSAGE(STATUS "****************")

	IF(OSS)
		FIND_HEADER_AND_LIB(OSS sys/soundcard.h)
		PRINT_LIBRARY_INFO("OSS" OSS_FOUND "${OSS_INCLUDE_DIR}" "")

		IF(OSS_FOUND)
			SET(USE_OSS 1)
		ENDIF()
	ELSE()
		MESSAGE("${MSG_DISABLE_OPTION}")
	ENDIF()

	MESSAGE("")
	APPEND_SUMMARY_LIST("Audio Device" "OSS" "${USE_OSS}")
ELSE()
	SET(OSS_CAPABLE FALSE)
ENDIF()

########################################
# PulseAudioSimple
########################################
IF(UNIX AND NOT APPLE)
#[[
	OPTION(PULSEAUDIOSIMPLE "" ON)

	MESSAGE(STATUS "Checking for PULSEAUDIOSIMPLE")
	MESSAGE(STATUS "*****************************")
        IF(PULSEAUDIOSIMPLE_INCLUDE_DIR AND PULSEAUDIOSIMPLE_LIBRARIES)
        # in cache already
         SET(PULSEAUDIOSIMPLE_FIND_QUIETLY TRUE)
        ENDIF()

           # use pkg-config to get the directories and then use these values
        # in the FIND_PATH() and FIND_LIBRARY() calls
        include(FindPkgConfig)
        pkg_check_modules(PULSEAUDIOSIMPLE libpulse-simple)
        IF(PULSEAUDIOSIMPLE_FOUND)
          SET(PULSEAUDIOSIMPLE_DEFINITIONS ${PULSEAUDIOSIMPLE_CFLAGS})
          FIND_PATH(PULSEAUDIOSIMPLE_INCLUDE_DIR pulse/simple.h PATHS ${PULSEAUDIOSIMPLE_INCLUDE_DIRS} PATH_SUFFIXES pulse)
          FIND_LIBRARY(PULSEAUDIOSIMPLE_LIBRARIES NAMES pulse-simple libpulse-simple PATHS ${PULSEAUDIOSIMPLE_LIBRARY_DIRS})
        ENDIF()

        IF(PULSEAUDIOSIMPLE_INCLUDE_DIR AND PULSEAUDIOSIMPLE_LIBRARIES)
         SET(PULSEAUDIOSIMPLE_FOUND TRUE)
        ELSE()
         SET(PULSEAUDIOSIMPLE_FOUND FALSE)
        ENDIF()

        IF(PULSEAUDIOSIMPLE_FOUND)
         IF(NOT PULSEAUDIOSIMPLE_FIND_QUIETLY)
          MESSAGE(STATUS "Found PulseAudio Simple: ${PULSEAUDIOSIMPLE_LIBRARIES}")
        ENDIF()
        SET(USE_PULSE_SIMPLE 1)
        ELSE()
         MESSAGE(STATUS "Could NOT find PulseAudioSimple\n")
        ENDIF()

        MARK_AS_ADVANCED(PULSEAUDIOSIMPLE_INCLUDE_DIR PULSEAUDIOSIMPLE_LIBRARIES)

		APPEND_SUMMARY_LIST("Audio Device" "PulseAudioS" "${USE_PULSE_SIMPLE}")
]]
    OPTION(PULSEAUDIO "" ON)
    MESSAGE(STATUS "Checking for PulseAudio")
    include(FindPkgConfig)
    pkg_check_modules(PULSEAUDIO libpulse)
    IF(PULSEAUDIO_FOUND)
        SET(PULSEAUDIO_DEFINITIONS ${PULSEAUDIO_CFLAGS})
        SET(USE_PULSE 1)
    ELSE()
        MESSAGE(STATUS "Could not find PulseAudio")
    ENDIF()
    APPEND_SUMMARY_LIST("Audio Device" "PulseAudio" "${USE_PULSE}")
ELSE()
	SET(PULSEAUDIOSIMPLE_CAPABLE FALSE)
ENDIF()
