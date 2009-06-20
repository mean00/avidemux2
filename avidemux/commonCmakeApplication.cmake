
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)

if (COMMAND cmake_policy)
	cmake_policy(VERSION 2.4)
	cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)
########################################
# Definitions and Includes
########################################
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)
ADD_DEFINITIONS(-DHAVE_CONFIG_H)
########################################
# Where is the config.h dir
########################################
#
# Access to our cmake lib
#
SET(AVIDEMUX_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../..)
SET(CMAKE_MODULE_PATH "${AVIDEMUX_TOP_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
MESSAGE(STATUS "Avidemux cmake scripts are in ${CMAKE_MODULE_PATH}")
#
#
#

IF (ADM_DEBUG)
	ADD_DEFINITIONS(-DADM_DEBUG)
ENDIF (ADM_DEBUG)
###########################################
#  Basic cmake helper script
###########################################
include(admConfigHelper)
include(FindThreads)
INCLUDE(admCheckRequiredLibs)
include(admCheckMiscLibs)
########################################
# Add include dirs
########################################
SET(AVIDEMUX_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
MARK_AS_ADVANCED(AVIDEMUX_INSTALL_DIR)
include(admInstallDir)
include(admCoreIncludes)
LINK_DIRECTORIES("${AVIDEMUX_LIB_DIR}")
#
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ADM_muxerGate/include/")

IF (GETTEXT_FOUND)
	INCLUDE_DIRECTORIES(${GETTEXT_INCLUDE_DIR})
ENDIF (GETTEXT_FOUND)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../common/")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../common/ADM_audioFilter_old")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../common/ADM_commonUI")
#
# Add main source
#
SET(ADM_EXE_SRCS 
../common/ADM_preview.cpp  
../common/gtk_gui.cpp  
../common/gui_autodrive.cpp  
../common/GUI_jobs.cpp  
../common/gui_navigate.cpp  
../common/gui_play.cpp  
../common/gui_save.cpp  
../common/gui_savenew.cpp  
../common/gui_misc.cpp
../common/main.cpp  
../common/gui_action.cpp
../common/gui_blackframes.cpp
../common/ADM_imageLoader.cpp  
../common/ADM_gettext.cpp
)

#############################################
# Add core libs
#############################################
SET(coreLibs
ADM_core
ADM_audioParser
ADM_coreAudio
ADM_coreAudioFilterAPI
ADM_coreAudioDevice
ADM_coreAudioEncoder
ADM_coreDemuxer
ADM_coreDemuxerMpeg
ADM_coreImage
ADM_coreMuxer
ADM_coreUI
ADM_coreUtils
ADM_coreVideoEncoder
ADM_coreVideoFilter
ADM_libavcodec
ADM_libavformat
ADM_libavutil
ADM_libswscale
ADM_libpostproc
ADM_smjs
)

#############################################
# Add common libs
#############################################
SET(commonLibs1
ADM_muxerGate 
ADM_audioFilter
ADM_audioFilter_old 
ADM_editor 
ADM_audiocodec 
ADM_codecs 
ADM_commonUI
)
SET(commonLibs2
ADM_filter 
ADM_osSupport 
ADM_requant 
ADM_script 
ADM_toolkit 
ADM_videoEncoder 
ADM_video 
ADM_videoFilter 
ADM_internalVideoFilter
)

# END
