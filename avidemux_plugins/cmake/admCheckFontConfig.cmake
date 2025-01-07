MACRO(checkFontConfig)
  IF(NOT FONTCONFIG_CHECKED)
    OPTION(FONTCONFIG "" ON)

    MESSAGE(STATUS "Checking for FontConfig")
    MESSAGE(STATUS "***********************")

    IF(FONTCONFIG)
      FIND_PACKAGE(Fontconfig)
      PRINT_LIBRARY_INFO("FontConfig" Fontconfig_FOUND "${Fontconfig_INCLUDE_DIRS}" "${Fontconfig_LIBRARIES}")

      IF(Fontconfig_FOUND)
        SET(HAVE_FONTCONFIG 1)
        ADD_LIBRARY(adm_fontconfig INTERFACE)
        TARGET_INCLUDE_DIRECTORIES(adm_fontconfig INTERFACE ${Fontconfig_INCLUDE_DIRS})
        TARGET_LINK_LIBRARIES(adm_fontconfig INTERFACE ${Fontconfig_LIBRARIES})
        TARGET_COMPILE_OPTIONS(adm_fontconfig INTERFACE ${Fontconfig_COMPILE_OPTIONS})
      ENDIF()
    ELSE()
      MESSAGE("${MSG_DISABLE_OPTION}")
    ENDIF()

    SET(FONTCONFIG_CHECKED 1)

    MESSAGE("")
  ENDIF()

  APPEND_SUMMARY_LIST("Miscellaneous" "FontConfig" "${HAVE_FONTCONFIG}")
ENDMACRO()
