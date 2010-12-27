###################################################
#
#
###################################################
MESSAGE("#####################################")
MESSAGE("Custom demuxer plugin template Started")
MESSAGE("#####################################")
MESSAGE("")

SET(ADM_PROJECT AdmAsfDemuxer)
SET(PLUGINS TRUE)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
########################################
# Where is the top dir ?
########################################

MESSAGE(STATUS "Checking for AVIDEMUX_SOURCE_DIR, AVIDEMUX_LIB_DIR, AVIDEMUX_INCLUDE_DIR...")
IF (NOT AVIDEMUX_SOURCE_DIR)
	MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_SOURCE_DIR=path_to_avidemux_source. You just need the source tree.")
ELSE (NOT AVIDEMUX_TOP_SOURCE_DIR)
        # We need TOP_SOURCE_DIR 
	SET(AVIDEMUX_TOP_SOURCE_DIR "${AVIDEMUX_SOURCE_DIR}" CACHE STRING "")
ENDIF (NOT AVIDEMUX_SOURCE_DIR)
#IF (NOT AVIDEMUX_LIB_DIR)
	#MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_LIB_DIR=path_to_avidemux_lib. It is probably /usr/lib or /usr/lib64.")
#ELSE (NOT AVIDEMUX_LIB_DIR)
        ## We need AVIDEMUX_LIB_DIR 
	#SET(AVIDEMUX_LIB_DIR "${AVIDEMUX_LIB_DIR}" CACHE STRING "")
#ENDIF (NOT AVIDEMUX_LIB_DIR)
#IF (NOT AVIDEMUX_INCLUDE_DIR)
	#MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_INCLUDE_DIR=path_to_avidemux_lib. It is probably /usr/include .")
#ELSE (NOT AVIDEMUX_INCLUDE_DIR)
        ## We need AVIDEMUX_INCLUDE_DIR 
	#SET(AVIDEMUX_INCLUDE_DIR "${AVIDEMUX_INCLUDE_DIR}" CACHE STRING "")
#ENDIF (NOT AVIDEMUX_INCLUDE_DIR)




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
################################################
# Now really define our plugin...
###############################################

include(dm_plugin)
SET(ADM_wtv_SRCS 
	ADM_wtvAudio.cpp
        ADM_wtv.cpp  
        ADM_wtvPlugin.cpp
)

ADD_LIBRARY(ADM_dm_wtv SHARED ${ADM_wtv_SRCS})

INIT_DEMUXER(ADM_dm_wtv)
INSTALL_DEMUXER(ADM_dm_wtv)
