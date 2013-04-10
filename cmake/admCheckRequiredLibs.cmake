#
#
#
########################################
# pkg-config
########################################
MESSAGE(STATUS "Checking for pkg-config")
MESSAGE(STATUS "***********************")

FIND_PACKAGE(PkgConfig)

if (${CMAKE_VERSION} VERSION_EQUAL 2.8.8)
	# workaround for bug in CMake 2.8.8 (http://www.cmake.org/Bug/view.php?id=13125)
        if (DEFINED PKGCONFIG_FOUND)
          set(PKG_CONFIG_FOUND ${PKGCONFIG_FOUND})
       endif (DEFINED PKGCONFIG_FOUND)
endif (${CMAKE_VERSION} VERSION_EQUAL 2.8.8)

IF (NOT PKG_CONFIG_FOUND)
	MESSAGE(FATAL_ERROR "Could not find pkg-config")
ENDIF (NOT PKG_CONFIG_FOUND)

MESSAGE(STATUS "Found pkg-config")

IF (VERBOSE)
	MESSAGE(STATUS "Path: ${PKG_CONFIG_EXECUTABLE}")
ENDIF (VERBOSE)

MESSAGE("")


########################################
# pthreads
########################################
MESSAGE(STATUS "Checking for pthreads")
MESSAGE(STATUS "*********************")

FIND_PACKAGE(Threads)
if(CROSS)
	IF(NOT APPLE)
			  IF(X86_64_SUPPORTED)
                          	SET(PTHREAD_LIBRARIES "-lpthreadGC2-w64")
			  #ELSE(X86_64_SUPPORTED)
                          	#SET(PTHREAD_LIBRARIES "-lpthreadGC2")
			  ENDIF(X86_64_SUPPORTED)
			  MESSAGE(STATUS "Cross compile override using ${PTHREAD_LIBRARIES} hardcoded")
			  MESSAGE(STATUS "INCLUDE=<${PTHREAD_INCLUDE_DIR}>, LIB=<${PTHREAD_LIBRARIES}>")
	ENDIF(NOT APPLE)
			  SET(PTHREAD_FOUND TRUE)
endif(CROSS)

PRINT_LIBRARY_INFO("pthreads" PTHREAD_FOUND "${PTHREAD_INCLUDE_DIR}" "${PTHREAD_LIBRARIES}" FATAL_ERROR)

MESSAGE("")

########################################
# zlib
########################################
MESSAGE(STATUS "Checking for zlib")
MESSAGE(STATUS "*****************")

FIND_PACKAGE(ZLIB)
PRINT_LIBRARY_INFO("zlib" ZLIB_FOUND "${ZLIB_INCLUDE_DIR}" "${ZLIB_LIBRARY}" FATAL_ERROR)

MESSAGE("")
