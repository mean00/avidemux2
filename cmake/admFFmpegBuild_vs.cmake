include(admFFmpegBuild_helpers)

#@@
ADM_FF_SET_DEFAULT()


#@@
ADM_FF_PATCH_IF_NEEDED()




#@@
ADM_FF_ADD_OPTIONS()


xadd(--enable-w32threads)
#
xadd(--toolchain=msvc)

#  Cross compiler override (win32 & win64)
xadd(--extra-cflags  -I${VS_ROOT}/include)

message(STATUS "Using VS compilation flag: ${FFMPEG_FLAGS}")

ADM_FF_ADD_OPTIONS()



ADM_FF_SET_EXTRA_FLAGS()


IF(USE_DXVA2)
      xadd(--enable-dxva2)
      set(FFMPEG_DECODERS ${FFMPEG_DECODERS} h264_dxva2 hevc_dxva2)
ENDIF(USE_DXVA2)

#@@
ADM_FF_BUILD_UNIX_STYLE()
MACRO(FF_ADD_SUBLIB lib)
        add_custom_command(
				   OUTPUT       "${lib}"
                                   DEPENDS 	libavutil_dummy
				   COMMAND ${BASH_EXECUTABLE} -c echo "placeHolder")

ENDMACRO(FF_ADD_SUBLIB lib)

FF_ADD_SUBLIB(     	"${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}"       )
FF_ADD_SUBLIB(     	"${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}"       )
FF_ADD_SUBLIB(          "${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}"   )
FF_ADD_SUBLIB(          "${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}"   )
FF_ADD_SUBLIB(          "${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}"     )


MACRO(FF_ADD_SUBLIB lib)
        add_custom_command(
				   OUTPUT       "${lib}"
                                   DEPENDS 	"${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}"
				   COMMAND ${BASH_EXECUTABLE} -c echo "placeHolder")

ENDMACRO(FF_ADD_SUBLIB lib)

FF_ADD_SUBLIB(     	"${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}"       )
FF_ADD_SUBLIB(          "${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}"   )
FF_ADD_SUBLIB(          "${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}"   )
FF_ADD_SUBLIB(          "${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}"     )

ADM_FF_INSTALL_LIBS_AND_HEADERS()

IF(USE_DXVA2)
        INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/dxva2.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavcodec" COMPONENT dev)
        INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/dxva2_internal.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavcodec" COMPONENT dev)
ENDIF(USE_DXVA2)

#
#
#
