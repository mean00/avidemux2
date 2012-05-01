MACRO(checkFridibi)
########################################
# FRIDIBI
########################################
OPTION(FRIDIBI "" ON)

MESSAGE(STATUS "Checking for Fridibi")
MESSAGE(STATUS "********************")

IF (FRIDIBI)
	PKG_CHECK_MODULES(FRIDIBI fribidi)
	PRINT_LIBRARY_INFO("Fribidi" FRIDIBI_FOUND "${FRIDIBI_CFLAGS}" "${FRIDIBI_LDFLAGS}")

	IF (FRIDIBI_FOUND)
		SET(USE_FRIDIBI 1)
	ENDIF (FRIDIBI_FOUND)
ELSE (FRIDIBI)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (FRIDIBI)

APPEND_SUMMARY_LIST("Miscellaneous" "FRIBIDI" "${FRIDIBI_FOUND}")

MESSAGE("")

ENDMACRO(checkFridibi)
