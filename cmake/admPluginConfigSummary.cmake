# ARGV2 - capable flag
include(admSummary)


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
