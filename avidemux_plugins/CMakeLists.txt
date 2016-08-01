cmake_minimum_required(VERSION 2.6)
SET(AVIDEMUX_API_VERSION 2.6)

SET(ADM_PROJECT AdmPlugins)
SET(PLUGINS TRUE)

MESSAGE("")
MESSAGE("#####################################")
MESSAGE("Plugins Configure Started")
MESSAGE("#####################################")
MESSAGE("")
MESSAGE(STATUS "Checking for avidemux development files ..")



IF(NOT FAKEROOT)
	SET(AVIDEMUX_FAKEROOT "")
else(NOT FAKEROOT)
	SET(AVIDEMUX_FAKEROOT "${FAKEROOT}/")
endif(NOT FAKEROOT)



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


########################################
# Where is the top dir ?
########################################

# Common definitions...
include(commonCmakeApplication)

########################################
# Add include dirs
########################################
SET(AVIDEMUX_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
MARK_AS_ADVANCED(AVIDEMUX_INSTALL_DIR)
include(admInstallDir)

#########################################
# Get regular stuff from main app
#########################################
include(admCoreIncludes)
LINK_DIRECTORIES("${AVIDEMUX_SEARCH_LIB_DIR}")
INCLUDE_DIRECTORIES("${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/2.6")


IF (FRESH_BUILD)
	MESSAGE("")
ENDIF (FRESH_BUILD)
########################################
# retrieve core config, needed to get USE_VDPAU and friends
########################################
include(${ADM_HEADER_DIR}/ADM_coreConfig.cmake)

########################################
# Global options
########################################
OPTION(VERBOSE "" OFF)

########################################
# Select our UI
########################################
SET(DO_GTK    0)
SET(DO_QT4    0)
SET(DO_COMMON 0)
SET(DO_CLI    0)
SET(DO_SETTINGS    0)

IF (NOT PLUGIN_UI)
	MESSAGE(FATAL_ERROR "Please add -DPLUGIN_UI=[ALL|QT4|GTK|CLI|COMMON]")
ENDIF (NOT PLUGIN_UI)

if(PLUGIN_UI MATCHES "ALL")
    SET(DO_COMMON 1)
    SET(DO_GTK    1)
    SET(DO_QT4    1)
    SET(DO_CLI    1)
endif(PLUGIN_UI MATCHES "ALL")

if(PLUGIN_UI MATCHES "QT4")
    SET(DO_QT4    1)
endif(PLUGIN_UI MATCHES "QT4")

if(PLUGIN_UI MATCHES "GTK")
    SET(DO_GTK    1)
endif(PLUGIN_UI MATCHES "GTK")

if(PLUGIN_UI MATCHES "COMMON")
    SET(DO_COMMON    1)
endif(PLUGIN_UI MATCHES "COMMON")

if(PLUGIN_UI MATCHES "CLI")
    SET(DO_CLI    1)
endif(PLUGIN_UI MATCHES "CLI")

if(PLUGIN_UI MATCHES "SETTINGS")
    SET(DO_SETTINGS    1)
endif(PLUGIN_UI MATCHES "SETTINGS")


MESSAGE(STATUS "Plugin UI : ${PLUGIN_UI}")
MESSAGE("")

########################################
# Check for UI libraries
########################################
MACRO(CHECK_ADM_CONFIG_H flavor result)
    MESSAGE(STATUS "Checking for avidemux/${flavor} includes and lib...")
    MESSAGE(STATUS "It should be ${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/2.6/${flavor}/config.h")
    set(ADM_INC "NOTFOUND")
    FIND_PATH(ADM_INC config.h "${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/2.6/${flavor}")
    #MESSAGE(STATUS "result : ${ADM_INC_NOT_FOUND},${ADM_INC}")
    IF(${ADM_INC} MATCHES "NOTFOUND")
        SET(${result} 0)
        MESSAGE(STATUS "Include for ${flavor} not found")
    ELSE(${ADM_INC} MATCHES "NOTFOUND")
        SET(${result} 1)
        MESSAGE(STATUS "Include for ${flavor} found")
    ENDIF(${ADM_INC} MATCHES "NOTFOUND")
ENDMACRO(CHECK_ADM_CONFIG_H flavor result)

APPEND_SUMMARY_LIST("User Interface" "Common" "${DO_COMMON}")

#*****
# GTK
#*****
if(DO_GTK)
    INCLUDE(admCheckGtk)
    INCLUDE(admCheckGettext)
    checkGtk()
    checkGettext()
    IF (GTK_FOUND AND GTHREAD_FOUND)
        MESSAGE(STATUS "Gtk and gthread found, good")
        CHECK_ADM_CONFIG_H(gtk ADM_INC)
        IF(NOT ADM_INC)
            MESSAGE("ERROR GTK UI requested, but cannot find header file for avidemux/gtk...disabling it")
			MESSAGE("")
            SET(DO_GTK 0)
        ENDIF(NOT ADM_INC)
    else (GTK_FOUND AND GTHREAD_FOUND)
        MESSAGE("ERROR GTK UI requested, but gtk/gthread libraries not found... disabling it")
		MESSAGE("")
        SET(DO_GTK 0)
    endif (GTK_FOUND AND GTHREAD_FOUND)
endif(DO_GTK)

APPEND_SUMMARY_LIST("User Interface" "GTK+" "${DO_GTK}")

#*****
# QT4
#*****
if(DO_QT4)
    INCLUDE(admCheckQt) 
    checkQt()
    IF (NOT QT_FOUND)
        SET(DO_QT4 0)
        MESSAGE("ERROR QT4 UI requested, but qt4 libraries not found... disabling it")
		MESSAGE("")
    ELSE (NOT QT_FOUND)
        MESSAGE(STATUS "QT4 libraries found, good")
        CHECK_ADM_CONFIG_H(${QT_EXTENSION} ADM_INC)
		MESSAGE("")

        INCLUDE(admCheckOpenGl)
        IF(NOT ADM_INC)
            MESSAGE("ERROR QT4 UI requested, but cannot find header file for avidemux/qt4...disabling it")
			MESSAGE("")
            SET(DO_QT4 0)
        ENDIF(NOT ADM_INC)
    ENDIF (NOT QT_FOUND)
endif(DO_QT4)

APPEND_SUMMARY_LIST("User Interface" "Qt 4" "${DO_QT4}")

#*****
# CLI
#*****
if(DO_CLI)
		MESSAGE(STATUS "Checking for CLI")
		MESSAGE(STATUS "****************")

        CHECK_ADM_CONFIG_H(cli ADM_INC)

        IF(NOT ADM_INC)
            MESSAGE("ERROR CLI UI requested, but cannot find header file for avidemux/cli...disabling it")
            SET(DO_CLI 0)
        ENDIF(NOT ADM_INC)

		MESSAGE("")
endif(DO_CLI)

APPEND_SUMMARY_LIST("User Interface" "CLI" "${DO_CLI}")

########################################
# Actual work to do..
########################################
IF(DO_COMMON)
    ADD_SUBDIRECTORY(ADM_audioDecoders)
    ADD_SUBDIRECTORY(ADM_audioDevices)
    ADD_SUBDIRECTORY(ADM_videoDecoder)
    ADD_SUBDIRECTORY(ADM_audioEncoders)
    ADD_SUBDIRECTORY(ADM_demuxers)
    ADD_SUBDIRECTORY(ADM_muxers)
    ADD_SUBDIRECTORY(ADM_autoScrips)
    add_subdirectory(ADM_scriptEngines)
ENDIF(DO_COMMON)

ADD_SUBDIRECTORY(ADM_videoEncoder)
ADD_SUBDIRECTORY(ADM_videoFilters6)
IF(DO_QT4)
        add_subdirectory(ADM_demuxers/VapourSynth)
        add_subdirectory(ADM_scriptEngines)
        #IF(USE_OPENGL AND NOT WIN32)
        IF(USE_OPENGL )
         ADD_SUBDIRECTORY(ADM_videoFilters6_openGl)
        ENDIF(USE_OPENGL )
        #ENDIF(USE_OPENGL AND NOT WIN32)
ENDIF(DO_QT4)

########################################
# Config Summary
########################################
DISPLAY_SUMMARY_LIST()

include(admPackager)
admPackager(pluginsPackage)
MESSAGE("")
