########################################
# VDPAU
# We only need header for VDPAU
# as it is including its own dynlinker
########################################
MACRO(checkVDPAU)
	IF (NOT VDPAU_CHECKED)
		OPTION(VDPAU "" ON)

		MESSAGE(STATUS "Checking for VDPAU")
		MESSAGE(STATUS "*******************")

		IF (VDPAU)
		
			FIND_HEADER_AND_LIB(VDPAU vdpau/vdpau.h )
		
			IF (VDPAU_FOUND)
				SET(VDPAU_FOUND 1)
				SET(USE_VDPAU 1)
				SET(VDPAU_INCLUDE_DIR "${VDPAU_INCLUDE_DIR}")
				SET(VDPAU_LIBRARY_DIR "${VDPAU_LIBRARY_DIR}")
			ENDIF (VDPAU_FOUND)

			PRINT_LIBRARY_INFO("VDPAU" VDPAU_FOUND "${VDPAU_INCLUDE_DIR}" "${VDPAU_LIBRARY_DIR}")
		ELSE (VDPAU)
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF (VDPAU)

		SET(VDPAU_CHECKED 1)

		MESSAGE("")
	ENDIF (NOT VDPAU_CHECKED)
ENDMACRO(checkVDPAU)
