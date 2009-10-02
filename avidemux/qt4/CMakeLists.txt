MESSAGE("#########################################")
MESSAGE("Configure for avidemux Qt4 Started")
MESSAGE("#########################################")
MESSAGE("")
SET(VERSION 2.6.0)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
if (COMMAND cmake_policy)
	cmake_policy(VERSION 2.4)
	cmake_policy(SET CMP0003 NEW)
	#cmake_policy(SET CMP0011 OLD)
endif (COMMAND cmake_policy)
SET( ADM_PROJECT Avidemux_qt4)
# Use old behaviour
include(../commonCmakeApplication.cmake)
INCLUDE(admCheckQt4)
checkQt4()
IF (NOT QT4_FOUND)
        MESSAGE(FATAL_ERROR "Qt4 NOT FOUND")
ENDIF (NOT QT4_FOUND)

##########################################
# Config
##########################################

SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX qt4)
#
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_QT4)
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
ADD_SUBDIRECTORY(../common ./commonQt4)
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsQt4)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfacesQT4)
# Executable

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
ENDFOREACH (_libName ${commonLibs})


FOREACH (_libName ${commonLibs2})
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ${_libName})
ENDFOREACH (_libName ${commonLibs})
#TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_filter6)
#############################################
# Add qt specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_UIQT4)
TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_dialogQt4)
TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_guiQt4)
TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_filtersQt4)
TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_UIQT4)
TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_guiQt4)
TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_UI_QT4)
###########################################
# Construct Core libraries
###########################################

FOREACH (_libName ${coreLibs})
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ${_libName})
ENDFOREACH (_libName ${commonLibs})
        

###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux3_qt4 ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)


# PNG
IF (USE_PNG)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ${PNG_LIBRARIES})
	
ENDIF (USE_PNG)


# XVideo
IF (USE_XV)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ${XVIDEO_LIBRARY_DIR})
ENDIF (USE_XV)

# SDL
IF (USE_SDL)
        TARGET_LINK_LIBRARIES(avidemux3_qt4 ${SDL_LIBRARY})
ENDIF (USE_SDL)

###########################################
# UI Specific
###########################################
ADD_TARGET_LDFLAGS(avidemux3_qt4  ${GTHREAD_LDFLAGS})
TARGET_LINK_LIBRARIES(avidemux3_qt4 ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

###########################################
# OS Specific
###########################################
IF (WIN32)
	target_link_libraries(ADM_osSupport)

	IF (MINGW)
		target_link_libraries(avidemux3_qt4 -mno-cygwin)
	ENDIF (MINGW)

	target_link_libraries(avidemux3_qt4 winmm)
        ADD_TARGET_LDFLAGS(avidemux3_qt4 "-mwindows")

ENDIF (WIN32)
#
# xml
#
TARGET_LINK_LIBRARIES(avidemux3_qt4 ${LIBXML2_LIBRARIES})

###########################################
# Install
###########################################
ADM_LINK_THREAD(avidemux3_qt4)
ADM_INSTALL_BIN(avidemux3_qt4)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/qt4/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/qt4") 
include(./qt4PackageDebian.cmake)
