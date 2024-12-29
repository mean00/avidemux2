#
#
#
# #######################################
# pkg-config
# #######################################
MESSAGE(STATUS "Checking for pkg-config")
MESSAGE(STATUS "***********************")

FIND_PACKAGE(PkgConfig)

IF(${CMAKE_VERSION} VERSION_EQUAL 2.8.8)
  # workaround for bug in CMake 2.8.8 (http://www.cmake.org/Bug/view.php?id=13125)
  IF(DEFINED PKGCONFIG_FOUND)
    SET(PKG_CONFIG_FOUND ${PKGCONFIG_FOUND})
  ENDIF()
ENDIF()

IF(NOT PKG_CONFIG_FOUND)
  MESSAGE(FATAL_ERROR "Could not find pkg-config")
ENDIF()

MESSAGE(STATUS "Found pkg-config")

IF(VERBOSE)
  MESSAGE(STATUS "Path: ${PKG_CONFIG_EXECUTABLE}")
ENDIF()

MESSAGE("")


# #######################################
# pthreads
# #######################################
MESSAGE(STATUS "Checking for pthreads")
MESSAGE(STATUS "*********************")

include(admCheckThreads)
MESSAGE("")

# #######################################
# zlib
# #######################################
MESSAGE(STATUS "Checking for zlib")
MESSAGE(STATUS "*****************")

FIND_PACKAGE(ZLIB)
PRINT_LIBRARY_INFO("zlib" ZLIB_FOUND "${ZLIB_INCLUDE_DIR}" "${ZLIB_LIBRARY}" FATAL_ERROR)

MESSAGE("")
