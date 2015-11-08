cmake_minimum_required(VERSION 2.6)

SET(ADM_PROJECT Avidemux_qt4)

MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for avidemux Qt4 Started")
MESSAGE("#########################################")
MESSAGE("")

include(../commonCmakeApplication.cmake)
INCLUDE(admCheckQt)
INCLUDE(admWindRes)

MESSAGE(STATUS "Checking Qt")
checkQt()

IF (NOT QT_FOUND)
	MESSAGE(FATAL_ERROR "Qt NOT FOUND")
ENDIF (NOT QT_FOUND)

# Qt4 openGL
include(admCheckOpenGl)
#
#--
MESSAGE(STATUS "Adding Qt inc paths : <${QT_INCLUDES}> and definitions <${QT_DEFINITIONS}>")
INCLUDE_DIRECTORIES(${QT_INCLUDES})
ADD_DEFINITIONS(${QT_DEFINITIONS})
#--

#
##########################################
# Config
##########################################
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_QT4)
SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX ${QT_EXTENSION})

CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/config.h.cmake" "${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/config.h")
MESSAGE(STATUS "${QT_EXTENSION} config.h generated")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/")


###########################################
# Add job control
###########################################
ADD_SUBDIRECTORY(ADM_jobs)

########################################
# Add subdirectories 
########################################
INCLUDE_DIRECTORIES(ADM_UIs/include/)

if (WIN32 AND NOT CROSS)
	ADD_SUBDIRECTORY(../common ./commonQt4)
else (WIN32 AND NOT CROSS)
	# Make symlink else eclipe and kdev4 are puzzled by the tree structure
	# Not needed for plain build
	MESSAGE(STATUS "Creating common symlink in ${CMAKE_CURRENT_SOURCE_DIR}") 

	execute_process(COMMAND rm -f common 	
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	execute_process(COMMAND ln -s ../common .
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	ADD_SUBDIRECTORY(common ./commonQt4)
endif (WIN32 AND NOT CROSS)

ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsQt4)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfaces${QT_LIBRARY_EXTENSION})

SDLify(../common/main.cpp)
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION='\"${ADM_SUBVERSION}\"'")
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_VERSION='\"${AVIDEMUX_VERSION}\"'")
ADD_SOURCE_CFLAGS(../common/main.cpp "-DQT_FLAVOR='\"${QT_EXTENSION}\"'")

###########################################
# Version Info
###########################################
if (WIN32)
        WINDRESIFY(src) 
        SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ${src})
endif (WIN32)

###########################################
# Executable
###########################################
include_directories("${PTHREAD_INCLUDE_DIR}")
ADD_EXECUTABLE(avidemux3_${QT_EXTENSION} ${ADM_EXE_SRCS})

###########################################
# Construct common libraries
###########################################
FOREACH (_libName ${commonLibs1})
        TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${_libName})
ENDFOREACH (_libName ${commonLibs1})

FOREACH (_libName ${commonLibs2})
        TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${_libName})
ENDFOREACH (_libName ${commonLibs2})

FOREACH (_libName ${coreLibs})
	TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${_libName})
ENDFOREACH (_libName ${coreLibs})

#TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_filter6)

#############################################
# Add qt specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION}
	ADM_gui${QT_LIBRARY_EXTENSION}
	ADM_filters${QT_LIBRARY_EXTENSION}
	ADM_UI${QT_LIBRARY_EXTENSION}6
	ADM_dialogQt4
           #ADM_editor6
	ADM_internalVideoFilter6
	ADM_UI${QT_LIBRARY_EXTENSION}6
	ADM_gui${QT_LIBRARY_EXTENSION}
	ADM_UI_${QT_LIBRARY_EXTENSION}6
	ADM_shell${QT_LIBRARY_EXTENSION}
	ADM_toolkit6
	ADM_coreAudio6
	ADM_coreAudioDevice6
	ADM_osSupport6
)

###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)


# SDL
IF (USE_SDL)
	TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${SDL2_LIBRARY} ${SDL2_MAIN})
ENDIF (USE_SDL)


###########################################
# UI Specific
###########################################
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${PTHREAD_LIBRARIES})

###########################################
# OS Specific
###########################################
if (WIN32 OR APPLE)
	set_property(TARGET avidemux3_${QT_EXTENSION} PROPERTY OUTPUT_NAME avidemux)
else (WIN32 OR APPLE)
	target_link_libraries(avidemux3_${QT_EXTENSION} X11)
endif (WIN32 OR APPLE)

IF (MINGW)
	target_link_libraries(avidemux3_${QT_EXTENSION} winmm -mwindows -Wl,--export-all-symbols)
ENDIF (MINGW)

IF (APPLE)
	IF (USE_SDL)
		TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_libsdl)
	ENDIF(USE_SDL)

	target_link_libraries(avidemux3_${QT_EXTENSION} "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")

	# for Leopard but it doesn't hurt Tiger
	target_link_libraries(avidemux3_${QT_EXTENSION} "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF (APPLE)
#
# Needed for cross compiling
#
if(CROSS)
        TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_UI_${QT_LIBRARY_EXTENSION}6)
        TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_UI_${QT_LIBRARY_EXTENSION}6)
        TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_render6_${QT_LIBRARY_EXTENSION})
        TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_coreUtils6)
        TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_core6)
        TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_core6)
endif(CROSS)

IF(QT_QTOPENGL_FOUND)
        target_link_libraries(avidemux3_${QT_EXTENSION} ${QT_QTOPENGL_LIBRARIES})
ENDIF(QT_QTOPENGL_FOUND)
target_link_libraries(avidemux3_${QT_EXTENSION} "-lm -lstdc++")
#
# i18n
#
ADD_SUBDIRECTORY(i18n)
###########################################
# Install
###########################################
ADM_LINK_THREAD(avidemux3_${QT_EXTENSION})
ADM_INSTALL_BIN(avidemux3_${QT_EXTENSION})

DISPLAY_SUMMARY_LIST()

include(admPackager)
admPackager(qt4Package)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/${QT_EXTENSION}" COMPONENT dev) 
MESSAGE("")
