########################################
# libva
# We only need header for libva
# as it is including its own dynlinker
#######################################
# Check if it options 
MACRO(vaCheck fileToCompile varToSet title)
    ADM_COMPILE(${fileToCompile} "" "${LIBVA_INCLUDE_DIR}" "va" OUTPUT_IS_OK_${varToSet} vaOutput)
    MESSAGE(STATUS "Checking for ${title}, result of OUTPUT_IS_OK_${varToSet} is ${OUTPUT_IS_OK_${varToSet}}")
    IF(OUTPUT_IS_OK_${varToSet})
        MESSAGE(STATUS "${title} is supported")
        SET(LIBVA_CFLAGS ${LIBVA_CFLAGS} -D${varToSet} CACHE INTERNAL "")
        SET(${varToSet} 1 CACHE INTERNAL "")
    ELSE()
        MESSAGE(STATUS "${title} is not supported ")
    ENDIF()
ENDMACRO()
#
MACRO(checkLibVA)
    IF(UNIX)
        OPTION(LIBVA "" ON)
        IF(LIBVA)
            IF(NOT LIBVA_CHECKED)
                MESSAGE(STATUS "Checking for libva")
                MESSAGE(STATUS "******************")

                FIND_HEADER_AND_LIB(LIBVA va/va.h)
                IF(LIBVA_FOUND)
                    MESSAGE(STATUS "Checking it is the right version...")
                    ADM_COMPILE(libva.cpp "" "${LIBVA_INCLUDE_DIR}" "va" LIBVA_COMPILED LIBVA_COMPILE_OUTPUT)
                    IF(LIBVA_COMPILED)
                        SET(USE_LIBVA True CACHE BOOL "")
                        MESSAGE(STATUS "    yes")
                        # Check if it supports HEVC
                        vaCheck(libva_hevcdec.cpp LIBVA_HEVC_DEC "HEVC decoder")
                        # VP8
                        vaCheck(libva_vp9dec.cpp LIBVA_VP9_DEC "VP9 decoder")
                        # AV1
                        vaCheck(libva_av1dec.cpp LIBVA_AV1_DEC "AV1 decoder")
                        # Failsafe
                        vaCheck(libva_dummy.cpp  LIBVA_DUMMY_DEC "DUMMY decoder")
                    ELSE()
                        SET(LIBVA_FOUND 0)
                        MESSAGE(STATUS "Libva compile check failed")
                        MESSAGE(STATUS "${LIBVA_COMPILE_OUTPUT}")
                    ENDIF()
                ENDIF()
                SET(LIBVA_CHECKED 1 CACHE INTERNAL "")
                MESSAGE("")
            ELSE()
                MESSAGE(STATUS "Skipping check for libva, using cached results")
                MESSAGE(STATUS "**********************************************")
            ENDIF()

            PRINT_LIBRARY_INFO("LIBVA" LIBVA_FOUND "${LIBVA_INCLUDE_DIR}" "${LIBVA_CFLAGS}" "${LIBVA_LIBRARY_DIR}")
            MESSAGE("")
        ELSE()
            MESSAGE("${MSG_DISABLE_OPTION}")
        ENDIF()

        APPEND_SUMMARY_LIST("Miscellaneous" "LIBVA" "${LIBVA_FOUND}")
    ENDIF()
ENDMACRO()
