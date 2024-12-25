########################################
# VDPAU
# We only need header for VDPAU
# as it is including its own dynlinker
########################################
MACRO(checkVDPAU)
    IF(UNIX)
        OPTION(VDPAU "" ON)
        IF(VDPAU)
            IF(NOT VDPAU_CHECKED)
                MESSAGE(STATUS "Checking for VDPAU")
                MESSAGE(STATUS "******************")

                FIND_HEADER_AND_LIB(VDPAU vdpau/vdpau.h )
                IF(VDPAU_FOUND)
                    # CHECK That it has
                    ADM_COMPILE(check_vdpau.cpp "" "${VDPAU_INCLUDE_DIR}" "${VDPAU_LIBRARY_DIR}" VDPAU_COMPILE VDPAU_OUTPUT)
                    IF(VDPAU_COMPILE)
                        SET(USE_VDPAU True CACHE BOOL "")
                    ELSE()
                        MESSAGE(STATUS "The vdpau found does not compile, probably too old ${VDPAU_OUTPUT}.")
                    ENDIF()
                ENDIF()

                SET(VDPAU_CHECKED 1 CACHE INTERNAL "")
                MESSAGE("")

            ELSE()
                MESSAGE(STATUS "Skipping check for VDPAU, using cached results")
                MESSAGE(STATUS "**********************************************")
            ENDIF()

            PRINT_LIBRARY_INFO("VDPAU" VDPAU_FOUND "${VDPAU_INCLUDE_DIR}" "${VDPAU_LIBRARY_DIR}")
            MESSAGE("")
        ELSE()
            MESSAGE("${MSG_DISABLE_OPTION}")
        ENDIF()

        APPEND_SUMMARY_LIST("Miscellaneous" "VDPAU" "${VDPAU_FOUND}")
    ENDIF()
ENDMACRO()
