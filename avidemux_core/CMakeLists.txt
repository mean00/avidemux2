cmake_minimum_required(VERSION 2.6)

MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for avidemux core libs Started")
MESSAGE("#########################################")
MESSAGE("")

# need for shared cmake script between avidemux & plugins
SET(AVIDEMUX_SOURCE_DIR ${CMAKE_SOURCE_DIR})
SET(AVIDEMUX_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)
SET(AVIDEMUX_CORE_BUILD_DIR ${CMAKE_BINARY_DIR})

SET(CMAKE_MODULE_PATH "${AVIDEMUX_TOP_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
SET(ADM_PROJECT admCore)

IF (WIN32)
	SET(ADM_PLUGIN_DIR "plugins")
ELSE (WIN32)
	SET(ADM_PLUGIN_DIR "ADM_plugins6")
ENDIF (WIN32)

ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)

include(admConfigSummary)
INITIALISE_SUMMARY_LISTS()

include(admMainChecks)
include(admCheckFtello)
include(admAsNeeded)

#########################################
# Debug Summary
########################################
IF (VERBOSE)
	INCLUDE(adm_log)
	MESSAGE("")
ENDIF (VERBOSE)

########################################
# Scripting Engine Options (before generating config header)
########################################
OPTION(SPIDERMONKEY "" ON)

if (SPIDERMONKEY)
	set(USE_SPIDERMONKEY 1)
endif (SPIDERMONKEY)

APPEND_SUMMARY_LIST("Scripting" "SpiderMonkey" "${USE_SPIDERMONKEY}")

OPTION(TINYPY "" ON)

if (TINYPY)
	set(USE_TINYPY 1)
endif (TINYPY)

APPEND_SUMMARY_LIST("Scripting" "TinyPy" "${USE_TINYPY}")

########################################
# Checking if we should use ftello
# or ftello64 (win32)
########################################
checkFtello()

########################################
# Generate config.h
########################################
CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/ADM_coreConfig.h.cmake" "${CMAKE_BINARY_DIR}/config/ADM_coreConfig.h")
include_directories( "${CMAKE_BINARY_DIR}/config")
MESSAGE(STATUS "ADM_coreConfig.h generated")
MESSAGE("")

########################################
# Add include dirs
########################################
set(EXCLUDE_ADM_INCLUDE TRUE)
include(admCoreIncludes)

########################################
# Build FFmpeg
########################################
include(admFFmpegBuild)

########################################
# Directories to build
########################################
ADD_SUBDIRECTORY(ADM_core)
ADD_SUBDIRECTORY(ADM_coreAudioParser)
ADD_SUBDIRECTORY(ADM_coreAudio)
ADD_SUBDIRECTORY(ADM_coreAudioFilter)
ADD_SUBDIRECTORY(ADM_coreAudioDevice)
ADD_SUBDIRECTORY(ADM_coreAudioEncoder)
ADD_SUBDIRECTORY(ADM_coreDemuxer)
ADD_SUBDIRECTORY(ADM_coreDemuxerMpeg)
ADD_SUBDIRECTORY(ADM_coreVideoCodec)
ADD_SUBDIRECTORY(ADM_coreImage)
ADD_SUBDIRECTORY(ADM_coreMuxer)
ADD_SUBDIRECTORY(ADM_coreUI)
ADD_SUBDIRECTORY(ADM_coreUtils)
ADD_SUBDIRECTORY(ADM_coreVideoEncoder)
ADD_SUBDIRECTORY(ADM_coreVideoFilter)
ADD_SUBDIRECTORY(ADM_coreSqlLight3)
ADD_SUBDIRECTORY(ADM_coreJobs)
ADD_SUBDIRECTORY(ADM_coreSocket)
add_subdirectory(ADM_coreImageLoader)
add_subdirectory(ADM_coreScript)

if (SPIDERMONKEY)
	add_subdirectory(ADM_coreScriptSpiderMonkey)
endif (SPIDERMONKEY)

if (TINYPY)
	add_subdirectory(ADM_coreScriptPython)
endif (TINYPY)

if (USE_VDPAU)
	ADD_SUBDIRECTORY(ADM_coreVdpau)
endif (USE_VDPAU)

########################################
# Config Summary
########################################
DISPLAY_SUMMARY_LIST()

include(admPackager)
admPackager(corePackage)
IF(USE_VDPAU)
        FILE(WRITE  ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.cmake "SET(USE_VDPAU 1)")
ELSE(USE_VDPAU)
        FILE(WRITE  ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.cmake "")
ENDIF(USE_VDPAU)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6")
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.cmake DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6")
MESSAGE("")
