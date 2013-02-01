cmake_minimum_required(VERSION 2.6)

SET(ADM_PROJECT Avidemux_qt4)

MESSAGE("")
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

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/config/qt4/")


###########################################
# Add job control
###########################################
ADD_SUBDIRECTORY(ADM_jobs)

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
# Version Info
###########################################
if (WIN32)
	# add icon and version info
	SET(FILEVERSION_STRING "${VERSION}.${ADM_SUBVERSION}")
	SET(PRODUCTVERSION_STRING "${VERSION}")
	STRING(REPLACE "." "," FILEVERSION ${FILEVERSION_STRING})
	STRING(REPLACE "." "," PRODUCTVERSION ${PRODUCTVERSION_STRING})
	SET(PRODUCTVERSION "${PRODUCTVERSION},0")

	IF (ADM_CPU_X86_64)
		SET(WIN_RES_TARGET "pe-x86-64")
	ELSE (ADM_CPU_X86_64)
		SET(WIN_RES_TARGET "pe-i386")
	ENDIF (ADM_CPU_X86_64)

	SET(AVIDEMUX_ICON "adm.ico")
	CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/admWin32.rc.in" ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc IMMEDIATE)

	if (MINGW)
		SET(ADM_WIN_RES "adm.obj")
		SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ${ADM_WIN_RES})
        include(admWindRes)	
		ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ADM_WIN_RES} COMMAND ${WINDRES} -F ${WIN_RES_TARGET} -i ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc -o ${CMAKE_CURRENT_BINARY_DIR}/${ADM_WIN_RES} -O coff --define VS_VERSION_INFO=1)
	endif (MINGW)

	if (MSVC)
		SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc)
	endif (MSVC)
endif (WIN32)

###########################################
# Executable
###########################################
include_directories("${PTHREAD_INCLUDE_DIR}")
ADD_EXECUTABLE(avidemux3_qt4 ${ADM_EXE_SRCS})

###########################################
# Construct common libraries
###########################################
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
           #ADM_editor6
	ADM_internalVideoFilter6
	ADM_UIQT46
	ADM_guiQt4
	ADM_UI_QT46
	ADM_shellQt4
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
	TARGET_LINK_LIBRARIES(avidemux3_qt4 ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)


# SDL
IF (USE_SDL)
	TARGET_LINK_LIBRARIES(avidemux3_qt4 ${SDL_LIBRARY})
ENDIF (USE_SDL)


###########################################
# UI Specific
###########################################
TARGET_LINK_LIBRARIES(avidemux3_qt4 ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

###########################################
# OS Specific
###########################################
if (WIN32 OR APPLE)
	set_property(TARGET avidemux3_qt4 PROPERTY OUTPUT_NAME avidemux)
endif (WIN32 OR APPLE)

IF (MINGW)
	target_link_libraries(avidemux3_qt4 winmm -mwindows -Wl,--export-all-symbols)
ENDIF (MINGW)

IF (APPLE)
	IF (USE_SDL)
		TARGET_LINK_LIBRARIES(avidemux3_qt4 ADM_libsdl)
	ENDIF(USE_SDL)

	target_link_libraries(avidemux3_qt4 "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")

	# for Leopard but it doesn't hurt Tiger
	target_link_libraries(avidemux3_qt4 "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
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

DISPLAY_SUMMARY_LIST()

include(admPackager)
admPackager(qt4Package)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/qt4/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/qt4") 
MESSAGE("")
