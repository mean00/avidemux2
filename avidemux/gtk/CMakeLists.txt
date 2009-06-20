MESSAGE("#########################################")
MESSAGE("Configure for avidemux Gtk Started")
MESSAGE("#########################################")
MESSAGE("")
PROJECT(Avidemux_gtk)
include(../commonCmakeApplication.cmake)
include(admCheckGtk)
include(admCheckGlade)
#
checkGtk()
checkGlade()
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
ADD_EXECUTABLE(avidemux2_gtk ${ADM_EXE_SRCS})
ADD_TARGET_CFLAGS(avidemux2_gtk "-I${CMAKE_BINARY_DIR}/config/gtk")
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
	TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_nvwa)
ENDIF (ADM_DEBUG AND FIND_LEAKS)

FOREACH (_libName ${commonLibs1})
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs})

TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_filtersGtk)

FOREACH (_libName ${commonLibs2})
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs})
FOREACH (_libName ${coreLibs})
        TARGET_LINK_LIBRARIES(avidemux2_gtk ${_libName})
ENDFOREACH (_libName ${commonLibs})

#############################################
# Add gtk specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_UIGtk)
TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_dialogGtk)
#TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_filtersGtk)
TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_gui2Gtk)
#TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_ocrGtk)
TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_toolkitGtk)
TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_UI_GTK)
###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux2_gtk ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)
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
	target_link_libraries(ADM_osSupport)

	IF (MINGW)
		target_link_libraries(avidemux2_gtk -mno-cygwin)
	ENDIF (MINGW)

	target_link_libraries(avidemux2_gtk winmm)
        ADD_TARGET_LDFLAGS(avidemux2_gtk "-mwindows")

ENDIF (WIN32)
#
# libxml2
#
TARGET_LINK_LIBRARIES(avidemux2_gtk ${LIBXML2_LIBRARIES})
###########################################
# Install
###########################################
ADM_INSTALL_BIN(avidemux2_gtk)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/gtk/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/gtk") 
include(./gtkPackageDebian.cmake)
