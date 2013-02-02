cmake_minimum_required(VERSION 2.6)

SET(ADM_PROJECT Avidemux_gtk)

MESSAGE("")
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

########################################
# Add subdirectories 
########################################
ADD_SUBDIRECTORY(../common ./common)
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsGtk)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfacesGtk)

SDLify(../common/main.cpp)
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=${ADM_SUBVERSION}")

###########################################
# Version Info
###########################################
if (WIN32)
	# add icon and version info
	SET(FILEVERSION_STRING "${AVIDEMUX_VERSION}.${ADM_SUBVERSION}")
	SET(PRODUCTVERSION_STRING "${AVIDEMUX_VERSION}")
	STRING(REPLACE "." "," FILEVERSION ${FILEVERSION_STRING})
	STRING(REPLACE "." "," PRODUCTVERSION ${PRODUCTVERSION_STRING})
	SET(PRODUCTVERSION "${PRODUCTVERSION},0")

	IF (ADM_CPU_X86_64)
		SET(WIN_RES_TARGET "pe-x86-64")
	ELSE (ADM_CPU_X86_64)
		SET(WIN_RES_TARGET "pe-i386")
	ENDIF (ADM_CPU_X86_64)

	SET(AVIDEMUX_ICON "adm.ico")
	SET(ADM_WIN_RES "adm.obj")
	SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ${ADM_WIN_RES})
	CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/admWin32.rc.in" ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc IMMEDIATE)
        include(admWindRes)
	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ADM_WIN_RES} COMMAND ${WINDRES}.exe -F ${WIN_RES_TARGET} -i ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc -o ${CMAKE_CURRENT_BINARY_DIR}/${ADM_WIN_RES} -O coff --define VS_VERSION_INFO=1)
endif (WIN32)

###########################################
# Executable
###########################################
ADD_EXECUTABLE(avidemux3_gtk ${ADM_EXE_SRCS})

###########################################
# Construct common libraries
###########################################
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
	ADM_coreAudio6
	ADM_coreUtils6
	#ADM_ocrGtk
	ADM_toolkitGtk
	ADM_shellGtk
	ADM_UIGtk6
	ADM_coreAudioDevice6
        ADM_osSupport6
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
if (WIN32 OR APPLE)
	set_property(TARGET avidemux3_gtk PROPERTY OUTPUT_NAME avidemux_gtk)
endif (WIN32 OR APPLE)

IF (WIN32)
	target_link_libraries(avidemux3_gtk winmm -mwindows -Wl,--export-all-symbols)
ENDIF (WIN32)

###########################################
# Install
###########################################
ADM_LINK_THREAD(avidemux3_gtk)
ADM_INSTALL_BIN(avidemux3_gtk)

DISPLAY_SUMMARY_LIST()

include(admPackager)
admPackager(gtkPackage)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/gtk/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/gtk") 
MESSAGE("")
