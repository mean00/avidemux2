CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(PLUGINS TRUE)
IF(POLICY CMP0177)
  CMAKE_POLICY(SET CMP0177 OLD)
ENDIF()
# for MSVC we use vcpk, it is complicated way of detecting it
# because we have to do it before PROJECT(...)
IF(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  SET(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  MESSAGE(STATUS "Using MSVC VCPKG setup <${CMAKE_TOOLCHAIN_FILE}>")
ENDIF()

PROJECT(Avidemux_Plugins)
MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for Avidemux Plugins Started")
MESSAGE("#########################################")
MESSAGE("")
MESSAGE(STATUS "Checking for avidemux development files ..")

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
IF(NOT FAKEROOT)
  SET(AVIDEMUX_FAKEROOT "")
ELSE()
  SET(AVIDEMUX_FAKEROOT "${FAKEROOT}")
ENDIF()
INCLUDE(avidemux_plugin_api)
SET(ADM_HEADER_DIR ${AVIDEMUX_FAKEROOT}${CMAKE_INSTALL_PREFIX}/include/avidemux/${AVIDEMUX_API_VERSION})
SET(ADM_CMAKE_DIR  ${ADM_HEADER_DIR}/cmake)
LIST(APPEND CMAKE_MODULE_PATH ${ADM_CMAKE_DIR})
#
# Sanity check
#
MESSAGE(STATUS  "Checking if avidemux include folder exists as  ${ADM_HEADER_DIR}")
MESSAGE(STATUS  "  ( CMAKE_INSTALL_PREFIX/include/avidemux/${AVIDEMUX_API_VERSION}")
IF(NOT EXISTS "${ADM_HEADER_DIR}")
  MESSAGE(STATUS  "Make sure you installed all the files.\n i cannot find avidemux include folder.cmake .\nSet CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
  MESSAGE(FATAL_ERROR  "Aborting")
ENDIF()

MESSAGE(STATUS "Found avidemux include folder. good.")
MESSAGE(STATUS "Checking for cmake subfolder")

IF(NOT EXISTS "${ADM_CMAKE_DIR}/commonCmakeApplication.cmake")
  MESSAGE(STATUS  "Make sure you installed all the files.\n I cannot find content of the cmake subfolder .\n")
  MESSAGE(STATUS  "Set CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
  MESSAGE(STATUS  "I was looking for commonCmakeApplication.cmake in  ${ADM_CMAKE_DIR}")
  MESSAGE(FATAL_ERROR  "Aborting")
ENDIF()
MESSAGE(STATUS "Found cmake subfolder. good.")

#
# Go
#
INCLUDE(avidemuxVersion)
MESSAGE(STATUS "Building agains version ${AVIDEMUX_VERSION}.")

IF(WIN32)
  #IF(MSVC)
  #  INCLUDE(adm_win32_vs.cmake)
  #ELSE()
  #  INCLUDE(adm_win32_mingw.cmake)
  #ENDIF()
ELSE()
  IF(APPLE)
    # Silence missing nullability specifier warnings on macOS
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nullability-completeness")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-completeness")
    INCLUDE(adm_osx)
  ELSE()
    INCLUDE(adm_default)
  ENDIF()
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Where is the top dir ?
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Common definitions...
INCLUDE(commonCmakeApplication)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add include dirs
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
SET(AVIDEMUX_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
MARK_AS_ADVANCED(AVIDEMUX_INSTALL_DIR)
INCLUDE(admInstallDir)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Get regular stuff from main app
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
INCLUDE(admCoreIncludes)
LINK_DIRECTORIES("${AVIDEMUX_SEARCH_LIB_DIR}")
INCLUDE_DIRECTORIES("${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Unbundle some libraries here
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
OPTION(USE_EXTERNAL_LIBMAD "Use system installed libmad library." OFF)
OPTION(USE_EXTERNAL_LIBA52 "Use system installed liba52 library." OFF)
OPTION(USE_EXTERNAL_MP4V2 "Use system installed libmp4v2 library." OFF)

INCLUDE(FindPkgConfig)

# libass
PKG_CHECK_MODULES(LIBASS REQUIRED libass)
INCLUDE_DIRECTORIES(SYSTEM ${LIBASS_INCLUDE_DIRS})

# libmad
IF(USE_EXTERNAL_LIBMAD)
  FIND_PATH(LIBMAD_INCLUDE_DIR mad.h)
  FIND_LIBRARY(LIBMAD_LIBRARIES mad)
  INCLUDE_DIRECTORIES(SYSTEM ${LIBMAD_INCLUDE_DIRS})
ENDIF()

# liba52
IF(USE_EXTERNAL_LIBA52)
  FIND_PATH(LIBA52_INCLUDE_DIR a52dec/a52.h)
  FIND_LIBRARY(LIBA52_LIBRARIES a52)
  INCLUDE_DIRECTORIES(SYSTEM ${LIBA52_INCLUDE_DIR})
ENDIF()

# libmp4v2
#IF(USE_EXTERNAL_MP4V2)
#FIND_PATH(MP4V2_INCLUDE_DIR mp4v2/mp4v2.h)
#FIND_LIBRARY(MP4V2_LIBRARIES mp4v2)
#INCLUDE_DIRECTORIES(SYSTEM ${MP4V2_INCLUDE_DIR})
#ENDIF()

IF(FRESH_BUILD)
  MESSAGE("")
ENDIF()
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# retrieve core config, needed to get USE_VDPAU and friends
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
INCLUDE(ADM_coreConfig)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Global options
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
OPTION(VERBOSE "" OFF)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Select our UI
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
SET(DO_GTK    0)
SET(DO_QT4    0)
SET(DO_COMMON 0)
SET(DO_CLI    0)
SET(DO_SETTINGS    0)

IF(NOT PLUGIN_UI)
  MESSAGE(FATAL_ERROR "Please add -DPLUGIN_UI=[ALL|QT4|GTK|CLI|COMMON|SETTINGS]")
ENDIF()

IF(PLUGIN_UI MATCHES "ALL")
  SET(DO_COMMON 1)
  SET(DO_GTK    1)
  SET(DO_QT4    1)
  SET(DO_CLI    1)
  SET(DO_SETTINGS    1)
ENDIF()

IF(PLUGIN_UI MATCHES "QT4")
  SET(DO_QT4    1)
ENDIF()

IF(PLUGIN_UI MATCHES "GTK")
  SET(DO_GTK    1)
ENDIF()

IF(PLUGIN_UI MATCHES "COMMON")
  SET(DO_COMMON    1)
ENDIF()

IF(PLUGIN_UI MATCHES "CLI")
  SET(DO_CLI    1)
ENDIF()

IF(PLUGIN_UI MATCHES "SETTINGS")
  SET(DO_SETTINGS    1)
ENDIF()


MESSAGE(STATUS "Plugin UI : ${PLUGIN_UI}")
MESSAGE("")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Check for UI libraries
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
MACRO(CHECK_ADM_CONFIG_H flavor result)
  MESSAGE(STATUS "Checking for avidemux/${flavor} includes and lib...")
  MESSAGE(STATUS "It should be ${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/${flavor}/config.h")
  SET(ADM_INC "NOTFOUND")
  FIND_PATH(ADM_INC config.h "${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/${flavor}")
  #MESSAGE(STATUS "result : ${ADM_INC_NOT_FOUND},${ADM_INC}")
  IF(${ADM_INC} MATCHES "NOTFOUND")
    SET(${result} 0)
    MESSAGE(STATUS "Include for ${flavor} not found")
  ELSE()
    SET(${result} 1)
    MESSAGE(STATUS "Include for ${flavor} found")
  ENDIF()
ENDMACRO()

APPEND_SUMMARY_LIST("User Interface" "Common" "${DO_COMMON}")

INCLUDE(admCheckGettext)
checkGettext()

#*****
# GTK
#*****
IF(DO_GTK)
  INCLUDE(admCheckGtk)
  checkGtk()
  IF(GTK_FOUND AND GTHREAD_FOUND)
    MESSAGE(STATUS "Gtk and gthread found, good")
    CHECK_ADM_CONFIG_H(gtk ADM_INC)
    IF(NOT ADM_INC)
      MESSAGE("ERROR GTK UI requested, but cannot find header file for avidemux/gtk...disabling it")
      MESSAGE("")
      SET(DO_GTK 0)
    ENDIF()
  ELSE()
    MESSAGE("ERROR GTK UI requested, but gtk/gthread libraries not found... disabling it")
    MESSAGE("")
    SET(DO_GTK 0)
  ENDIF()
ENDIF()

APPEND_SUMMARY_LIST("User Interface" "GTK+" "${DO_GTK}")

#*****
# QT4
#*****
IF(DO_QT4)
  INCLUDE(admCheckQt)
  checkQt()
  IF(NOT QT_FOUND)
    SET(DO_QT4 0)
    MESSAGE("ERROR QT4 type UI requested, but Qt libraries not found... disabling it")
    MESSAGE("")
  ELSE()
    MESSAGE(STATUS "Qt libraries found, good")
    CHECK_ADM_CONFIG_H(${QT_EXTENSION} ADM_INC)
    MESSAGE("")

    INCLUDE(admCheckOpenGl)
    IF(NOT ADM_INC)
      MESSAGE("ERROR QT4 type UI requested, but cannot find header files from avidemux/qt4... disabling it")
      MESSAGE("")
      SET(DO_QT4 0)
    ENDIF()
  ENDIF()
ENDIF()

APPEND_SUMMARY_LIST("User Interface" "Qt" "${DO_QT4}")

#*****
# CLI
#*****
IF(DO_CLI)
  MESSAGE(STATUS "Checking for CLI")
  MESSAGE(STATUS "****************")

  CHECK_ADM_CONFIG_H(cli ADM_INC)

  IF(NOT ADM_INC)
    MESSAGE("ERROR CLI type UI requested, but cannot find header files from avidemux/cli... disabling it")
    SET(DO_CLI 0)
  ENDIF()

  MESSAGE("")
ENDIF()
#
APPEND_SUMMARY_LIST("User Interface" "CLI" "${DO_CLI}")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Actual work to do..
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
IF(DO_COMMON)
  ADD_SUBDIRECTORY(ADM_audioDecoders)
  ADD_SUBDIRECTORY(ADM_audioDevices)
  ADD_SUBDIRECTORY(ADM_videoDecoder)
  ADD_SUBDIRECTORY(ADM_audioEncoders)
  ADD_SUBDIRECTORY(ADM_demuxers)
  ADD_SUBDIRECTORY(ADM_muxers)
  ADD_SUBDIRECTORY(ADM_autoScrips)
  ADD_SUBDIRECTORY(ADM_scriptEngines)
  IF(APPLE)
    APPEND_SUMMARY_LIST("Video Encoder" "VideoToolbox" "${USE_VIDEOTOOLBOX}")
  ELSE()
    APPEND_SUMMARY_LIST("Video Encoder" "NVENC" "${USE_NVENC}")
    IF(UNIX)
      APPEND_SUMMARY_LIST("Video Encoder" "VAAPI H264" "${USE_LIBVA}")
      APPEND_SUMMARY_LIST("Video Encoder" "VAAPI HEVC" "${LIBVA_HEVC_DEC}")
      APPEND_SUMMARY_LIST("Video Encoder" "VAAPI AV1" "${LIBVA_AV1_DEC}")
      APPEND_SUMMARY_LIST("Miscellaneous" "VDPAU" "${USE_VDPAU}")
    ENDIF()
  ENDIF()
ENDIF()

ADD_SUBDIRECTORY(ADM_videoEncoder)
ADD_SUBDIRECTORY(ADM_videoFilters6)
IF(DO_QT4)
  IF(NOT DO_COMMON)
    ADD_SUBDIRECTORY(ADM_demuxers/VapourSynth)
  ENDIF()
  #    add_subdirectory(ADM_scriptEngines)
  #IF(USE_OPENGL AND NOT WIN32)
  IF(USE_OPENGL)
    ADD_SUBDIRECTORY(ADM_videoFilters6_openGl)
  ENDIF()
  #ENDIF()
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Config Summary
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
DISPLAY_SUMMARY_LIST()
MACRO(ADD_DUMMY condt flder)
  IF(${${condt}})  # # Do a dummy install in case there is nothing...
    MESSAGE(STATUS "Adding dummy install file (${condt}:${${condt}})")
    INSTALL(FILES cmake/FindAlsa.cmake
    DESTINATION ${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/dummy_${flder}
    COMPONENT runtime
  )
  ENDIF()
ENDMACRO()
#
MESSAGE(STATUS "Adding dummy installation files")
ADD_DUMMY(DO_CLI cli)
ADD_DUMMY(DO_GTK gtk)
ADD_DUMMY(DO_QT4 qt4)
ADD_DUMMY(DO_SETTING settings)
#
INCLUDE(admPackager)
admPackager(pluginsPackage)
MESSAGE("")
