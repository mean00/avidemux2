########################################
# VDPAU
# We only need header for VDPAU
# as it is including its own dynlinker
########################################
MACRO(checkVDPAU)
    if (UNIX)
        OPTION(VDPAU "" ON)
        IF (VDPAU)
            IF (NOT VDPAU_CHECKED)
                MESSAGE(STATUS "Checking for VDPAU")
                MESSAGE(STATUS "******************")

                FIND_HEADER_AND_LIB(VDPAU vdpau/vdpau.h )
                IF (VDPAU_FOUND)
                    # CHECK That it has
                    ADM_COMPILE(check_vdpau.cpp "" "${VDPAU_INCLUDE_DIR}" "${VDPAU_LIBRARY_DIR}" VDPAU_COMPILE VDPAU_OUTPUT)
                    IF (VDPAU_COMPILE)
                        SET(USE_VDPAU True CACHE BOOL "")
                    ELSE (VDPAU_COMPILE)
                        MESSAGE(STATUS "The vdpau found does not compile, probably too old ${VDPAU_OUTPUT}.")
                    ENDIF (VDPAU_COMPILE)
                ENDIF (VDPAU_FOUND)

                SET(VDPAU_CHECKED 1 CACHE INTERNAL "")
                MESSAGE("")

            ELSE (NOT VDPAU_CHECKED)
                MESSAGE(STATUS "Skipping check for VDPAU, using cached results")
                MESSAGE(STATUS "**********************************************")
            ENDIF (NOT VDPAU_CHECKED)

            PRINT_LIBRARY_INFO("VDPAU" VDPAU_FOUND "${VDPAU_INCLUDE_DIR}" "${VDPAU_LIBRARY_DIR}")
            MESSAGE("")
        ELSE (VDPAU)
            MESSAGE("${MSG_DISABLE_OPTION}")
        ENDIF (VDPAU)

        APPEND_SUMMARY_LIST("Miscellaneous" "VDPAU" "${VDPAU_FOUND}")
    endif (UNIX)
ENDMACRO(checkVDPAU)
