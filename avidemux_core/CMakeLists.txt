CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for MSVC we use vcpk, it is complicated way of detecting it
# because we have to do it before PROJECT(...)
IF(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  SET(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  MESSAGE(STATUS "Using MSVC VCPKG setup <${CMAKE_TOOLCHAIN_FILE}>")
ENDIF()

PROJECT(admCore)

MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for avidemux core libs Started")
MESSAGE("#########################################")
MESSAGE("")

# need for shared cmake script between avidemux & plugins
SET(AVIDEMUX_CORE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
SET(AVIDEMUX_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR})
SET(AVIDEMUX_CORE_BUILD_DIR ${CMAKE_BINARY_DIR})
SET(AVIDEMUX_THIS_IS_CORE True)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_win32_rc")

IF(WIN32)
  SET(ADM_PLUGIN_DIR "plugins")
ELSE()
  SET(ADM_PLUGIN_DIR "ADM_plugins6")
ENDIF()

ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)

INCLUDE(admConfigSummary)
INITIALISE_SUMMARY_LISTS()

INCLUDE(admMainChecks)
INCLUDE(admCheckFtello)
INCLUDE(admAsNeeded)
INCLUDE(admCheckVDPAU)
INCLUDE(admCheckLibVA)
INCLUDE(admCheckVideoToolbox)
INCLUDE(admCheckSqlLite)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Debug Summary
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
IF(VERBOSE)
  INCLUDE(adm_log)
  MESSAGE("")
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Checking if we should use ftello
# or ftello64 (win32)
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
checkFtello()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# sqllite
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
checkSqlite3()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #"
# Check for DXVA2
# We do it here, cmake messes things up when
# we use try_compile later on, so do it once here
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #"

IF(WIN32) # AND NOT MSVC)
  INCLUDE(admCheckDxva2)
ELSE()
  IF(UNIX AND NOT APPLE)  # linux /bsd/...
    checkVDPAU()
    checkLibVA()
  ELSE() # Apple
    checkVideoToolbox()
    # Silence missing nullability specifier warnings on macOS
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nullability-completeness")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-completeness")
  ENDIF()
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Generate config.h
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
CONFIGURE_FILE("${AVIDEMUX_CORE_SOURCE_DIR}/cmake/ADM_coreConfig.h.cmake" "${CMAKE_BINARY_DIR}/config/ADM_coreConfig.h")
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/config")
MESSAGE(STATUS "ADM_coreConfig.h generated")
MESSAGE("")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add include dirs
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
SET(EXCLUDE_ADM_INCLUDE TRUE)
INCLUDE(admCoreIncludes)
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Build FFmpeg
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
INCLUDE(admFFmpegBuild)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Directories to build
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
SET(SUBFOLDERS
    ADM_core
    ADM_coreAudioParser
    ADM_coreAudio
    ADM_coreAudioFilter
    ADM_coreAudioDevice
    ADM_coreAudioEncoder
    ADM_coreDemuxer
    ADM_coreDemuxerMpeg
    ADM_coreImage
    ADM_coreMuxer
    ADM_coreUI
    ADM_coreUtils
    ADM_coreVideoFilter
    ADM_coreSqlLight3
    ADM_coreJobs
    ADM_coreSocket
    ADM_coreImageLoader
    ADM_coreScript
    ADM_coreSubtitles
)
FOREACH(subfolder ${SUBFOLDERS})
  ADD_SUBDIRECTORY(${subfolder}/src)
ENDFOREACH()

ADD_SUBDIRECTORY(ADM_coreVideoEncoder)
ADD_SUBDIRECTORY(ADM_coreVideoCodec)

# Update nsi version now
IF(WIN32)
  SET(NSI_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../avidemux/winInstaller/AvidemuxVersion.nsh")
  FILE(WRITE   ${NSI_FILE} "!define CORE_VERSION \"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}\"\n")
  FILE(APPEND  ${NSI_FILE} "!define POINT_RELEASE \"${CPACK_PACKAGE_VERSION_P}\"\n")
  ADM_TIMESTAMP(date)
  FILE(APPEND  ${NSI_FILE} "!define SVN_VERSION \"${date}\"\n")
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Config Summary
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
DISPLAY_SUMMARY_LIST()

INCLUDE(admPackager)
admPackager(corePackage)

SET(HW_ACCEL_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/ADM_coreVideoCodec/ADM_hwAccel/")

# install headers
FOREACH(subfolder ${SUBFOLDERS} ADM_coreAudioCodec ADM_coreVideoCodec ADM_coreVideoEncoder)
  ADM_INSTALL_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/${subfolder}/include/" ${subfolder})
ENDFOREACH()

IF(USE_VDPAU)
  ADM_INSTALL_INCLUDE_FOLDER("${HW_ACCEL_FOLDER}/ADM_coreVdpau/include/" ADM_coreVdpau)
ENDIF()
IF(USE_LIBVA)
  ADM_INSTALL_INCLUDE_FOLDER("${HW_ACCEL_FOLDER}/ADM_coreLibVA/include/" ADM_coreLibVA)
  ADM_INSTALL_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_coreVideoEncoder/ADM_hwAccelEncoder/ADM_coreLibVAEnc/include/" ADM_coreLibVAEnc )
ENDIF()
IF(USE_DXVA2)
  ADM_INSTALL_INCLUDE_FOLDER("${HW_ACCEL_FOLDER}/ADM_coreDxva2/include/" ADM_coreDxva2)
ENDIF()
IF(USE_VIDEOTOOLBOX)
  ADM_INSTALL_INCLUDE_FOLDER("${HW_ACCEL_FOLDER}/ADM_coreVideoToolbox/include/" ADM_coreVideoToolbox)
ENDIF()

ADM_INSTALL_CMAKE_FOLDER("${AVIDEMUX_CORE_SOURCE_DIR}/cmake")
ADM_INSTALL_CMAKE_HELPER_FOLDER("${AVIDEMUX_CORE_SOURCE_DIR}/cmake/" "cmake_compile_check/")
ADM_INSTALL_CMAKE_HELPER_FOLDER("${AVIDEMUX_CORE_SOURCE_DIR}/cmake/" "cmake_win32_rc/")
#

INSTALL(FILES ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.h DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}" COMPONENT dev)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/ADM_coreConfig.cmake DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/cmake/" COMPONENT dev)

IF(MSVC)
  SET(FOREIGN_MSVC ${CMAKE_SOURCE_DIR}/../foreignBuilds/msvc/include/)
  INSTALL(FILES
    ${FOREIGN_MSVC}/stdint.h
    ${FOREIGN_MSVC}/inttypes.h
    ${FOREIGN_MSVC}/GL/glext.h
  DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/GL" COMPONENT dev)
  # install runtimes also
  SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${AVIDEMUX_BIN_DIR})
  INCLUDE(InstallRequiredSystemLibraries)
ENDIF()

MESSAGE("")
