MESSAGE("#########################################")
MESSAGE("Configure for avidemux Gtk Started")
MESSAGE("#########################################")
MESSAGE("")
SET(VERSION 2.6.0)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
if (COMMAND cmake_policy)
	cmake_policy(VERSION 2.4)
	cmake_policy(SET CMP0003 NEW)
	#cmake_policy(SET CMP0011 OLD)
endif (COMMAND cmake_policy)
SET( ADM_PROJECT Avidemux_gtk)

include(../commonCmakeApplication.cmake)
include(admCheckGtk)
#include(admCheckGlade)
#
checkGtk()
#checkGlade()
##########################################
# Config
##########################################
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_GTK)
SET(CONFIG_HEADER_TYPE ADM_BUILD_GTK)
SET(UI_SUFFIX gtk)
#
CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/config.h.cmake" "${CMAKE_BINARY_DIR}/config/gtk/config.h")
MESSAGE(STATUS "GTK config.h generated")

INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/config/gtk/")
###########################################
# Executable
###########################################
ADD_EXECUTABLE(avidemux3_gtk ${ADM_EXE_SRCS})
ADD_TARGET_CFLAGS(avidemux3_gtk "-I${CMAKE_BINARY_DIR}/config/gtk")
########################################
# Add subdirectories 
########################################
ADD_SUBDIRECTORY(../common ./common)
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsGtk)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfacesGtk)
# Executable

SDLify(../common/main.cpp)
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=${ADM_SUBVERSION}")
###########################################
# Construct common libraries
###########################################

IF (ADM_DEBUG AND FIND_LEAKS)
	TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_nvwa)
ENDIF (ADM_DEBUG AND FIND_LEAKS)

FOREACH (_libName ${commonLibs1})
        TARGET_LINK_LIBRARIES(avidemux3_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs})

TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_filtersGtk)

FOREACH (_libName ${commonLibs2})
        TARGET_LINK_LIBRARIES(avidemux3_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs})
FOREACH (_libName ${coreLibs})
        TARGET_LINK_LIBRARIES(avidemux3_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs})

#############################################
# Add gtk specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_UIGtk6)
TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_dialogGtk)
#TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_filtersGtk)
TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_gui2Gtk)
#TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_ocrGtk)
TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_toolkitGtk)
TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_UI_GTK)
TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_shellGtk)
TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_toolkit6)
###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux3_gtk ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)

# XVideo
IF (USE_XV)
        TARGET_LINK_LIBRARIES(avidemux3_gtk ${XVIDEO_LIBRARY_DIR})
ENDIF (USE_XV)

# SDL
IF (USE_SDL)
        TARGET_LINK_LIBRARIES(avidemux3_gtk ${SDL_LIBRARY})
ENDIF (USE_SDL)

###########################################
# UI Specific
###########################################
ADD_TARGET_LDFLAGS(avidemux3_gtk ${GTK_LDFLAGS} ${GTHREAD_LDFLAGS})


###########################################
# OS Specific
###########################################
IF (WIN32)
	target_link_libraries(ADM_osSupport)

	IF (MINGW)
		target_link_libraries(avidemux3_gtk -mno-cygwin)
	ENDIF (MINGW)

	target_link_libraries(avidemux3_gtk winmm)
        ADD_TARGET_LDFLAGS(avidemux3_gtk "-mwindows")

ENDIF (WIN32)
#
# libxml2
#
TARGET_LINK_LIBRARIES(avidemux3_gtk ${LIBXML2_LIBRARIES})
###########################################
# Install
###########################################
ADM_LINK_THREAD(avidemux3_gtk)
ADM_INSTALL_BIN(avidemux3_gtk)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/gtk/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/gtk") 
#
# Packaging
#
include(admPackager)
admPackager(gtkPackage)
