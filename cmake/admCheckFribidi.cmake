INCLUDE(admCheckFontConfig)
MACRO(checkFribidi minVersion)
  # # # # # # # # ################################
  # FRIBIDI
  # # # # # # # # ################################
  OPTION(FRIBIDI "" ON)
  checkFontConfig()
  MESSAGE(STATUS "Checking for Fribidi (want ${minVersion})")
  MESSAGE(STATUS "**************************************")

  IF(HAVE_FONTCONFIG)
    IF(FRIBIDI)
      PKG_CHECK_MODULES(FRIBIDI fribidi)
      PRINT_LIBRARY_INFO("Fribidi" FRIBIDI_FOUND "${FRIBIDI_CFLAGS}" "${FRIBIDI_LDFLAGS}")
      IF(FRIBIDI_FOUND)
        MESSAGE(STATUS "Fribidi version ${FRIBIDI_VERSION}")
        IF(("${FRIBIDI_VERSION}" VERSION_GREATER "${minVersion}.*") OR ("${FRIBIDI_VERSION}" VERSION_EQUAL "${minVersion}.*"))
          SET(USE_FRIBIDI 1)
          ADD_LIBRARY(adm_fribidi INTERFACE)
          TARGET_LINK_LIBRARIES(adm_fribidi INTERFACE ${FRIBIDI_LIBRARIES})
          TARGET_LINK_DIRECTORIES(adm_fribidi INTERFACE ${FRIBIDI_LIBRARY_DIRS})
          TARGET_INCLUDE_DIRECTORIES(adm_fribidi INTERFACE ${FRIBIDI_INCLUDE_DIRS})
          TARGET_COMPILE_OPTIONS(adm_fribidi INTERFACE ${FRIBIDI_CFLAGS})
        ELSE()
          MESSAGE(STATUS "Your version of fribidi is too old, we need at least ${minVersion}.*")
        ENDIF()
      ENDIF()
    ELSE()
      MESSAGE("${MSG_DISABLE_OPTION}")
    ENDIF()
  ELSE()
    MESSAGE(STATUS "No fontconfig => no fribidi...")
  ENDIF()

  APPEND_SUMMARY_LIST("Miscellaneous" "FRIBIDI" "${USE_FRIBIDI}")

  MESSAGE("")

ENDMACRO()
