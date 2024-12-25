MACRO(checkFreeType)
  # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # ##
  # FreeType2
  # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # ##
  OPTION(FREETYPE2 "" ON)

  MESSAGE(STATUS "Checking for FreeType2")
  MESSAGE(STATUS "**********************")

  IF(FREETYPE2)
    FIND_PACKAGE(Freetype)
    IF(FREETYPE_FOUND)
      SET(USE_FREETYPE 1)
      PRINT_LIBRARY_INFO("FreeType2" FREETYPE_FOUND ":${FREETYPE_INCLUDE_DIRS}" ",${FREETYPE_LIBRARIES}")
      ADD_LIBRARY(adm_freetype INTERFACE)
      TARGET_INCLUDE_DIRECTORIES(adm_freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
      TARGET_LINK_LIBRARIES(adm_freetype INTERFACE ${FREETYPE_LIBRARIES})
    ENDIF()
  ELSE()
    MESSAGE("${MSG_DISABLE_OPTION}")
  ENDIF()
  APPEND_SUMMARY_LIST("Miscellaneous" "Freetype" "${USE_FREETYPE}")

  MESSAGE("")

  # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # ##
  # libiconv
  # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # ##
  IF(USE_FREETYPE)
    MESSAGE(STATUS "Checking for libiconv")
    MESSAGE(STATUS "*********************")
    FIND_PACKAGE(Iconv)
    IF(ICONV_FOUND)
      MESSAGE(STATUS "  Iconv found")
    ELSE()
      MESSAGE(STATUS "iconv not found, disabling FreeType2")
      SET(USE_FREETYPE)
    ENDIF()

    MESSAGE("")
  ENDIF()
ENDMACRO()
