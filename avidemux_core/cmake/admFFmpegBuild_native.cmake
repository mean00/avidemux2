

INCLUDE(admFFmpegBuild_helpers)

#@@
ADM_FF_SET_DEFAULT()

IF(USE_NVENC)
  SET(FFMPEG_ENCODERS ${FFMPEG_ENCODERS} nvenc)
  SET(FFMPEG_DECODERS ${FFMPEG_DECODERS} nvdec)
  xadd("--enable-nvenc")
  xadd("--enable-nvdec")
  xadd("--extra-cflags=-I${NVENC_INCLUDE_DIR}")
  SET(FFMPEG_ENCODERS  ${FFMPEG_ENCODERS} h264_nvenc hevc_nvenc)
ENDIF()

#@@
ADM_FF_PATCH_IF_NEEDED()

IF(FFMPEG_PERFORM_PATCH)
  MESSAGE(STATUS "Patching Linux common.mak")
  patch_file("${FFMPEG_SOURCE_DIR}" "${FFMPEG_PATCH_DIR}/common.mak.diff")
ENDIF()

xadd(--enable-pthreads)
# help for debugging xadd(--disable-optimizations)

IF(USE_VDPAU)
  xadd(--enable-vdpau)
ENDIF()

IF(USE_LIBVA)
  xadd(--enable-vaapi)
  SET(FFMPEG_DECODERS ${FFMPEG_DECODERS} h264_vaapi hevc_vaapi av1_vaapi)
  SET(FFMPEG_ENCODERS ${FFMPEG_ENCODERS} h264_vaapi hevc_vaapi av1_vaapi)
ENDIF()

IF(USE_VIDEOTOOLBOX)
  xadd(--enable-videotoolbox)
  SET(FFMPEG_ENCODERS ${FFMPEG_ENCODERS} h264_videotoolbox hevc_videotoolbox)
ENDIF()

IF(APPLE AND "$ENV{MACOSX_DEPLOYMENT_TARGET}" VERSION_EQUAL "10.15")
  xadd(--extra-cflags="-fno-stack-check") # see https://trac.ffmpeg.org/ticket/8073
ENDIF()

IF(NOT APPLE AND NOT ADM_CPU_X86_32 AND NOT "${CMAKE_C_COMPILER}" MATCHES ".*[cC]lang.*")
  xadd(--enable-lto)
ENDIF()

#@@
ADM_FF_ADD_OPTIONS()

xadd(--cc "${CMAKE_C_COMPILER}")
xadd(--ld "${CMAKE_C_COMPILER}")
xadd(--ar "${CMAKE_AR}")

#@@
ADM_FF_SET_EXTRA_FLAGS()

#@@
ADM_FF_BUILD_UNIX_STYLE()
#
ADM_FF_ADD_DUMMY_TARGET()

IF(APPLE)
  MESSAGE(STATUS "Patching config.mak - mac")
  patch_file("${FFMPEG_BINARY_DIR}" "${FFMPEG_PATCH_DIR}/config.mak.mac.diff")
ELSE()
  MESSAGE(STATUS "Patching Linux config.mak")
  patch_file("${FFMPEG_BINARY_DIR}" "${FFMPEG_PATCH_DIR}/config.mak.diff")
ENDIF()
#
MACRO(FF_ADD_SUBLIB lib)
  ADD_CUSTOM_COMMAND(
                   OUTPUT       "${lib}"
           DEPENDS     libavutil_dummy
                   COMMAND ${BASH_EXECUTABLE} -c echo "placeHolder")

ENDMACRO()

FF_ADD_SUBLIB("${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}")
FF_ADD_SUBLIB("${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}")
FF_ADD_SUBLIB("${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}")
FF_ADD_SUBLIB("${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}")
FF_ADD_SUBLIB("${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}")

ADM_FF_INSTALL_LIBS_AND_HEADERS()

IF(USE_LIBVA)
  #INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/vaapi.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
  #INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/vaapi_internal.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
ENDIF()
# EOF
