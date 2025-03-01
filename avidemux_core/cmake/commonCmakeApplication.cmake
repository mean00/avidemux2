# # # # # # # # # # # # # # # # # # # # # # # # ################
# Definitions and Includes
# # # # # # # # # # # # # # # # # # # # # # # # ################

# Common definitions...
SET(CMAKE_MODULE_PATH "${ADM_CMAKE_DIR}/cmake_win32_rc" "${CMAKE_MODULE_PATH}")
MESSAGE(STATUS "Cmake module path = ${CMAKE_MODULE_PATH}")
#INCLUDE(admPluginLocation)
INCLUDE(admConfigSummary)
INITIALISE_SUMMARY_LISTS()

# # # # # # # # # # # # # # # # # # # # # # # # ################
# Shared cmake part
# # # # # # # # # # # # # # # # # # # # # # # # ################
#
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_EXTENSIONS OFF)
#
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)
INCLUDE(admMainChecks)

UNSET(CMAKE_CXX_VISIBILITY_PRESET)
UNSET(CMAKE_VISIBILITY_INLINES_HIDDEN)

# # # # # # # # # # # # # # # # # # # # # # # # ################
# Add include dirs
# # # # # # # # # # # # # # # # # # # # # # # # ################
INCLUDE(admCoreIncludes)
# Add ffmpeg to resolve ADM_libavcodec to the actual name, needed for vdpau
INCLUDE(admFFmpegUtil)
registerFFmpeg("${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}" "${AVIDEMUX_SEARCH_LIB_DIR}" 1)
# Verify ADM_coreConfig is there
IF(NOT EXISTS "${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/ADM_coreConfig.h")
  MESSAGE(FATAL_ERROR "CMAKE_INSTALL_PREFIX does not contain include/avidemux/${AVIDEMUX_MAJOR_MINOR}/ADM_coreConfig.h (${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/ADM_coreConfig.h)")
ENDIF()

LINK_DIRECTORIES("${AVIDEMUX_SEARCH_LIB_DIR}")
#

include(ADM_coreConfig)

#
# Add main source
#
SET(ADM_EXE_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/ADM_preview.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/ADM_previewNavigate.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/gui_main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/gui_autodrive.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/gui_navigate.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/gui_play.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/gui_save.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/gui_savenew.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/gui_action.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/gui_blackframes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/ADM_gettext.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/ADM_slave.cpp
)

# # # # # # # # # # # # # # # # # # # # # # # # ################
# FFmpeg
# # # # # # # # # # # # # # # # # # # # # # # # ################
INCLUDE_DIRECTORIES("${ADM_HEADER_DIR}/ffmpeg")

# # # # # # # # # # # # # # # # # # # # # # # # #####################
# Add core libs
# # # # # # # # # # # # # # # # # # # # # # # # #####################
SET(coreLibs
  ADM_core6
  ADM_audioParser6
  ADM_coreAudio6
  ADM_coreAudioFilterAPI6
  ADM_coreAudioDevice6
  ADM_coreAudioEncoder6
  ADM_coreDemuxer6
  ADM_coreImage6
  ADM_coreMuxer6
  ADM_coreUI6
  ADM_coreUtils6
  ADM_coreSocket6
  ADM_coreVideoEncoder6
  ADM_coreVideoFilter6
)

# # # # # # # # # # # # # # # # # # # # # # # # #####################
# Add common libs
# # # # # # # # # # # # # # # # # # # # # # # # #####################
SET(commonLibs1
  ADM_muxerGate6
  ADM_audioFilter6
  ADM_editor6
  ADM_audiocodec6
  ADM_videoCodec6
  ADM_coreVideoCodec6
  ADM_ui_interface
)
#
MACRO(ADD_LIB_IF cond lib)
  IF(${cond})
    LIST(APPEND commonLibs1 ${lib} ${ARGN})
  ENDIF()
ENDMACRO()

ADD_LIB_IF(USE_DXVA2 ADM_coreDxva26)
ADD_LIB_IF(USE_VDPAU ADM_coreVDPAU6)
ADD_LIB_IF(USE_XVBA ADM_coreXvba6)
ADD_LIB_IF(USE_LIBVA ADM_coreLibVA6 ADM_coreLibVAEnc6)

LIST(APPEND commonLibs1 ADM_libavcodec ADM_libavutil)

SET(commonLibs2
  ADM_coreJobs
  ADM_osSupport6
  ADM_script6
  ADM_videoEncoder6
  ADM_internalVideoFilter6
  ADM_toolkit6
)

# END
