cmake_minimum_required(VERSION 2.6)

SET(VERSION 2.6.0)
SET(ADM_PROJECT Avidemux_cli)

MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for avidemux Cli Started")
MESSAGE("#########################################")
MESSAGE("")

include(../commonCmakeApplication.cmake)

##########################################
# Config
##########################################
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_CLI)
SET(CONFIG_HEADER_TYPE ADM_BUILD_CLI)
SET(UI_SUFFIX cli)

CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/config.h.cmake" "${CMAKE_BINARY_DIR}/config/cli/config.h")
MESSAGE(STATUS "CLI config.h generated")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/config/cli/")

########################################
# Add subdirectories 
########################################
ADD_SUBDIRECTORY(../common ./common)
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsCli)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfacesCli)

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
ADD_EXECUTABLE(avidemux3_cli ${ADM_EXE_SRCS})

###########################################
# Construct common libraries
###########################################
FOREACH (_libName ${commonLibs1})
        TARGET_LINK_LIBRARIES(avidemux3_cli ${_libName})
ENDFOREACH (_libName ${commonLibs1})

TARGET_LINK_LIBRARIES(avidemux3_cli ADM_filtersCli6)

FOREACH (_libName ${commonLibs2})
        TARGET_LINK_LIBRARIES(avidemux3_cli ${_libName})
ENDFOREACH (_libName ${commonLibs2})

FOREACH (_libName ${coreLibs})
        TARGET_LINK_LIBRARIES(avidemux3_cli ${_libName})
ENDFOREACH (_libName ${coreLibs})

#############################################
# Add gtk specific libs
#############################################
TARGET_LINK_LIBRARIES(avidemux3_cli
	ADM_UI_Cli6
	ADM_dialogCli6
	ADM_toolkit6
	ADM_coreAudio6
	ADM_coreUtils6
	ADM_gui2Cli6
	ADM_toolkitCli6
	ADM_shellCli
)

###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux3_cli ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)


# SDL
IF (USE_SDL)
        TARGET_LINK_LIBRARIES(avidemux3_cli ${SDL_LIBRARY})
ENDIF (USE_SDL)

###########################################
# OS Specific
###########################################
if (WIN32 OR APPLE)
	set_property(TARGET avidemux3_cli PROPERTY OUTPUT_NAME avidemux_cli)
endif (WIN32 OR APPLE)

IF (MINGW)
	target_link_libraries(avidemux3_cli winmm -mwindows -Wl,-subsystem,console -Wl,--export-all-symbols)
ENDIF (MINGW)

###########################################
# Install
###########################################
ADM_LINK_THREAD(avidemux3_cli)
ADM_INSTALL_BIN(avidemux3_cli)

DISPLAY_SUMMARY_LIST()

include(admPackager)
admPackager(cliPackage)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/cli/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/cli") 
MESSAGE("")