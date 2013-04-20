include(admCheckFontConfig)
MACRO(checkFridibi minVersion)
########################################
# FRIDIBI
########################################
OPTION(FRIDIBI "" ON)
checkFontConfig()
MESSAGE(STATUS "Checking for Fridibi (want ${minVersion})")
MESSAGE(STATUS "**************************************")

IF(HAVE_FONTCONFIG)

        IF (FRIDIBI)
	        PKG_CHECK_MODULES(FRIDIBI fribidi)
	        PRINT_LIBRARY_INFO("Fribidi" FRIDIBI_FOUND "${FRIDIBI_CFLAGS}" "${FRIDIBI_LDFLAGS}")
	        IF (FRIDIBI_FOUND)
		        # Warning does not work if we ask 0.19 and get 0.20
		        MESSAGE(STATUS "Fridibi version ${FRIDIBI_VERSION}")
		        IF("${FRIDIBI_VERSION}" MATCHES "${minVersion}.*")
			        SET(USE_FRIDIBI 1)
		        ELSE("${FRIDIBI_VERSION}" MATCHES "${minVersion}.*")
			        MESSAGE(STATUS "Your version of cmake is too old, we need  ${minVersion}.*")
		        ENDIF("${FRIDIBI_VERSION}" MATCHES "${minVersion}.*")
	        ENDIF (FRIDIBI_FOUND)
        ELSE (FRIDIBI)
	        MESSAGE("${MSG_DISABLE_OPTION}")
        ENDIF (FRIDIBI)
ELSE(HAVE_FONTCONFIG)
        MESSAGE(STATUS "No fontconfig=> no fribidi....")
ENDIF(HAVE_FONTCONFIG)

APPEND_SUMMARY_LIST("Miscellaneous" "FRIBIDI" "${FRIDIBI_FOUND}")

MESSAGE("")

ENDMACRO(checkFridibi)
