include(admCheckFontConfig)
MACRO(checkFribidi minVersion)
########################################
# FRIBIDI
########################################
OPTION(FRIBIDI "" ON)
checkFontConfig()
MESSAGE(STATUS "Checking for Fribidi (want ${minVersion})")
MESSAGE(STATUS "**************************************")

IF(HAVE_FONTCONFIG)
    IF (FRIBIDI)
        PKG_CHECK_MODULES(FRIBIDI fribidi)
        PRINT_LIBRARY_INFO("Fribidi" FRIBIDI_FOUND "${FRIBIDI_CFLAGS}" "${FRIBIDI_LDFLAGS}")
        IF (FRIBIDI_FOUND)
            MESSAGE(STATUS "Fribidi version ${FRIBIDI_VERSION}")
                IF(("${FRIBIDI_VERSION}" VERSION_GREATER "${minVersion}.*") OR ("${FRIBIDI_VERSION}" VERSION_EQUAL "${minVersion}.*"))
                    SET(USE_FRIBIDI 1)
                ELSE(("${FRIBIDI_VERSION}" VERSION_GREATER "${minVersion}.*") OR ("${FRIBIDI_VERSION}" VERSION_EQUAL "${minVersion}.*"))
                    MESSAGE(STATUS "Your version of fribidi is too old, we need at least ${minVersion}.*")
                ENDIF(("${FRIBIDI_VERSION}" VERSION_GREATER "${minVersion}.*") OR ("${FRIBIDI_VERSION}" VERSION_EQUAL "${minVersion}.*"))
        ENDIF (FRIBIDI_FOUND)
    ELSE (FRIBIDI)
        MESSAGE("${MSG_DISABLE_OPTION}")
    ENDIF (FRIBIDI)
ELSE(HAVE_FONTCONFIG)
    MESSAGE(STATUS "No fontconfig => no fribidi...")
ENDIF(HAVE_FONTCONFIG)

APPEND_SUMMARY_LIST("Miscellaneous" "FRIBIDI" "${USE_FRIBIDI}")

MESSAGE("")

ENDMACRO(checkFribidi)
