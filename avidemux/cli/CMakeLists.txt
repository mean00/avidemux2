MESSAGE("#########################################")
MESSAGE("Configure for avidemux Cli Started")
MESSAGE("#########################################")
MESSAGE("")
SET(VERSION 2.6.0)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
if (COMMAND cmake_policy)
	cmake_policy(VERSION 2.4)
	cmake_policy(SET CMP0003 NEW)
	#cmake_policy(SET CMP0011 OLD)
endif (COMMAND cmake_policy)
SET( ADM_PROJECT Avidemux_cli)

include(../commonCmakeApplication.cmake)
##########################################
# Config
##########################################
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_CLI)
SET(CONFIG_HEADER_TYPE ADM_BUILD_CLI)
SET(UI_SUFFIX cli)
#
CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/config.h.cmake" "${CMAKE_BINARY_DIR}/config/cli/config.h")
MESSAGE(STATUS "CLI config.h generated")

INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/config/cli/")
###########################################
# Executable
###########################################
ADD_EXECUTABLE(avidemux3_cli ${ADM_EXE_SRCS})
ADD_TARGET_CFLAGS(avidemux3_cli "-I${CMAKE_BINARY_DIR}/config/cli")
########################################
# Add subdirectories 
########################################
ADD_SUBDIRECTORY(../common ./common)
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsCli)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfacesCli)
# Executable

SDLify(../common/main.cpp)
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=${ADM_SUBVERSION}")
###########################################
# Construct common libraries
###########################################

IF (ADM_DEBUG AND FIND_LEAKS)
	TARGET_LINK_LIBRARIES(avidemux3_cli ADM_nvwa)
ENDIF (ADM_DEBUG AND FIND_LEAKS)

FOREACH (_libName ${commonLibs1})
        TARGET_LINK_LIBRARIES(avidemux3_cli ${_libName})
ENDFOREACH (_libName ${commonLibs})

TARGET_LINK_LIBRARIES(avidemux3_cli ADM_filtersCli6)

FOREACH (_libName ${commonLibs2})
        TARGET_LINK_LIBRARIES(avidemux3_cli ${_libName})
ENDFOREACH (_libName ${commonLibs})
FOREACH (_libName ${coreLibs})
        TARGET_LINK_LIBRARIES(avidemux3_cli ${_libName})
ENDFOREACH (_libName ${commonLibs})

#############################################
# Add gtk specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux3_cli ADM_UI_Cli6)
TARGET_LINK_LIBRARIES(avidemux3_cli ADM_dialogCli6)
TARGET_LINK_LIBRARIES(avidemux3_cli ADM_gui2Cli6)
TARGET_LINK_LIBRARIES(avidemux3_cli ADM_toolkitCli6)
TARGET_LINK_LIBRARIES(avidemux3_cli ADM_UI_Cli6)
TARGET_LINK_LIBRARIES(avidemux3_cli ADM_UI_Cli6)
TARGET_LINK_LIBRARIES(avidemux3_cli ADM_shellCli)
TARGET_LINK_LIBRARIES(avidemux3_cli ADM_toolkit6)
###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux3_cli ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)
# PNG
IF (USE_PNG)
        TARGET_LINK_LIBRARIES(avidemux3_cli ${PNG_LIBRARIES})
	
ENDIF (USE_PNG)


# XVideo
IF (USE_XV)
        TARGET_LINK_LIBRARIES(avidemux3_cli ${XVIDEO_LIBRARY_DIR})
ENDIF (USE_XV)

# SDL
IF (USE_SDL)
        TARGET_LINK_LIBRARIES(avidemux3_cli ${SDL_LIBRARY})
ENDIF (USE_SDL)

###########################################
# UI Specific
###########################################
ADD_TARGET_LDFLAGS(avidemux3_cli ${GTK_LDFLAGS} ${GTHREAD_LDFLAGS})


###########################################
# OS Specific
###########################################
IF (WIN32)
	target_link_libraries(ADM_osSupport)

	IF (MINGW)
		target_link_libraries(avidemux3_cli -mno-cygwin)
	ENDIF (MINGW)

	target_link_libraries(avidemux3_cli winmm)
        ADD_TARGET_LDFLAGS(avidemux3_cli "-mwindows")

ENDIF (WIN32)
#
# libxml2
#
TARGET_LINK_LIBRARIES(avidemux3_cli ${LIBXML2_LIBRARIES})
###########################################
# Install
###########################################
ADM_LINK_THREAD(avidemux3_cli)
ADM_INSTALL_BIN(avidemux3_cli)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/cli/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/cli") 
#
# Packaging
#
include(admPackager)
admPackager(cliPackage)
