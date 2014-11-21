MACRO(checkX265)
	IF (NOT X265_CHECKED)
		OPTION(X265 "" ON)

		MESSAGE(STATUS "Checking for x265")
		MESSAGE(STATUS "*****************")

		IF (X265)
			FIND_HEADER_AND_LIB(_X265 x265.h)

			IF (_X265_FOUND)
				FILE(READ ${_X265_INCLUDE_DIR}/x265.h X265_H)
				STRING(REGEX MATCH "#define[ ]+X265_BUILD[ ]+([0-9]+)" X265_H "${X265_H}")
                                MESSAGE(STATUS "  test: ${X265_H}")
                                STRING(REGEX REPLACE ".*[ ]([0-9]+).*[ ]([0-9]+).*" "\\1" x265_major_version "${X265_H}")
				STRING(REGEX REPLACE ".*[ ]([0-9]+).*" "\\1" x265_minor_version "${X265_H}")
				MESSAGE(STATUS "  major version: ${x265_major_version}  minor version: ${x265_minor_version}")
				
				IF (x265_major_version LESS 1)
					MESSAGE("WARNING: x265 core version is too old.  At least version 1.0 is required.")
					SET(X265_FOUND 0)
				ELSEIF (x265_major_version GREATER 0)
					FIND_HEADER_AND_LIB(X265 x265.h x265 x265_encoder_open_${x265_version})
				ELSE (x265_version LESS 1)
					FIND_HEADER_AND_LIB(X265 x265.h x265 x265_encoder_open)
				ENDIF (x265_version LESS 1)
			ELSE (_X265_FOUND)
				SET(X265_FOUND 0)
			ENDIF (_X265_FOUND)

			PRINT_LIBRARY_INFO("x265" X265_FOUND "${X265_INCLUDE_DIR}" "${X265_LIBRARY_DIR}")
		ELSE (X265)
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF (X265)

		SET(X265_CHECKED 1)

		MESSAGE("")
	ENDIF (NOT X265_CHECKED)

	APPEND_SUMMARY_LIST("Video Encoder" "x265" "${X265_FOUND}")
ENDMACRO(checkX265)
