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
MESSAGE("***  Video Codec  ***")
ADM_DISPLAY("x264      " "$ENV{ADM_HAVE_X264}")
ADM_DISPLAY("Xvid      " "$ENV{ADM_HAVE_XVID}")

MESSAGE("***  Audio Codec  ***")
ADM_DISPLAY("amrnb     " "$ENV{ADM_HAVE_AMRNB}")
ADM_DISPLAY("FAAD      " "$ENV{ADM_HAVE_FAAD}")
ADM_DISPLAY("libdca    " "$ENV{ADM_HAVE_LIBDCA}")

MESSAGE("*** Miscellaneous ***")
ADM_DISPLAY("FontConfig" "$ENV{ADM_HAVE_FONTCONFIG}")
ADM_DISPLAY("FreeType2 " "${FREETYPE2_FOUND}")
ADM_DISPLAY("gettext   " "${HAVE_GETTEXT}")

MESSAGE("*********************")

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE("***  Debug Build  ***")
ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE("*** Release Build ***")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")
MESSAGE("******** UI *************")
ADM_DISPLAY("Common    " "${DO_COMMON}")
ADM_DISPLAY("Gtk       " "${DO_GTK}")
ADM_DISPLAY("Qt4       " "${DO_QT4}")
ADM_DISPLAY("Cli       " "${DO_CLI}")

MESSAGE("*********************")
