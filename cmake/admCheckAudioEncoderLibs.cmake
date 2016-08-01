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
		SET(USE_AFTEN True CACHE BOOL "")
	        SET(TMP_LIBS "${AFTEN_LIBRARY_DIR}")	
                IF(CROSS)
                                        MESSAGE(WARNING "  Cross compiling mode used. Let's assume aften is recent, it might fail at compile time!")
                                        SET(USE_AFTEN True CACHE BOOL "")
                ELSE(CROSS)

        		IF (NOT DEFINED AFTEN_TEST_RUN_RESULT)
			        TRY_RUN(AFTEN_TEST_RUN_RESULT
				        AFTEN_TEST_COMPILE_RESULT
				        ${CMAKE_BINARY_DIR}
				        "${CMAKE_CURRENT_SOURCE_DIR}/aften/aften_check.cpp"
				        CMAKE_FLAGS -DINCLUDE_DIRECTORIES:PATH=${AFTEN_INCLUDE_DIR}   -DLINK_LIBRARIES:STRING=${TMP_LIBS}
                                        OUTPUT_VARIABLE AFTEN_OUTPUT)
		        ENDIF (NOT DEFINED AFTEN_TEST_RUN_RESULT)
                        #MESSAGE(STATUS " Compile ${AFTEN_TEST_COMPILE_RESULT}, TMP_LIBS=${TMP_LIBS},OUTPUT=${AFTEN_OUTPUT}")
                        IF( AFTEN_TEST_COMPILE_RESULT )
                                IF(AFTEN_TEST_RUN_RESULT EQUAL 99)
                                        MESSAGE(STATUS "  version: SVN")
                                        MESSAGE(WARNING "  This is a svn version of Aften. We will assume it is compatible with 0.0.8, build may fail")
                                        SET(USE_AFTEN True CACHE BOOL "")
		                ELSEIF (AFTEN_TEST_RUN_RESULT EQUAL 8)
                                        MESSAGE(STATUS " This is version 0.8 of aften")
			                SET(USE_AFTEN True CACHE BOOL "")
			                SET(USE_AFTEN_08 1)
                                ENDIF(AFTEN_TEST_RUN_RESULT EQUAL 99)
                        ELSE( AFTEN_TEST_COMPILE_RESULT )
			                MESSAGE(STATUS "  Cannot compile test program to determine Aften version")
			                SET(USE_AFTEN 0)
                        ENDIF( AFTEN_TEST_COMPILE_RESULT )
                ENDIF(CROSS)
	ENDIF (AFTEN_FOUND)
ELSE (AFTEN)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (AFTEN)

APPEND_SUMMARY_LIST("Audio Encoder" "Aften" "${USE_AFTEN}")
MESSAGE("")
########################################
# TWOLAME
########################################
OPTION(TWOLAME "" ON)

MESSAGE(STATUS "Checking for TWOLAME")
MESSAGE(STATUS "*****************")

IF (TWOLAME)
	IF (UNIX)
		SET(TWOLAME_REQUIRED_FLAGS "-lm")
	ENDIF (UNIX)

	FIND_HEADER_AND_LIB(TWOLAME twolame.h twolame twolame_close ${TWOLAME_REQUIRED_FLAGS})
	PRINT_LIBRARY_INFO("TWOLAME" TWOLAME_FOUND "${TWOLAME_INCLUDE_DIR}" "${TWOLAME_LIBRARY_DIR}")

	IF (TWOLAME_FOUND)
		SET(USE_TWOLAME True CACHE BOOL "")
	ENDIF (TWOLAME_FOUND)
ELSE (TWOLAME)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (TWOLAME)

APPEND_SUMMARY_LIST("Audio Encoder" "TWOLAME" "${USE_TWOLAME}")
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
		SET(USE_LAME True CACHE BOOL "")
	ENDIF (LAME_FOUND)
ELSE (LAME)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (LAME)

APPEND_SUMMARY_LIST("Audio Encoder" "LAME" "${USE_LAME}")
MESSAGE("")

########################################
# DCAENC
########################################
OPTION(DCAENC "" ON)

MESSAGE(STATUS "Checking for DCAENC")
MESSAGE(STATUS "*******************")

IF (DCAENC)
	IF (UNIX)
		SET(DCAENC_REQUIRED_FLAGS "-lm")
	ENDIF (UNIX)

	FIND_HEADER_AND_LIB(DCAENC dcaenc.h  dcaenc dcaenc_create ${DCAENC_REQUIRED_FLAGS})
	PRINT_LIBRARY_INFO("DCAENC" DCAENC_FOUND "${DCAENC_INCLUDE_DIR}" "${DCAENC_LIBRARY_DIR}")

	IF (DCAENC_FOUND)
		SET(USE_DCAENC True CACHE BOOL "")
	ENDIF (DCAENC_FOUND)
ELSE (DCAENC)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (DCAENC)

APPEND_SUMMARY_LIST("Audio Encoder" "DCAENC" "${USE_DCAENC}")
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
		SET(USE_FAAC True CACHE BOOL "")
	ENDIF (FAAC_FOUND)
ELSE (FAAC)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (FAAC)

APPEND_SUMMARY_LIST("Audio Encoder" "FAAC" "${USE_FAAC}")
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
		SET(USE_VORBIS True CACHE BOOL "")
	ELSE (VORBIS_FOUND AND VORBISENC_FOUND)
		SET(VORBIS_FOUND 0 CACHE INTERNAL "")
	ENDIF (VORBIS_FOUND AND VORBISENC_FOUND)

	PRINT_LIBRARY_INFO("Vorbis" VORBIS_FOUND "${VORBIS_INCLUDE_DIR}" "${VORBIS_LIBRARY_DIR} ${VORBISENC_LIBRARY_DIR}")
ELSE (VORBIS)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (VORBIS)

APPEND_SUMMARY_LIST("Audio Encoder" "Vorbis" "${USE_VORBIS}")
MESSAGE ("")
