########################################
# XVBA
# We only need header for XVBA
# as it is including its own dynlinker
########################################
MACRO(checkXvba)
	if (UNIX)
		IF (NOT XVBA_CHECKED)
			OPTION(XVBA "" ON)

			MESSAGE(STATUS "Checking for XVBA")
			MESSAGE(STATUS "*******************")

			IF (XVBA)
		
				FIND_HEADER_AND_LIB(XVBA amd/amdxvba.h )
		
				IF (XVBA_FOUND)
							SET(XVBA_FOUND 1)
							SET(USE_XVBA 1)
							SET(XVBA_INCLUDE_DIR "${XVBA_INCLUDE_DIR}")
							#SET(XVBA_LIBRARY_DIR "${XVBA_LIBRARY_DIR}")
                                
				ENDIF (XVBA_FOUND)

				PRINT_LIBRARY_INFO("XVBA" XVBA_FOUND "${XVBA_INCLUDE_DIR}" "${XVBA_LIBRARY_DIR}")
			ELSE (XVBA)
				MESSAGE("${MSG_DISABLE_OPTION}")
			ENDIF (XVBA)

			SET(XVBA_CHECKED 1)
			MESSAGE("")
		ENDIF (NOT XVBA_CHECKED)

		APPEND_SUMMARY_LIST("Miscellaneous" "XVBA" "${XVBA_FOUND}")
	endif (UNIX)
ENDMACRO(checkXvba)
