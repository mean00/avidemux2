cmake_minimum_required(VERSION 2.6)

SET(VERSION 2.6.0)
SET(ADM_PROJECT Avidemux_qt4)

MESSAGE("#########################################")
MESSAGE("Configure for avidemux Qt4 Started")
MESSAGE("#########################################")
MESSAGE("")

include(../commonCmakeApplication.cmake)
INCLUDE(admCheckQt4)

checkQt4()

IF (NOT QT4_FOUND)
	MESSAGE(FATAL_ERROR "Qt4 NOT FOUND")
ENDIF (NOT QT4_FOUND)

# Qt4 openGL
include(admCheckOpenGl)

##########################################
# Config
##########################################
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_QT4)
SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX qt4)

CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/config.h.cmake" "${CMAKE_BINARY_DIR}/config/qt4/config.h")
MESSAGE(STATUS "QT4 config.h generated")

INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/config/qt4/")

###########################################
# Executable
###########################################
ADD_EXECUTABLE(avidemux3_qt4 ${ADM_EXE_SRCS})

########################################
# Add subdirectories 
########################################
INCLUDE_DIRECTORIES(ADM_UIs/include/)

if (WIN32)
	ADD_SUBDIRECTORY(../common ./commonQt4)
else (WIN32)
	# Make symlink else eclipe and kdev4 are puzzled by the tree structure
	# Not needed for plain build
	MESSAGE(STATUS "Creating common symlink in ${CMAKE_CURRENT_SOURCE_DIR}") 

	execute_process(COMMAND rm -f common 	
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	execute_process(COMMAND ln -s ../common .
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	ADD_SUBDIRECTORY(common ./commonQt4)
endif (WIN32)

ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsQt4)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfacesQT4)

SDLify(../common/main.cpp)
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=${ADM_SUBVERSION}")

###########################################
# Construct common libraries
###########################################
IF (ADM_DEBUG AND FIND_LEAKS)
	TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_nvwa)
ENDIF (ADM_DEBUG AND FIND_LEAKS)

FOREACH (_libName ${commonLibs1})
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ${_libName})
ENDFOREACH (_libName ${commonLibs1})

FOREACH (_libName ${commonLibs2})
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ${_libName})
ENDFOREACH (_libName ${commonLibs2})

FOREACH (_libName ${coreLibs})
	TARGET_LINK_LIBRARIES(avidemux3_qt4 ${_libName})
ENDFOREACH (_libName ${coreLibs})

#TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_filter6)

#############################################
# Add qt specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux3_qt4
	ADM_guiQt4
	ADM_filtersQt4
	ADM_UIQT46
	ADM_dialogQt4
	ADM_internalVideoFilter6
	ADM_UIQT46
	ADM_guiQt4
	ADM_UI_QT46
	ADM_shellQt4
	ADM_toolkit6
	ADM_coreAudioDevice6
)

###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux3_qt4 ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)

# XVideo
IF (USE_XV)
	TARGET_LINK_LIBRARIES(avidemux3_qt4 ${XVIDEO_LIBRARY_DIR})
ENDIF (USE_XV)

# SDL
IF (USE_SDL)
	TARGET_LINK_LIBRARIES(avidemux3_qt4 ${SDL_LIBRARY})
ENDIF (USE_SDL)

# libxml2
TARGET_LINK_LIBRARIES(avidemux3_qt4 ${LIBXML2_LIBRARIES})

###########################################
# UI Specific
###########################################
TARGET_LINK_LIBRARIES(avidemux3_qt4 ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

###########################################
# OS Specific
###########################################
IF (WIN32)
	target_link_libraries(ADM_osSupport)

	IF (MINGW)
		target_link_libraries(avidemux3_qt4 -mno-cygwin)
	ENDIF (MINGW)

	target_link_libraries(avidemux3_qt4 winmm -mwindows)
ENDIF (WIN32)

IF (APPLE)
	IF (USE_SDL)
		TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_libsdl)
	ENDIF(USE_SDL)

	ADD_TARGET_LDFLAGS(avidemux3_qt4 "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")

	# for Leopard but it doesn't hurt Tiger
	ADD_TARGET_LDFLAGS(avidemux3_qt4 "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF (APPLE)
#
# Needed for cross compiling
#
if(CROSS)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_UI_QT46)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_UI_QT46)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_render6_qt4)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_coreUtils6)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_core6)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_core6)
endif(CROSS)


###########################################
# Install
###########################################
ADM_LINK_THREAD(avidemux3_qt4)
ADM_INSTALL_BIN(avidemux3_qt4)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/qt4/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/qt4") 
#
# Packaging
#
include(admPackager)
admPackager(qt4Package)
