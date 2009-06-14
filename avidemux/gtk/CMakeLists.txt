MESSAGE("#########################################")
MESSAGE("Configure for avidemux Gtk Started")
MESSAGE("#########################################")
MESSAGE("")
PROJECT(Avidemux_gtk)

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
IF (NOT AVIDEMUX_CORE_BUILD_DIR)
	MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_CORECONFIG_DIR=path_to_avidemux_core_build_dir")
ELSE (NOT AVIDEMUX_CORE_BUILD_DIR)
	SET(AVIDEMUX_CORE_BUILD_DIR "${AVIDEMUX_CORE_BUILD_DIR}" CACHE STRING "")
ENDIF (NOT AVIDEMUX_CORE_BUILD_DIR)
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
# Check Gtk & friends
###########################################
include(admConfigHelper)
INCLUDE(admCheckRequiredLibs)
include(admCheckMiscLibs)
include(admCheckGtk)
include(admCheckGlade)
#
checkGtk()
checkGlade()
########################################
# Add include dirs
########################################
include(admCoreIncludes)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ADM_muxerGate/include/")

IF (GETTEXT_FOUND)
	INCLUDE_DIRECTORIES(${GETTEXT_INCLUDE_DIR})
ENDIF (GETTEXT_FOUND)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../common/")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../common/ADM_audioFilter_old")
# 
##########################################
# Config
##########################################

SET(CONFIG_HEADER_TYPE ADM_BUILD_GTK)
CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/config.h.cmake" "${CMAKE_BINARY_DIR}/config/gtk/config.h")
MESSAGE(STATUS "GTK config.h generated")

INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/config/gtk/")
INCLUDE_DIRECTORIES("${AVIDEMUX_CORE_BUILD_DIR}/config")
########################################
# Add subdirectories 
########################################
ADD_SUBDIRECTORY(../common ./common)
ADD_SUBDIRECTORY(ADM_UIs)
ADD_SUBDIRECTORY(ADM_userInterfaces)
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
# Executable
ADD_EXECUTABLE(avidemux2_gtk ${ADM_EXE_SRCS})
ADD_TARGET_CFLAGS(avidemux2_gtk "-I${CMAKE_BINARY_DIR}/config/gtk")

SDLify(../common/main.cpp)
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=${ADM_SUBVERSION}")
###########################################
# Construct ADM_libraries
###########################################

IF (ADM_DEBUG AND FIND_LEAKS)
	ADD_ADMLIB_ALL_TARGETS(ADM_nvwa)
ENDIF (ADM_DEBUG AND FIND_LEAKS)

SET(commonLibs
ADM_audiocodec 
ADM_audioFilter_old 
ADM_editor 
ADM_filter 
ADM_osSupport 
ADM_requant 
ADM_script 
ADM_toolkit 
ADM_video 
ADM_videoFilter )
FOREACH (_libName ${commonLibs})
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs})

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
FOREACH (_libName ${coreLibs})
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs})

###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux2_gtk ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)

# pthreads
IF (UNIX AND PTHREAD_FOUND)
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${PTHREAD_LIBRARIES})
ENDIF (UNIX AND PTHREAD_FOUND)

# PNG
IF (USE_PNG)
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${PNG_LIBRARIES})
	
ENDIF (USE_PNG)


# XVideo
IF (USE_XV)
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${XVIDEO_LIBRARY_DIR})
ENDIF (USE_XV)

# SDL
IF (USE_SDL)
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${SDL_LIBRARY})
ENDIF (USE_SDL)

###########################################
# UI Specific
###########################################
ADD_TARGET_LDFLAGS(avidemux2_gtk ${GTK_LDFLAGS} ${GTHREAD_LDFLAGS})


###########################################
# OS Specific
###########################################
IF (WIN32)
	ADD_LIB_ALL_TARGETS(ADM_osSupport)

	IF (MINGW)
		ADD_LIB_ALL_TARGETS(-mno-cygwin)
	ENDIF (MINGW)

	ADD_LIB_ALL_TARGETS(winmm)

	ADD_TARGET_LDFLAGS(avidemux2_cli "-Wl,-subsystem,console")

        ADD_TARGET_LDFLAGS(avidemux2_gtk "-mwindows")

ENDIF (WIN32)

###########################################
# Install
###########################################
#INSTALL(TARGETS avidemux2_gtk RUNTIME DESTINATION ${BIN_DIR} LIBRARY DESTINATION lib)
