########################################
# gettext
########################################
INCLUDE(admCheckGettext)

checkGettext()

SET(ADM_LOCALE "${CMAKE_INSTALL_PREFIX}/share/locale")
########################################
# SDL3
########################################
OPTION(SDL3 "" ON)

MESSAGE(STATUS "Checking for SDL3 ")
MESSAGE(STATUS "**************************************")

IF(SDL3 AND NOT MSVC)
  FIND_PACKAGE(PkgConfig)
  PKG_CHECK_MODULES(SDL3 sdl3)
  PRINT_LIBRARY_INFO("SDL3" SDL3_FOUND "${SDL3_INCLUDE_DIR}" "${SDL3_LIBRARY}")

  MARK_AS_ADVANCED(SDL3_INCLUDE_DIR)
  MARK_AS_ADVANCED(SDL3_LIBRARY)

  IF(SDL3_FOUND)
    SET(USE_SDL3 1)
  ENDIF()
ELSE()
  MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF()

APPEND_SUMMARY_LIST("Miscellaneous" "SDL3" "${USE_SDL3}")

MESSAGE("")


########################################
# SDL
######################### /tmp/logCmakebuildCore_debug###############
OPTION(SDL "" ON)

MESSAGE(STATUS "Checking for SDL>=2 (only for windows)")
MESSAGE(STATUS "**************************************")

IF(SDL AND WIN32 AND NOT MSVC)
  FIND_PACKAGE(SDL2)
  PRINT_LIBRARY_INFO("SDL2" SDL2_FOUND "${SDL2_INCLUDE_DIR}" "${SDL2_LIBRARY}")

  MARK_AS_ADVANCED(SDLMAIN_LIBRARY)
  MARK_AS_ADVANCED(SDL2_INCLUDE_DIR)
  MARK_AS_ADVANCED(SDL2_LIBRARY)

  IF(SDL2_FOUND)
    SET(USE_SDL 1)
  ENDIF()
ELSE()
  MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF()

APPEND_SUMMARY_LIST("Miscellaneous" "SDL" "${USE_SDL}")

MESSAGE("")

########################################
# XVideo
########################################
IF(UNIX AND NOT APPLE)
  OPTION(XVIDEO "" ON)

  IF(XVIDEO)
    MESSAGE(STATUS "Checking for XVideo")
    MESSAGE(STATUS "*******************")

    FIND_HEADER_AND_LIB(XVIDEO X11/extensions/Xvlib.h Xv XvShmPutImage)
    FIND_HEADER_AND_LIB(XEXT X11/extensions/XShm.h Xext XShmAttach)
    PRINT_LIBRARY_INFO("XVideo" XVIDEO_FOUND "${XVIDEO_INCLUDE_DIR}" "${XVIDEO_LIBRARY_DIR}")
    PRINT_LIBRARY_INFO("Xext" XEXT_FOUND "${XEXT_INCLUDE_DIR}" "${XEXT_LIBRARY_DIR}")

    IF(XVIDEO_FOUND AND XEXT_FOUND)
      SET(USE_XV 1)
    ENDIF()

    MESSAGE("")
  ENDIF()

  APPEND_SUMMARY_LIST("Miscellaneous" "XVideo" "${XVIDEO_FOUND}")
ELSE()
  SET(XVIDEO_CAPABLE FALSE)
ENDIF()

########################################
# Execinfo
########################################
MESSAGE(STATUS "Checking for execinfo")
MESSAGE(STATUS "*********************")

FIND_HEADER_AND_LIB(EXECINFO execinfo.h c backtrace_symbols)
PRINT_LIBRARY_INFO("execinfo" EXECINFO_FOUND "${EXECINFO_INCLUDE_DIR}" "${EXECINFO_LIBRARY_DIR}")

IF(EXECINFO_INCLUDE_DIR)
  # Try linking without -lexecinfo
  ADM_COMPILE(execinfo.cpp "" ${EXECINFO_INCLUDE_DIR} "" WITHOUT_LIBEXECINFO outputWithoutLibexecinfo)

  IF(WITHOUT_LIBEXECINFO)
    SET(EXECINFO_LIBRARY_DIR "")
    SET(HAVE_EXECINFO 1)

    MESSAGE(STATUS "execinfo not required")
  ELSE()
    ADM_COMPILE(execinfo.cpp "" ${EXECINFO_INCLUDE_DIR} ${EXECINFO_LIBRARY_DIR} WITH_LIBEXECINFO outputWithLibexecinfo)

    IF(WITH_LIBEXECINFO)
      SET(HAVE_EXECINFO 1)

      MESSAGE(STATUS "execinfo is required")
    ELSE()
      MESSAGE(STATUS "Does not work, without ${outputWithoutLibexecinfo}")
      MESSAGE(STATUS "Does not work, with ${outputWithLibexecinfo}")
    ENDIF()
  ENDIF()
ENDIF()

MESSAGE("")
