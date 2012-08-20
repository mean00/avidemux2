###################################################
MESSAGE("#####################################")
MESSAGE("Custom videoFilter plugin template Started")
MESSAGE("#####################################")
MESSAGE("")

SET(ADM_PROJECT AdmLogoVideoFilter)
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
LINK_DIRECTORIES("${AVIDEMUX_SEARCH_LIB_DIR}")
INCLUDE_DIRECTORIES(${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/2.6/)

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
INCLUDE(vf_plugin)


SET(ADM_vf_logo_SRCS ADM_vidLogo.cpp)

ADD_VIDEO_FILTER(ADM_vf_logo ${ADM_vf_logo_SRCS})
TARGET_LINK_LIBRARIES(ADM_vf_logo ADM_coreImageLoader6 )
INIT_VIDEO_FILTER(ADM_vf_logo)
INSTALL_VIDEO_FILTER(ADM_vf_logo)
