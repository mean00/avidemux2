########################################
# Aften
########################################
OPTION(AFTEN "" ON)

MESSAGE(STATUS "Checking for Aften")
MESSAGE(STATUS "******************")

IF (AFTEN)
	FIND_HEADER_AND_LIB(AFTEN aften/aften.h aften aften_encode_init)
	PRINT_LIBRARY_INFO("Aften" AFTEN_FOUND "${AFTEN_INCLUDE_DIR}" "${AFTEN_LIBRARY_DIR}")

	IF (AFTEN_FOUND)
		SET(USE_AFTEN 1)
		
		IF (NOT DEFINED AFTEN_TEST_RUN_RESULT)
			TRY_RUN(AFTEN_TEST_RUN_RESULT
				AFTEN_TEST_COMPILE_RESULT
				${CMAKE_BINARY_DIR}
				"${AVIDEMUX_SOURCE_DIR}/cmake_compile_check/aften_check.cpp"
				CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${AFTEN_INCLUDE_DIR}" "-DLINK_LIBRARIES=${AFTEN_LIBRARY_DIR}")
		ENDIF (NOT DEFINED AFTEN_TEST_RUN_RESULT)

		IF (AFTEN_TEST_RUN_RESULT EQUAL 8)
			MESSAGE(STATUS "  version: 0.0.8")
			SET(USE_AFTEN_08 1)
		ELSEIF (AFTEN_TEST_RUN_RESULT EQUAL 7)
			MESSAGE(STATUS "  version: 0.07")
			SET(USE_AFTEN_07 1)
		ELSE (AFTEN_TEST_RUN_RESULT EQUAL 8)
			MESSAGE(STATUS "Warning: Unable to determine Aften version - support for Aften will be turned off")
			SET(USE_AFTEN 0)
		ENDIF (AFTEN_TEST_RUN_RESULT EQUAL 8)
	ENDIF (AFTEN_FOUND)
ELSE (AFTEN)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (AFTEN)

MESSAGE("")

########################################
# LAME
########################################
OPTION(LAME "" ON)

MESSAGE(STATUS "Checking for LAME")
MESSAGE(STATUS "*****************")

IF (LAME)
	IF (UNIX)
		SET(LAME_REQUIRED_FLAGS "-lm")
	ENDIF (UNIX)

	FIND_HEADER_AND_LIB(LAME lame/lame.h mp3lame lame_init ${LAME_REQUIRED_FLAGS})
	PRINT_LIBRARY_INFO("LAME" LAME_FOUND "${LAME_INCLUDE_DIR}" "${LAME_LIBRARY_DIR}")

	IF (LAME_FOUND)
		SET(USE_LAME 1)
	ENDIF (LAME_FOUND)
ELSE (LAME)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (LAME)

MESSAGE("")


########################################
# FAAC
########################################
OPTION(FAAC "" ON)

MESSAGE(STATUS "Checking for FAAC")
MESSAGE(STATUS "*****************")

IF (FAAC)
	FIND_HEADER_AND_LIB(FAAC faac.h faac faacEncClose)
	PRINT_LIBRARY_INFO("FAAC" FAAC_FOUND "${FAAC_INCLUDE_DIR}" "${FAAC_LIBRARY_DIR}")

	IF (FAAC_FOUND)
		SET(USE_FAAC 1)
	ENDIF (FAAC_FOUND)
ELSE (FAAC)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (FAAC)

MESSAGE("")

########################################
# Vorbis
########################################
OPTION(VORBIS "" ON)

MESSAGE(STATUS "Checking for Vorbis")
MESSAGE(STATUS "*******************")

IF (VORBIS)
	FIND_HEADER_AND_LIB(VORBIS vorbis/vorbisenc.h vorbis vorbis_info_init)
	FIND_HEADER_AND_LIB(VORBISENC "" vorbisenc vorbis_encode_init)

	IF (VORBIS_FOUND AND VORBISENC_FOUND)
		SET(USE_VORBIS 1)
	ELSE (VORBIS_FOUND AND VORBISENC_FOUND)
		SET(VORBIS_FOUND 0 CACHE INTERNAL "")
	ENDIF (VORBIS_FOUND AND VORBISENC_FOUND)

	PRINT_LIBRARY_INFO("Vorbis" VORBIS_FOUND "${VORBIS_INCLUDE_DIR}" "${VORBIS_LIBRARY_DIR} ${VORBISENC_LIBRARY_DIR}")
ELSE (VORBIS)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (VORBIS)

MESSAGE ("")
