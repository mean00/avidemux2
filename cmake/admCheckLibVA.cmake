########################################
# libva
# We only need header for libva
# as it is including its own dynlinker
#######################################
# Check if it options 
MACRO(vaCheck fileToCompile varToSet title)
   ADM_COMPILE(${fileToCompile} "" "${LIBVA_INCLUDE_DIR}" "va" OUTPUT_IS_OK_${varToSet} vaOutput)
   MESSAGE(STATUS "Checking for ${title}, result of OUTPUT_IS_OK_${varToSet} is  ${OUTPUT_IS_OK_${varToSet}}")
   IF(OUTPUT_IS_OK_${varToSet})
           MESSAGE(STATUS "${title} is supported") 
           #MESSAGE(STATUS "(${vaOutput})") 
           SET(LIBVA_CFLAGS ${LIBVA_CFLAGS} -D${varToSet})
   ELSE(OUTPUT_IS_OK_${varToSet})
           MESSAGE(STATUS "${title} is not supported ") 
   ENDIF(OUTPUT_IS_OK_${varToSet})
ENDMACRO(vaCheck fileToCompile varToSet title)
#
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
                                                ADM_COMPILE(libva.cpp "" "${LIBVA_INCLUDE_DIR}" "va" LIBVA_COMPILED LIBVA_COMPILE_OUTPUT)
                                                IF( LIBVA_COMPILED)
							SET(LIBVA_FOUND 1)
							SET(USE_LIBVA 1)
							SET(LIBVA_INCLUDE_DIR "${LIBVA_INCLUDE_DIR}")
                                                        MESSAGE(STATUS "    yes")
                                                        # Check if it supports HEVC
                                                        vaCheck(libva_hevcdec.cpp  LIBVA_HEVC_DEC "HEVC decoder")
                                                        # VP8
                                                        vaCheck(libva_vp9dec.cpp  LIBVA_VP9_DEC "VP9 decoder")
                                                        # Failsafe
                                                        vaCheck(libva_dummy.cpp  LIBVA_DUMMY_DEC "DUMMY decoder")
                                                ELSE( LIBVA_COMPILED)
                                                        SET(LIBVA_FOUND 0)
                                                        MESSAGE(STATUS "Libva compile check failed")
                                                        MESSAGE(STATUS "${LIBVA_COMPILE_OUTPUT}")
                                                ENDIF( LIBVA_COMPILED)
                                
				ENDIF (LIBVA_FOUND)
                                MESSAGE(STATUS "LibVA cflags=<${LIBVA_CFLAGS}>")
				PRINT_LIBRARY_INFO("LIBVA" LIBVA_FOUND "${LIBVA_INCLUDE_DIR},${LIBVA_CFLAGS}" "${LIBVA_LIBRARY_DIR}")
			ELSE (LIBVA)
				MESSAGE("${MSG_DISABLE_OPTION}")
			ENDIF (LIBVA)

			SET(LIBVA_CHECKED 1)
			MESSAGE("")
		ENDIF (NOT LIBVA_CHECKED)

		APPEND_SUMMARY_LIST("Miscellaneous" "LIBVA" "${LIBVA_FOUND}")
	endif (UNIX)
ENDMACRO(checkLibVA)
