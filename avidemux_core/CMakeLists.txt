cmake_minimum_required(VERSION 2.6)

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

ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)
include(admMainChecks)
include(admAsNeeded)
########################################
# VDPAU
########################################
IF(NOT CROSS)
	checkVDPAU()
ENDIF(NOT CROSS)

#########################################
# Debug Summary
########################################
IF (VERBOSE)
	INCLUDE(adm_log)
	MESSAGE("")
ENDIF (VERBOSE)

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
ADD_SUBDIRECTORY(ADM_coreTinyPy)
ADD_SUBDIRECTORY(ADM_coreMuxer)
ADD_SUBDIRECTORY(ADM_coreUI)
ADD_SUBDIRECTORY(ADM_coreUtils)
ADD_SUBDIRECTORY(ADM_coreVideoEncoder)
ADD_SUBDIRECTORY(ADM_coreVideoFilter)
ADD_SUBDIRECTORY(ADM_coreJobs)
ADD_SUBDIRECTORY(ADM_coreSocket)

if (NOT USE_SYSTEM_SPIDERMONKEY)
	ADD_SUBDIRECTORY(ADM_smjs)
endif (NOT USE_SYSTEM_SPIDERMONKEY)

ADD_SUBDIRECTORY(ADM_coreImageLoader)

if (USE_VDPAU)
	ADD_SUBDIRECTORY(ADM_coreVdpau)
endif (USE_VDPAU)

########################################
# Config Summary
########################################
INCLUDE(admConfigSummary)
#
# Packaging
#
include(admPackager)
admPackager(corePackage)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6") 
MESSAGE("")
