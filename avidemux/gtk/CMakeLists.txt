cmake_minimum_required(VERSION 2.6)

SET(VERSION 2.6.0)
SET(ADM_PROJECT Avidemux_gtk)

MESSAGE("#########################################")
MESSAGE("Configure for avidemux Gtk Started")
MESSAGE("#########################################")
MESSAGE("")

include(../commonCmakeApplication.cmake)
include(admCheckGtk)

checkGtk()

##########################################
# Config
##########################################
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_GTK)
SET(CONFIG_HEADER_TYPE ADM_BUILD_GTK)
SET(UI_SUFFIX gtk)

CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/config.h.cmake" "${CMAKE_BINARY_DIR}/config/gtk/config.h")
MESSAGE(STATUS "GTK config.h generated")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/config/gtk/")

###########################################
# Executable
###########################################
ADD_EXECUTABLE(avidemux3_gtk ${ADM_EXE_SRCS})

########################################
# Add subdirectories 
########################################
ADD_SUBDIRECTORY(../common ./common)
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsGtk)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfacesGtk)

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
ENDFOREACH (_libName ${commonLibs1})

TARGET_LINK_LIBRARIES(avidemux3_gtk ADM_filtersGtk)

FOREACH (_libName ${commonLibs2})
        TARGET_LINK_LIBRARIES(avidemux3_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs2})

FOREACH (_libName ${coreLibs})
        TARGET_LINK_LIBRARIES(avidemux3_gtk ${_libName})
ENDFOREACH (_libName ${coreLibs})

#############################################
# Add gtk specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux3_gtk
	ADM_UI_GTK
	ADM_dialogGtk
	#ADM_filtersGtk
	ADM_gui2Gtk
	ADM_toolkit6
	ADM_coreUtils6
	#ADM_ocrGtk
	ADM_toolkitGtk
	ADM_shellGtk
	ADM_UIGtk6
	ADM_coreAudioDevice6
)

###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux3_gtk ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)


# SDL
IF (USE_SDL)
        TARGET_LINK_LIBRARIES(avidemux3_gtk ${SDL_LIBRARY})
ENDIF (USE_SDL)


###########################################
# UI Specific
###########################################
TARGET_LINK_LIBRARIES(avidemux3_gtk ${GTK_LDFLAGS} ${GTHREAD_LDFLAGS})

###########################################
# OS Specific
###########################################
IF (WIN32)
	target_link_libraries(ADM_osSupport)

	IF (MINGW)
		target_link_libraries(avidemux3_gtk -mno-cygwin)
	ENDIF (MINGW)

	target_link_libraries(avidemux3_gtk winmm -mwindows)
ENDIF (WIN32)

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
