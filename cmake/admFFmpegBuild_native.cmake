

include(admFFmpegBuild_helpers)

#@@
ADM_FF_SET_DEFAULT()

IF(USE_NVENC)
   SET(FFMPEG_ENCODERS ${FFMPEG_ENCODERS} nvenc)
   xadd("--enable-nvenc")
   xadd("--extra-cflags=-I${NVENC_INCLUDE_DIR}")
   set(FFMPEG_ENCODERS  ${FFMPEG_ENCODERS} nvenc_h264 nvenc_hevc)
ENDIF(USE_NVENC)

#@@
ADM_FF_PATCH_IF_NEEDED()

if (FFMPEG_PERFORM_PATCH)
    MESSAGE(STATUS "Patching Linux common.mak")
    patch_file("${FFMPEG_SOURCE_DIR}" "${FFMPEG_PATCH_DIR}/common.mak.diff")
endif (FFMPEG_PERFORM_PATCH)

xadd(--enable-pthreads)
# help for debugging xadd(--disable-optimizations)

if (USE_VDPAU)
    xadd(--enable-vdpau)
    set(FFMPEG_DECODERS ${FFMPEG_DECODERS} h264_vdpau  vc1_vdpau  mpeg1_vdpau  mpeg_vdpau  wmv3_vdpau)
endif (USE_VDPAU)

if (USE_LIBVA)
    xadd(--enable-vaapi)
    set(FFMPEG_DECODERS ${FFMPEG_DECODERS} h264_vaapi hevc_vaapi)
    set(FFMPEG_ENCODERS ${FFMPEG_ENCODERS} h264_vaapi hevc_vaapi)
endif (USE_LIBVA)

if (USE_VIDEOTOOLBOX)
    xadd(--enable-videotoolbox)
    set(FFMPEG_DECODERS ${FFMPEG_DECODERS} h263_videotoolbox h264_videotoolbox mpeg1_videotoolbox mpeg2_videotoolbox mpeg4_videotoolbox)
    set(FFMPEG_ENCODERS ${FFMPEG_ENCODERS} h264_videotoolbox)
endif (USE_VIDEOTOOLBOX)

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
ELSE(APPLE)
        MESSAGE(STATUS "Patching Linux config.mak")
        patch_file("${FFMPEG_BINARY_DIR}" "${FFMPEG_PATCH_DIR}/config.mak.diff")
ENDIF(APPLE)
#
MACRO(FF_ADD_SUBLIB lib)
            add_custom_command(
                   OUTPUT       "${lib}"
           DEPENDS     libavutil_dummy
                   COMMAND ${BASH_EXECUTABLE} -c echo "placeHolder")

ENDMACRO(FF_ADD_SUBLIB lib)

FF_ADD_SUBLIB(         "${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}"       )
FF_ADD_SUBLIB(         "${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}"       )
FF_ADD_SUBLIB(      "${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}"   )
FF_ADD_SUBLIB(      "${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}"   )
FF_ADD_SUBLIB(      "${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}"     )

ADM_FF_INSTALL_LIBS_AND_HEADERS()

IF(USE_LIBVA)
                INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/vaapi.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
                #INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/vaapi_internal.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
ENDIF(USE_LIBVA)
# EOF
