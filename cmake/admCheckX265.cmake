MACRO(checkX265)
	IF (NOT X265_CHECKED)
		OPTION(X265 "" ON)

		MESSAGE(STATUS "Checking for x265")
		MESSAGE(STATUS "*****************")

		IF (X265)
			FIND_HEADER_AND_LIB(_X265 x265.h)
			FIND_HEADER_AND_LIB(_X265_CONFIG x265_config.h)

			IF (_X265_FOUND AND _X265_CONFIG_FOUND)
				FILE(READ ${_X265_INCLUDE_DIR}/x265_config.h X265_CONFIG_H)
				STRING(REGEX MATCH "#define[ ]+X265_BUILD[ ]+([0-9]+)" X265_CONFIG_H "${X265_CONFIG_H}")
				STRING(REGEX REPLACE ".*[ ]([0-9]+).*" "\\1" x265_version "${X265_CONFIG_H}")
				MESSAGE(STATUS "  core version: ${x265_version}")
				
				IF (x265_version LESS 9)
					MESSAGE("WARNING: x265 core version is too old.  At least version 9 is required.")
					SET(X265_FOUND 0)
				ELSE (x265_version LESS 9)
                                        IF(NOT WIN32)
                                                SET(DL dl)
                                        ENDIF(NOT WIN32)
					FIND_HEADER_AND_LIB(X265 x265.h x265 x265_encoder_open_${x265_version} ${DL})
				ENDIF (x265_version LESS 9)
			ELSE (_X265_FOUND AND _X265_CONFIG_FOUND)
				SET(X265_FOUND 0)
			ENDIF (_X265_FOUND AND _X265_CONFIG_FOUND)

			PRINT_LIBRARY_INFO("x265" X265_FOUND "${X265_INCLUDE_DIR}" "${X265_LIBRARY_DIR}")
		ELSE (X265)
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF (X265)

		SET(X265_CHECKED 1)

		MESSAGE("")
	ENDIF (NOT X265_CHECKED)

	APPEND_SUMMARY_LIST("Video Encoder" "x265" "${X265_FOUND}")
ENDMACRO(checkX265)
