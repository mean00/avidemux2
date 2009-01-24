########################################
# pkg-config
########################################
MESSAGE(STATUS "Checking for pkg-config")
MESSAGE(STATUS "***********************")

FIND_PACKAGE(PkgConfig)

IF (NOT PKG_CONFIG_FOUND)
	MESSAGE(FATAL_ERROR "Could not find pkg-config")
ENDIF (NOT PKG_CONFIG_FOUND)

MESSAGE(STATUS "Found pkg-config")

IF (VERBOSE)
	MESSAGE(STATUS "Path: ${PKG_CONFIG_EXECUTABLE}")
ENDIF (VERBOSE)

MESSAGE("")

########################################
# Libxml2
########################################
INCLUDE(admCheckLibxml2)

checkLibxml2()

IF (LIBXML2_FOUND)
	SET(USE_LIBXML2 1)
ELSEIF (LIBXML2_FOUND)
	MESSAGE(${ARGV4} "Could not find Libxml2")
ENDIF (LIBXML2_FOUND)

########################################
# libpng
########################################
MESSAGE(STATUS "Checking for libpng")
MESSAGE(STATUS "*******************")

FIND_PACKAGE(PNG)
PRINT_LIBRARY_INFO("libpng" PNG_FOUND "${PNG_INCLUDE_DIR} ${PNG_DEFINITIONS}" "${PNG_LIBRARIES}" FATAL_ERROR)

FOREACH(_flag ${PNG_INCLUDE_DIR})
	SET(PNG_CFLAGS ${PNG_CFLAGS} -I${_flag})
ENDFOREACH(_flag)

SET(PNG_CFLAGS ${PNG_CFLAGS} ${PNG_DEFINITIONS})
SET(USE_PNG 1)
MESSAGE("")

########################################
# pthreads
########################################
MESSAGE(STATUS "Checking for pthreads")
MESSAGE(STATUS "*********************")

FIND_PACKAGE(Threads)
PRINT_LIBRARY_INFO("pthreads" PTHREAD_FOUND "${PTHREAD_INCLUDE_DIR}" "${PTHREAD_LIBRARIES}" FATAL_ERROR)

MESSAGE("")