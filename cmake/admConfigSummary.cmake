# ARGV2 - capable flag
MACRO(ADM_DISPLAY _name)
	SET(PROCEED 1)

	IF (NOT ${ARGV2})
		SET(PROCEED 0)
	ENDIF (NOT ${ARGV2})

	IF (${PROCEED})
		IF (${ARGV1})
			SET(s "Yes")
		ELSE (${ARGV1})
			SET(s "No")
		ENDIF (${ARGV1})

		MESSAGE("   ${_name}  ${s}")
	ENDIF (${PROCEED})
ENDMACRO(ADM_DISPLAY)

MESSAGE("*********************")
MESSAGE("***    SUMMARY    ***")
MESSAGE("*********************")

MESSAGE("*** Miscellaneous ***")
ADM_DISPLAY("gettext   " "${HAVE_GETTEXT}")
ADM_DISPLAY("SDL       " "${USE_SDL}")
ADM_DISPLAY("XVideo    " "${USE_XV}" "${XVIDEO_CAPABLE}")
ADM_DISPLAY("VDPAU     " "${USE_VDPAU}" )
ADM_DISPLAY("YASM      " "${USE_YASM}" )

MESSAGE("*********************")

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE("***  Debug Build  ***")
ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE("*** Release Build ***")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

MESSAGE("*********************")
