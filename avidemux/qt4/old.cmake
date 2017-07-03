cmake_minimum_required(VERSION 3.0)
SET(AVIDEMUX_API_VERSION 2.7)
SET(ADM_PROJECT Avidemux_qt4)

MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for Avidemux Qt Started")
MESSAGE("#########################################")
MESSAGE("")
#fedora 
ADD_DEFINITIONS("-std=c++11")
MESSAGE(STATUS "Checking for avidemux development files ..")

IF(NOT FAKEROOT)
	SET(AVIDEMUX_FAKEROOT "")
else(NOT FAKEROOT)
	SET(AVIDEMUX_FAKEROOT "${FAKEROOT}/")
endif(NOT FAKEROOT)


IF(WIN32)
        IF(MSVC) 
                include(adm_win32_vs.cmake)
        ELSEIF(MINGW)
                include(adm_win32_mingw.cmake)
ELSE(WIN32)
        IF(APPLE)
                include(adm_osx.cmake)
        ELSE(APPLE)
                include(adm_default.cmake)
        ENDIF(APPLE)
ENDIF(WIN32)
#-----------------------------------
# Set extra libs, system dependant
#-----------------------------------
# -- Windows --
IF(WIN32)
	IF(MSVC)
			LIST(APPEND PlatformLibs  Qt5::WinMain)
	ENDIF(MSVC)
	IF(MINGW)
		LIST(APPEND PlatformLibs   "-lm -lstdc++")
		LIST(APPEND PlatformLibs   "winmm -mwindows -Wl,--export-all-symbols")
	ENDIF (MINGW)
# -- Windows --
ELSE(WIN32) # OSx, linux, BSD
		LIST(APPEND PlatformLibs   "-lm -lstdc++")
		IF(APPLE)  # OSX
				LIST(APPEND PlatformLibs  "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")
				LIST(APPEND PlatformLibs  "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
		ELSE(APPLE)
			LIST(APPEND PlatformLibs   "X11") # Linux, BSD
		ENDIF(APPLE)
ENDIF(WIN32)

#
#
SET(ADM_HEADER_DIR ${AVIDEMUX_FAKEROOT}${CMAKE_INSTALL_PREFIX}/include/avidemux/${AVIDEMUX_API_VERSION})
SET(ADM_CMAKE_DIR  ${ADM_HEADER_DIR}/cmake)
# Common definitions...
SET(CMAKE_MODULE_PATH "${ADM_CMAKE_DIR}" "${CMAKE_MODULE_PATH}")
MESSAGE(STATUS  "Checking for avidemux include folder (i.e. CMAKE_INSTALL_PREFIX/include/avidemux/${AVIDEMUX_API_VERSION}, looking in ${ADM_HEADER_DIR}")
if(NOT EXISTS "${ADM_HEADER_DIR}")
        MESSAGE(STATUS  "Make sure you installed all the files.\n i cannot find avidemux include folder.cmake .\nSet CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
        MESSAGE(FATAL_ERROR  "Aborting")
endif(NOT EXISTS "${ADM_HEADER_DIR}")

MESSAGE(STATUS "Found avidemux include folder. good.")
MESSAGE(STATUS "Checking for cmake subfolder")

if(NOT EXISTS "${ADM_CMAKE_DIR}/commonCmakeApplication.cmake")
        MESSAGE(STATUS  "Make sure you installed all the files.\n I cannot find content of the cmake subfolder .\n")
        MESSAGE(STATUS  "Set CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
        MESSAGE(STATUS  "I was looking for commonCmakeApplication.cmake in  ${ADM_CMAKE_DIR}")
        MESSAGE(FATAL_ERROR  "Aborting")
endif(NOT EXISTS "${ADM_CMAKE_DIR}/commonCmakeApplication.cmake")
MESSAGE(STATUS "Found cmake subfolder.good.")

include(commonCmakeApplication)
include(../admAppSettings.cmake)
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

CONFIGURE_FILE("${ADM_CMAKE_DIR}/config.h.cmake" "${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/config.h")
MESSAGE(STATUS "${QT_EXTENSION} config.h generated")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/")


###########################################
# Add job control
###########################################
IF(NOT MSVC)
    ADD_SUBDIRECTORY(ADM_jobs)
ENDIF(NOT MSVC)
ADD_SUBDIRECTORY(ADM_update)
IF(USE_OPENGL)
    ADD_SUBDIRECTORY(ADM_openGL)
ENDIF(USE_OPENGL)

########################################
# Add common as a symlink or directly
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

# Add misc subdirs
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsQt4)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfaces${QT_LIBRARY_EXTENSION})

SDLify(../common/main.cpp)
if (ADM_SUBVERSION)
        ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=\"${ADM_SUBVERSION}\"")
endif (ADM_SUBVERSION)
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_VERSION=\"${AVIDEMUX_VERSION}\"")
ADD_SOURCE_CFLAGS(../common/main.cpp "-DQT_FLAVOR=\"${QT_EXTENSION}\"")

###########################################
# Icon for windows
###########################################
if (WIN32 )
        WINDRESIFY(src)
        SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ${src})
endif (WIN32)

###########################################
# Executable
###########################################
include_directories("${PTHREAD_INCLUDE_DIR}")
IF(MSVC)
		SET(EXEC_TYPE "WIN32")
ENDIF(MSVC)
ADD_EXECUTABLE(avidemux3_${QT_EXTENSION} ${EXEC_TYPE} ${ADM_EXE_SRCS})
IF(MSVC)
	set_target_properties(avidemux3_${QT_EXTENSION} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
	set_target_properties(avidemux3_${QT_EXTENSION} PROPERTIES WIN32_EXECUTABLE True)
ENDIF(MSVC)
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



#############################################
# Add qt specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION}
	ADM_gui${QT_LIBRARY_EXTENSION}
	ADM_filters${QT_LIBRARY_EXTENSION}
	ADM_UI${QT_LIBRARY_EXTENSION}6
	ADM_dialogQt4
  ADM_update${QT_LIBRARY_EXTENSION}6
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
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${PTHREAD_LIBRARIES})

###########################################
# OS Specific
###########################################
if (WIN32 OR APPLE)
	set_property(TARGET avidemux3_${QT_EXTENSION} PROPERTY OUTPUT_NAME avidemux)
endif (WIN32 OR APPLE)


IF (APPLE)
	IF (USE_SDL)
		TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_libsdl)
	ENDIF(USE_SDL)
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

TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${PlatformLibs})


IF(QT_QTOPENGL_FOUND)
        target_link_libraries(avidemux3_${QT_EXTENSION} ${QT_QTOPENGL_LIBRARIES})
ENDIF(QT_QTOPENGL_FOUND)
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

IF(MSVC)
    include(./installMsvcRunTime.cmake)
ENDIF(MSVC)
include(admPackager)
admPackager(qt4Package)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_API_VERSION}/${QT_EXTENSION}" COMPONENT dev)


# install headers
ADM_INSTALL_QT_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_openGL/include/"  ADM_openGL)
ADM_INSTALL_QT_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_UIs/include/"  ADM_UIs)

#
IF(WIN32)
        include(FindBourne)
        IF(RELEASE)
                configure_file(
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/ChangeLog.release 
                                ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/ChangeLog.html
                                COPYONLY)
        ELSE(RELEASE)
                execute_process(
                        COMMAND ${BASH_EXECUTABLE} genlog.sh
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../winInstaller/
                        )
        ENDIF(RELEASE)
ENDIF(WIN32)

MESSAGE("")
