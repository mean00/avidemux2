cmake_minimum_required(VERSION 2.6)

MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for avidemux core libs Started")
MESSAGE("#########################################")
MESSAGE("")

# need for shared cmake script between avidemux & plugins
SET(AVIDEMUX_SOURCE_DIR ${CMAKE_SOURCE_DIR})
SET(AVIDEMUX_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)
SET(AVIDEMUX_CORE_BUILD_DIR ${CMAKE_BINARY_DIR})
SET(AVIDEMUX_THIS_IS_CORE True)
SET(CMAKE_MODULE_PATH "${AVIDEMUX_TOP_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
SET(ADM_PROJECT admCore)

IF (WIN32)
	SET(ADM_PLUGIN_DIR "plugins")
ELSE (WIN32)
	SET(ADM_PLUGIN_DIR "ADM_plugins6")
ENDIF (WIN32)

ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)

include(admConfigSummary)
INITIALISE_SUMMARY_LISTS()

include(admMainChecks)
include(admCheckFtello)
include(admAsNeeded)

#########################################
# Debug Summary
########################################
IF (VERBOSE)
	INCLUDE(adm_log)
	MESSAGE("")
ENDIF (VERBOSE)

########################################
# Checking if we should use ftello
# or ftello64 (win32)
########################################
checkFtello()

########################################
# sqllite
########################################
MESSAGE(STATUS "Checking for sqlite")
MESSAGE(STATUS "*******************")

IF (UNIX)
	SET(SQLITE_REQUIRED_FLAGS "-ldl")
ENDIF (UNIX)

FIND_HEADER_AND_LIB(SQLITE sqlite3.h sqlite3 sqlite3_close ${SQLITE_REQUIRED_FLAGS})
PRINT_LIBRARY_INFO("SQLite" SQLITE_FOUND "${SQLITE_INCLUDE_DIR}" "${SQLITE_LIBRARY_DIR}" FATAL_ERROR)
MESSAGE("")

########################################
# Generate config.h
########################################
CONFIGURE_FILE("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/ADM_coreConfig.h.cmake" "${CMAKE_BINARY_DIR}/config/ADM_coreConfig.h")
include_directories( "${CMAKE_BINARY_DIR}/config")
MESSAGE(STATUS "ADM_coreConfig.h generated")
MESSAGE("")

########################################
# Add include dirs
########################################
set(EXCLUDE_ADM_INCLUDE TRUE)
include(admCoreIncludes)

########################################
# Build FFmpeg
########################################
include(admFFmpegBuild)

########################################
# Directories to build
########################################
SET(SUBFOLDERS
    ADM_core
    ADM_coreAudioParser
    ADM_coreAudio
    ADM_coreAudioFilter
    ADM_coreAudioDevice
    ADM_coreAudioEncoder
    ADM_coreDemuxer
    ADM_coreDemuxerMpeg
    ADM_coreVideoCodec
    ADM_coreImage
    ADM_coreMuxer
    ADM_coreUI
    ADM_coreUtils
    ADM_coreVideoEncoder
    ADM_coreVideoFilter
    ADM_coreSqlLight3
    ADM_coreJobs
    ADM_coreSocket
    ADM_coreImageLoader
    ADM_coreScript
    ADM_coreSubtitles
)
FOREACH( subfolder ${SUBFOLDERS})
   ADD_SUBDIRECTORY(${subfolder})
ENDFOREACH( subfolder ${SUBFOLDERS})

# Update nsi version now
IF(WIN32)
        SET(NSI_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../avidemux/winInstaller/AvidemuxVersion.nsh")
        FILE(WRITE   ${NSI_FILE} "!define CORE_VERSION \"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}\"\n")
        FILE(APPEND  ${NSI_FILE} "!define POINT_RELEASE \"${CPACK_PACKAGE_VERSION_P}\"\n")
        ADM_TIMESTAMP(date)
        FILE(APPEND  ${NSI_FILE} "!define SVN_VERSION \"${date}\"\n")
ENDIF(WIN32)

########################################
# Config Summary
########################################
DISPLAY_SUMMARY_LIST()

include(admPackager)
admPackager(corePackage)

# install headers
FOREACH( subfolder ${SUBFOLDERS} ADM_coreAudioCodec)
   ADM_INSTALL_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/${subfolder}/include/" ${subfolder})     
ENDFOREACH( subfolder ${SUBFOLDERS} ADM_coreAudioCodec)

IF(USE_VDPAU)
   ADM_INSTALL_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_coreVideoCodec/ADM_hwAccel/ADM_coreVdpau/include/" ADM_coreVdpau )     
ENDIF(USE_VDPAU)
IF(USE_LIBVA)
   ADM_INSTALL_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_coreVideoCodec/ADM_hwAccel/ADM_coreLibVA/include/" ADM_coreLibVA )     
ENDIF(USE_LIBVA)
IF(USE_DXVA2)
   ADM_INSTALL_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_coreVideoCodec/ADM_hwAccel/ADM_coreDxva2/include/" ADM_coreDxva2 )     
ENDIF(USE_DXVA2)

SET(CMAKE_TOP_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../cmake/")

ADM_INSTALL_CMAKE_FOLDER("${CMAKE_TOP_SOURCE_DIR}/" cmake)     
ADM_INSTALL_CMAKE_HELPER_FOLDER("${CMAKE_SOURCE_DIR}/../cmake/cmake_compile_check/" cmake_compile_check)     
# 

INSTALL(FILES ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6" COMPONENT dev)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.cmake DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6" COMPONENT dev)
MESSAGE("")
