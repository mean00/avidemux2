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
ADM_DISPLAY("GTK+      " "${ADM_UI_GTK}")
ADM_DISPLAY("Qt 4      " "${ADM_UI_QT4}")

#MESSAGE("*** Audio Device  ***")
#ADM_DISPLAY("ALSA      " "${ALSA_SUPPORT}" "${ALSA_CAPABLE}")
#ADM_DISPLAY("aRts      " "${USE_ARTS}" "${ARTS_CAPABLE}")
#ADM_DISPLAY("ESD       " "${USE_ESD}" "${ESD_CAPABLE}")
#ADM_DISPLAY("JACK      " "${USE_JACK}" "${JACK_CAPABLE}")
#ADM_DISPLAY("OSS       " "${OSS_SUPPORT}" "${OSS_CAPABLE}")
#ADM_DISPLAY("SRC       " "${USE_SRC}" "${JACK_CAPABLE}")

#IF (WIN32)
#	ADM_DISPLAY("Win32     " 1)
#ENDIF (WIN32)

MESSAGE("*** Miscellaneous ***")
ADM_DISPLAY("gettext   " "${HAVE_GETTEXT}")
ADM_DISPLAY("SDL       " "${USE_SDL}")
ADM_DISPLAY("XVideo    " "${USE_XV}" "${XVIDEO_CAPABLE}")

MESSAGE("*********************")

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE("***  Debug Build  ***")
ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE("*** Release Build ***")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

MESSAGE("*********************")
