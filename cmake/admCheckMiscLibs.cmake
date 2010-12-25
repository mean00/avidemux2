########################################
# gettext
########################################
INCLUDE(admCheckGettext)

if (NOT CROSS)
	checkGettext()
endif (NOT CROSS)

SET(ADM_LOCALE "${CMAKE_INSTALL_PREFIX}/share/locale")

########################################
# SDL
########################################
OPTION(SDL "" ON)

MESSAGE(STATUS "Checking for SDL")
MESSAGE(STATUS "****************")

IF (SDL)
	FIND_PACKAGE(SDL)
	PRINT_LIBRARY_INFO("SDL" SDL_FOUND "${SDL_INCLUDE_DIR}" "${SDL_LIBRARY}")

	MARK_AS_ADVANCED(SDLMAIN_LIBRARY)
	MARK_AS_ADVANCED(SDL_INCLUDE_DIR)
	MARK_AS_ADVANCED(SDL_LIBRARY)

	IF (SDL_FOUND)
		SET(USE_SDL 1)
	ENDIF (SDL_FOUND)
ELSE (SDL)
	MESSAGE("${MSG_DISABLE_OPTION}")
ENDIF (SDL)

MESSAGE("")

########################################
# XVideo
########################################
IF (UNIX AND NOT APPLE)
	OPTION(XVIDEO "" ON)

	IF (XVIDEO)
		MESSAGE(STATUS "Checking for XVideo")
		MESSAGE(STATUS "*******************")

		FIND_HEADER_AND_LIB(XVIDEO X11/extensions/Xvlib.h Xv XvShmPutImage)
		PRINT_LIBRARY_INFO("XVideo" XVIDEO_FOUND "${XVIDEO_INCLUDE_DIR}" "${XVIDEO_LIBRARY_DIR}")

		IF (XVIDEO_FOUND)
			SET(USE_XV 1)
		ENDIF (XVIDEO_FOUND)

		MESSAGE("")
	ENDIF (XVIDEO)
ELSE (UNIX AND NOT APPLE)
	SET(XVIDEO_CAPABLE FALSE)
ENDIF (UNIX AND NOT APPLE)

########################################
# SpiderMonkey
########################################
OPTION(USE_SYSTEM_SPIDERMONKEY "" OFF)

MESSAGE(STATUS "Checking for SpiderMonkey")
MESSAGE(STATUS "*************************")

IF (USE_SYSTEM_SPIDERMONKEY)
	FIND_HEADER_AND_LIB(SPIDERMONKEY jsapi.h js JS_InitStandardClasses)
	PRINT_LIBRARY_INFO("SpiderMonkey" SPIDERMONKEY_FOUND "${SPIDERMONKEY_INCLUDE_DIR}" "${SPIDERMONKEY_LIBRARY_DIR}" FATAL_ERROR)
ELSE (USE_SYSTEM_SPIDERMONKEY)
	MESSAGE("Skipping check and using bundled version.")
ENDIF (USE_SYSTEM_SPIDERMONKEY)

MESSAGE("")

########################################
# Execinfo
########################################
MESSAGE(STATUS "Checking for execinfo")
MESSAGE(STATUS "*********************")

FIND_PATH(LIBEXECINFO_H_DIR execinfo.h $ENV{CXXFLAGS})
MESSAGE(STATUS "libexecinfo Header Path: ${LIBEXECINFO_H_DIR}")

IF (LIBEXECINFO_H_DIR)
	FIND_LIBRARY(LIBEXECINFO_LIB_DIR execinfo $ENV{CXXFLAGS})
	MESSAGE(STATUS "libexecinfo Library Path: ${LIBEXECINFO_LIB_DIR}")

	# Try linking without -lexecinfo
	ADM_COMPILE(execinfo.cpp "" ${LIBEXECINFO_H_DIR} "" WITHOUT_LIBEXECINFO outputWithoutLibexecinfo)

	IF (WITHOUT_LIBEXECINFO)
		SET(HAVE_EXECINFO 1)
		MESSAGE(STATUS "OK, No lib needed (${ADM_EXECINFO_LIB})")
	ELSE (WITHOUT_LIBEXECINFO)
		ADM_COMPILE(execinfo.cpp "" ${LIBEXECINFO_H_DIR} ${LIBEXECINFO_LIB_DIR} WITH_LIBEXECINFO outputWithLibexecinfo)

		IF (WITH_LIBEXECINFO)
			SET(HAVE_EXECINFO 1)
			MESSAGE(STATUS "OK, libexecinfo needed")
		ELSE (WITH_LIBEXECINFO)
			MESSAGE(STATUS "Does not work, without ${outputWithoutLibexecinfo}")
			MESSAGE(STATUS "Does not work, with ${outputWithLibexecinfo}")
		ENDIF (WITH_LIBEXECINFO)
	ENDIF (WITHOUT_LIBEXECINFO)
ENDIF (LIBEXECINFO_H_DIR)

IF (HAVE_EXECINFO)
	SET(CMAKE_CLINK_FLAGS "${CFLAGS} -lexecinfo")
	SET(CMAKE_CXX_LINK_FLAGS "${CXXFLAGS} -lexecinfo")
ENDIF(HAVE_EXECINFO)

MESSAGE("")
