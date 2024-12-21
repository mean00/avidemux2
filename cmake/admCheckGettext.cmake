MACRO(checkGettext)
	IF (NOT GETTEXT_CHECKED)
		OPTION(GETTEXT "" ON)

		MESSAGE(STATUS "Checking for gettext")
		MESSAGE(STATUS "********************")

		IF (GETTEXT)
			FIND_HEADER_AND_LIB(GETTEXT libintl.h intl)
			PRINT_LIBRARY_INFO("Gettext" GETTEXT_FOUND "${GETTEXT_INCLUDE_DIR}" "${GETTEXT_LIBRARY_DIR}")

			IF (GETTEXT_INCLUDE_DIR)
				# Try linking without -lintl
				ADM_COMPILE(gettext.cpp "" "${GETTEXT_INCLUDE_DIR}" "" LIBINTL_NOT_REQUIRED outputWithoutLibintl)

				IF (LIBINTL_NOT_REQUIRED)
					SET(GETTEXT_LIBRARY_DIR "")
					SET(HAVE_GETTEXT 1)

					MESSAGE(STATUS "libintl not required for gettext support")
				ELSE (LIBINTL_NOT_REQUIRED)
					ADM_COMPILE(gettext.cpp "" "${GETTEXT_INCLUDE_DIR}" "${GETTEXT_LIBRARY_DIR}" LIBINTL_REQUIRED outputWithLibintl)

					IF (LIBINTL_REQUIRED)
						SET(HAVE_GETTEXT 1)

						MESSAGE(STATUS "libintl required for gettext support")
					ELSE (LIBINTL_REQUIRED)
						SET(GETTEXT_FOUND 0 CACHE INTERNAL "")

						MESSAGE("gettext support failed with or without libintl")
						
						IF (VERBOSE)
							MESSAGE(STATUS "Compilation error with libintl:")
							MESSAGE(STATUS ${outputWithLibintl})

							MESSAGE(STATUS "Compilation error without libintl:")
							MESSAGE(STATUS ${outputWithoutLibintl})
						ENDIF (VERBOSE)
					ENDIF (LIBINTL_REQUIRED)
				ENDIF (LIBINTL_NOT_REQUIRED)
			ENDIF (GETTEXT_INCLUDE_DIR)
		ELSE (GETTEXT)
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF (GETTEXT)

		SET(GETTEXT_CHECKED 1)
		MESSAGE("")
	ENDIF (NOT GETTEXT_CHECKED)

	APPEND_SUMMARY_LIST("Miscellaneous" "gettext" "${HAVE_GETTEXT}")
ENDMACRO(checkGettext)