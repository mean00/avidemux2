MESSAGE("#####################################")
MESSAGE("Plugins Configure Started")
MESSAGE("#####################################")
MESSAGE("")

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)

if (COMMAND cmake_policy)
	cmake_policy(VERSION 2.4)
	cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

IF (NOT CMAKE_CXX_COMPILER_WORKS MATCHES "^${CMAKE_CXX_COMPILER_WORKS}$")
	SET(FRESH_BUILD 1)
ENDIF (NOT CMAKE_CXX_COMPILER_WORKS MATCHES "^${CMAKE_CXX_COMPILER_WORKS}$")

PROJECT(AdmPlugins)

IF (${AdmPlugins_SOURCE_DIR} MATCHES ${AdmPlugins_BINARY_DIR})
	MESSAGE("Please do an out-of-tree build:")
	MESSAGE("rm CMakeCache.txt; mkdir build; cd build; cmake ..; make")
	MESSAGE(FATAL_ERROR "in-tree-build detected")
ENDIF (${AdmPlugins_SOURCE_DIR} MATCHES ${AdmPlugins_BINARY_DIR})

########################################
# Where is the top dir
########################################
IF (NOT AVIDEMUX_SOURCE_DIR)
	MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_SOURCE_DIR=path_to_avidemux_source")
ELSE (NOT AVIDEMUX_SOURCE_DIR)
	SET(AVIDEMUX_SOURCE_DIR "${AVIDEMUX_SOURCE_DIR}" CACHE STRING "")
ENDIF (NOT AVIDEMUX_SOURCE_DIR)

INCLUDE_DIRECTORIES(${AVIDEMUX_SOURCE_DIR})

########################################
# Where are the libraries
########################################
IF(NOT AVIDEMUX_LIB_DIR)
	MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_LIB_DIR=path_to_avidemux_libs")
ELSE (NOT AVIDEMUX_LIB_DIR)
	SET(AVIDEMUX_LIB_DIR "${AVIDEMUX_LIB_DIR}" CACHE STRING "")
ENDIF(NOT AVIDEMUX_LIB_DIR)

LINK_DIRECTORIES(${AVIDEMUX_LIB_DIR})

########################################
# Where is coreConfig ?
########################################
IF(NOT AVIDEMUX_CORECONFIG_DIR)
	MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_CORECONFIG_DIR=path_to_ADM_coreConfig.h")
ELSE (NOT AVIDEMUX_CORECONFIG_DIR)
	SET(AVIDEMUX_CORECONFIG_DIR "${AVIDEMUX_CORECONFIG_DIR}" CACHE STRING "")
ENDIF(NOT AVIDEMUX_CORECONFIG_DIR)

INCLUDE_DIRECTORIES(${AVIDEMUX_CORECONFIG_DIR})

SET(CMAKE_MODULE_PATH "${AVIDEMUX_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

include(admConfigHelper)

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

IF (CYGWIN)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwin32")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwin32")
ENDIF (CYGWIN)

IF (ADM_CPU_ALTIVEC)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADM_ALTIVEC_FLAGS}")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADM_ALTIVEC_FLAGS}")
ENDIF (ADM_CPU_ALTIVEC)

IF (WIN32)
	SET(BIN_DIR .)
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-enable-auto-image-base -Wl,-enable-auto-import")

	IF (CMAKE_BUILD_TYPE STREQUAL "Release")
		SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")
	ENDIF (CMAKE_BUILD_TYPE STREQUAL "Release")
ELSE (WIN32)
	SET(BIN_DIR bin)
ENDIF (WIN32)

########################################
# Standard Avidemux defines
########################################
SET(VERSION 2.5.0)
SET(ADM_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(ADM_DEBUG 1)
ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")

########################################
# Check for libraries
########################################
SET(MSG_DISABLE_OPTION "Disabled per request")

INCLUDE(admCheckRequiredLibs)

INCLUDE(admCheckGtk)
checkGtk()

INCLUDE(admCheckQt4)
checkQt4()

INCLUDE(admCheckMiscLibs)

########################################
# Actual work to do..
########################################
ADD_SUBDIRECTORY(ADM_audioDecoders)
ADD_SUBDIRECTORY(ADM_audioDevices)
ADD_SUBDIRECTORY(ADM_videoEncoder)
#ADD_SUBDIRECTORY(ADM_videoFilters)
ADD_SUBDIRECTORY(ADM_audioEncoders)
ADD_SUBDIRECTORY(ADM_demuxers)
ADD_SUBDIRECTORY(ADM_muxers)

########################################
# Config Summary
########################################
INCLUDE(admPluginConfigSummary)
MESSAGE("")
