########################################
# VDPAU
# We only need header for VDPAU
# as it is including its own dynlinker
########################################
MACRO(checkVDPAU)
    if (UNIX)
        IF (NOT VDPAU_CHECKED)
            OPTION(VDPAU "" ON)

            MESSAGE(STATUS "Checking for VDPAU")
            MESSAGE(STATUS "*******************")

            IF (VDPAU)
        
                FIND_HEADER_AND_LIB(VDPAU vdpau/vdpau.h )
        
                IF (VDPAU_FOUND)
                            # CHECK That it has 
                            ADM_COMPILE(check_vdpau.cpp "" "${VDPAU_INCLUDE_DIR}" "${VDPAU_LIBRARY_DIR}" VDPAU_COMPILE VDPAU_OUTPUT)
                            IF(VDPAU_COMPILE)
                                SET(VDPAU_FOUND 1)
                                SET(USE_VDPAU 1)
                                SET(VDPAU_INCLUDE_DIR "${VDPAU_INCLUDE_DIR}")
                                SET(VDPAU_LIBRARY_DIR "${VDPAU_LIBRARY_DIR}")
                            ELSE(VDPAU_COMPILE)
                                MESSAGE(STATUS "The vdpau found does not compile, probably too old ${VDPAU_OUTPUT}.")
                            ENDIF(VDPAU_COMPILE)
                                
                ENDIF (VDPAU_FOUND)

                PRINT_LIBRARY_INFO("VDPAU" VDPAU_FOUND "${VDPAU_INCLUDE_DIR}" "${VDPAU_LIBRARY_DIR}")
            ELSE (VDPAU)
                MESSAGE("${MSG_DISABLE_OPTION}")
            ENDIF (VDPAU)

            SET(VDPAU_CHECKED 1)
            MESSAGE("")
        ENDIF (NOT VDPAU_CHECKED)

        APPEND_SUMMARY_LIST("Miscellaneous" "VDPAU" "${VDPAU_FOUND}")
    endif (UNIX)
ENDMACRO(checkVDPAU)
