
# Override...
SET(CMAKE_C_FLAGS $ENV{CFLAGS})
SET(CMAKE_CXX_FLAGS $ENV{CXXFLAGS})
SET(CMAKE_LD_FLAGS $ENV{LDFLAGS})

IF (NOT CMAKE_CXX_COMPILER_WORKS MATCHES "^${CMAKE_CXX_COMPILER_WORKS}$")
	SET(FRESH_BUILD 1)
ENDIF (NOT CMAKE_CXX_COMPILER_WORKS MATCHES "^${CMAKE_CXX_COMPILER_WORKS}$")
#
#
include(admCrossCompile)
MESSAGE(STATUS "[BUILD] EXTRA Cflags:${CMAKE_C_FLAGS}")
MESSAGE(STATUS "[BUILD] EXTRA CXXflags:${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "[BUILD] EXTRA LDflags:${CMAKE_LD_FLAGS}")
MESSAGE(STATUS "[BUILD] Compiler ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS "[BUILD] Linker   ${CMAKE_LINKER}")
#
MESSAGE(STATUS "Top Source dir is ${AVIDEMUX_TOP_SOURCE_DIR}")
MESSAGE("")

PROJECT(${ADM_PROJECT})

IF (${Avidemux_SOURCE_DIR} MATCHES ${Avidemux_BINARY_DIR})
	MESSAGE("Please do an out-of-tree build:")
	MESSAGE("rm CMakeCache.txt; mkdir build; cd build; cmake ..; make")
	MESSAGE(FATAL_ERROR "in-tree-build detected")
ENDIF (${Avidemux_SOURCE_DIR} MATCHES ${Avidemux_BINARY_DIR})

if (${CMAKE_VERSION} VERSION_GREATER 2.8.5)
	include(GenerateExportHeader)
else (${CMAKE_VERSION} VERSION_GREATER 2.8.5)
	include(_GenerateExportHeader)
endif (${CMAKE_VERSION} VERSION_GREATER 2.8.5)

INCLUDE(admConfigHelper)
include(admGetRevision)

IF (FRESH_BUILD)
	MESSAGE("")
ENDIF (FRESH_BUILD)

########################################
# Global options
########################################
OPTION(VERBOSE "" OFF)

IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF (NOT CMAKE_BUILD_TYPE)

########################################
# Avidemux system specific tweaks
########################################
INCLUDE(admDetermineSystem)

# Address sanitizer only works with llvm/clang
IF(${CMAKE_CXX_COMPILER} MATCHES ".*[cC]lang.*")
        IF (ASAN)
                MESSAGE(STATUS "Address Sanitizer activated")
	        SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
	        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
                SET(CMAKE_LD_FLAGS  "${CMAKE_LD_FLAGS}  -fsanitize=address ")
        ELSE (ASAN)
                MESSAGE(STATUS "Address Sanitizer not activated")
	        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
        ENDIF (ASAN)
ENDIF(${CMAKE_CXX_COMPILER} MATCHES ".*[cC]lang.*")

IF (ADM_CPU_ALTIVEC)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADM_ALTIVEC_FLAGS}")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADM_ALTIVEC_FLAGS}")
ENDIF (ADM_CPU_ALTIVEC)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	# jog shuttle is only available on Linux due to its interface
	SET(USE_JOG 1)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

IF (WIN32)
	SET(BIN_DIR .)

	IF (MINGW)
		SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-enable-auto-image-base -Wl,-enable-auto-import")
		SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-enable-auto-import")

		IF (CMAKE_BUILD_TYPE STREQUAL "Release")
			SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-s")
			SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")
		ENDIF (CMAKE_BUILD_TYPE STREQUAL "Release")
	ENDIF (MINGW)
ELSE (WIN32)
	SET(BIN_DIR bin)
ENDIF (WIN32)

if (CMAKE_COMPILER_IS_GNUCC)
	add_definitions("-Werror=attributes") 
endif (CMAKE_COMPILER_IS_GNUCC)
# Clang
IF(${CMAKE_CXX_COMPILER} MATCHES ".*[cC]lang.*")
        add_definitions(-Qunused-arguments)
ENDIF()
# MacOsX stuff
IF(APPLE)
	set (CMAKE_OSX_ARCHITECTURES "x86_64")
        SET(CMAKE_INSTALL_NAME_DIR @executable_path/../lib) # Make sure we are relative to bundle
ENDIF(APPLE)

########################################
# Standard Avidemux defines
########################################
# Define internal flags for GTK+ and Qt4 builds.  These are turned off
# if a showstopper is found.  CLI is automatically assumed as possible
# since it uses the minimum set of required libraries and CMake will 
# fail if these aren't met.

SET(AVIDEMUX_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
MARK_AS_ADVANCED(AVIDEMUX_INSTALL_DIR)
include(admInstallDir)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(ADM_DEBUG 1)
ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")
########################################
# Subversion
########################################

MESSAGE("")
MESSAGE(STATUS "Checking for SCM")
MESSAGE(STATUS "****************")
IF(RELEASE OR AVIDEMUX_EXTERNAL_BUILD)
        SET( ADM_SUBVERSION 0)
ELSE()
        admGetRevision( ${CMAKE_SOURCE_DIR} ADM_SUBVERSION)
ENDIF()
MESSAGE("")
include(avidemuxVersion)


########################################
# Check for libraries
########################################
SET(MSG_DISABLE_OPTION "Disabled per request")
INCLUDE(admCheckRequiredLibs)
IF(NOT PLUGINS)
INCLUDE(admCheckMiscLibs)
INCLUDE(FindThreads)
INCLUDE(admCheckVDPAU)
IF(WIN32)
INCLUDE(admCheckDxva2)
ENDIF(WIN32)

INCLUDE(admCheckNvEnc)
#INCLUDE(admCheckXvba)
INCLUDE(admCheckLibVA)
checkVDPAU()
#checkXvba()
checkLibVA()
checkNvEnc()
ENDIF(NOT PLUGINS)

########################################
# Check functions and includes
########################################
IF (NOT SYSTEM_HEADERS_CHECKED)
	MESSAGE(STATUS "Checking system headers")
	MESSAGE(STATUS "***********************")

	INCLUDE(CheckIncludeFiles)
	INCLUDE(CheckFunctionExists)

	if (MSVC)
		include_directories("${AVIDEMUX_TOP_SOURCE_DIR}/foreignBuilds/msvc/include")
		set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} "${AVIDEMUX_TOP_SOURCE_DIR}/foreignBuilds/msvc/include")
	endif (MSVC)

	set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${CMAKE_INCLUDE_PATH})
	CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)

	CHECK_INCLUDE_FILES(inttypes.h   HAVE_INTTYPES_H)	# internal use, mpeg2enc, mplex
	CHECK_INCLUDE_FILES(stdint.h     HAVE_STDINT_H)		# internal use, mpeg2enc, mplex
	SET(SYSTEM_HEADERS_CHECKED 1 CACHE BOOL "")
	MARK_AS_ADVANCED(SYSTEM_HEADERS_CHECKED)

	MESSAGE("")
ENDIF (NOT SYSTEM_HEADERS_CHECKED)

