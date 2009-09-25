MESSAGE("#####################################")
MESSAGE("Plugins Configure Started")
MESSAGE("#####################################")
MESSAGE("")

SET(ADM_PROJECT AdmPlugins)
SET(PLUGINS TRUE)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
########################################
# Where is the top dir ?
########################################
IF (NOT AVIDEMUX_SOURCE_DIR)
	MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_SOURCE_DIR=path_to_avidemux_source")
ELSE (NOT AVIDEMUX_TOP_SOURCE_DIR)
        # We need TOP_SOURCE_DIR 
	SET(AVIDEMUX_TOP_SOURCE_DIR "${AVIDEMUX_SOURCE_DIR}" CACHE STRING "")
ENDIF (NOT AVIDEMUX_SOURCE_DIR)

# Common definitions...
SET(CMAKE_MODULE_PATH "${AVIDEMUX_TOP_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
include(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/commonCmakeApplication.cmake)

########################################
# Add include dirs
########################################
SET(AVIDEMUX_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
MARK_AS_ADVANCED(AVIDEMUX_INSTALL_DIR)
include(admInstallDir)
#########################################
# Get regular stuff from main app
#########################################
include(admCoreIncludes)
LINK_DIRECTORIES("${AVIDEMUX_LIB_DIR}")
INCLUDE_DIRECTORIES(${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/)

IF (FRESH_BUILD)
	MESSAGE("")
ENDIF (FRESH_BUILD)

########################################
# Global options
########################################
OPTION(VERBOSE "" OFF)

########################################
# Check for libraries
########################################

INCLUDE(admCheckGtk)
INCLUDE(admCheckGlade)
checkGtk()
checkGlade()

INCLUDE(admCheckQt4)
checkQt4()

########################################
# Actual work to do..
########################################
ADD_SUBDIRECTORY(ADM_audioDecoders)
ADD_SUBDIRECTORY(ADM_audioDevices)
ADD_SUBDIRECTORY(ADM_videoEncoder)
ADD_SUBDIRECTORY(ADM_videoFilters6)
ADD_SUBDIRECTORY(ADM_audioEncoders)
ADD_SUBDIRECTORY(ADM_demuxers)
ADD_SUBDIRECTORY(ADM_muxers)

########################################
# Config Summary
########################################
INCLUDE(admPluginConfigSummary)
MESSAGE("")
include(./pluginsPackageDebian.cmake)
