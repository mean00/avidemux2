########################################
# libva
# We only need header for libva
# as it is including its own dynlinker
########################################
MACRO(checkLibVA)
	if (UNIX)
		IF (NOT LIBVA_CHECKED)
			OPTION(LIBVA "" ON)

			MESSAGE(STATUS "Checking for LIBVA")
			MESSAGE(STATUS "*******************")

			IF (LIBVA)
		
				FIND_HEADER_AND_LIB(LIBVA va/va.h )
		
				IF (LIBVA_FOUND)
                                                MESSAGE(STATUS "Checking it is the right version...")
                                                ADM_COMPILE(libva.cpp "" "${LIBVA_INCLUDE_DIR}" "va" LIBVA_ERROR LIBVA_COMPILE_OUTPUT)
                                                IF( LIBVA_ERROR)
							SET(LIBVA_FOUND 1)
							SET(USE_LIBVA 1)
							SET(LIBVA_INCLUDE_DIR "${LIBVA_INCLUDE_DIR}")
                                                ELSE( LIBVA_ERROR)
                                                        SET(LIBVA_FOUND 0)
                                                        MESSAGE(STATUS "Libva compile check failed")
                                                        MESSAGE(STATUS "${LIBVA_COMPILE_OUTPUT}")
                                                ENDIF( LIBVA_ERROR)
                                
				ENDIF (LIBVA_FOUND)

				PRINT_LIBRARY_INFO("LIBVA" LIBVA_FOUND "${LIBVA_INCLUDE_DIR}" "${LIBVA_LIBRARY_DIR}")
			ELSE (LIBVA)
				MESSAGE("${MSG_DISABLE_OPTION}")
			ENDIF (LIBVA)

			SET(LIBVA_CHECKED 1)
			MESSAGE("")
		ENDIF (NOT LIBVA_CHECKED)

		APPEND_SUMMARY_LIST("Miscellaneous" "LIBVA" "${LIBVA_FOUND}")
	endif (UNIX)
ENDMACRO(checkLibVA)
