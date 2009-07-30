#
#
#
SET(VERSION 2.6.0)
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
if(NOT PLUGINS)
        SET(AVIDEMUX_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../..)
endif(NOT PLUGINS)
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

IF (UNIX AND NOT APPLE)
	# jog shuttle is only available on Linux due to its interface
	SET(USE_JOG 1)
ENDIF (UNIX AND NOT APPLE)

IF (WIN32)
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-enable-auto-image-base -Wl,-enable-auto-import")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-enable-auto-import")

	IF (CMAKE_BUILD_TYPE STREQUAL "Release")
		SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-s")
		SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")
	ENDIF (CMAKE_BUILD_TYPE STREQUAL "Release")
ENDIF (WIN32)


include(FindThreads)
INCLUDE(admCheckRequiredLibs)
include(admCheckMiscLibs)
include( admGetRevision)
########################################
# Subversion
########################################
MESSAGE("")
MESSAGE(STATUS "Checking for SCM")
MESSAGE(STATUS "****************")
admGetRevision( ${AVIDEMUX_TOP_SOURCE_DIR} ADM_SUBVERSION)
MESSAGE("")
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
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../common/ADM_audioFilter/include")
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
../common/main.cpp  
../common/gui_action.cpp
../common/gui_audio.cpp
../common/gui_blackframes.cpp
../common/ADM_imageLoader.cpp  
../common/ADM_gettext.cpp
)

#############################################
# Add core libs
#############################################
SET(coreLibs
ADM_core6
ADM_audioParser6
ADM_coreAudio6
ADM_coreAudioFilterAPI6
ADM_coreAudioDevice6
ADM_coreAudioEncoder6
ADM_coreDemuxer6
ADM_coreDemuxerMpeg6
ADM_coreImage6
ADM_coreMuxer6
ADM_coreUI6
ADM_coreUtils6
ADM_coreVideoEncoder6
ADM_coreVideoFilter6
ADM_libavcodec6
ADM_libavformat6
ADM_libavutil6
ADM_libswscale6
ADM_libpostproc6
ADM_smjs6
)

#############################################
# Add common libs
#############################################
SET(commonLibs1
ADM_muxerGate6
ADM_audioFilter6
ADM_editor6
ADM_audiocodec6 
ADM_codecs6 
ADM_commonUI6
)
SET(commonLibs2
ADM_filter6 
ADM_osSupport6 
ADM_requant6 
ADM_script6 
ADM_toolkit6
ADM_videoEncoder6 
ADM_video6 
ADM_videoFilter6 
ADM_internalVideoFilter6
)

# END
