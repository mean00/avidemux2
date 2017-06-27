include(admFFmpegBuild_helpers)

#@@
ADM_FF_SET_DEFAULT()

IF(USE_NVENC)
   SET(FFMPEG_ENCODERS ${FFMPEG_ENCODERS} nvenc)
   xadd("--enable-nonfree")
   xadd("--enable-nvenc")
   set(FFMPEG_ENCODERS  ${FFMPEG_ENCODERS} nvenc_h264 nvenc_hevc)
ENDIF(USE_NVENC)



#@@
ADM_FF_PATCH_IF_NEEDED()

IF("${CROSS_C_COMPILER}" STREQUAL "clang")
	patch_file("${FFMPEG_SOURCE_DIR}" "${FFMPEG_PATCH_DIR}/clang_win32_workaround.diff")
ENDIF("${CROSS_C_COMPILER}" STREQUAL "clang")



#@@
ADM_FF_ADD_OPTIONS()

if (ADM_CPU_X86_32)
        IF("${CROSS_C_COMPILER}" STREQUAL "clang")
                # With clang we use the -mstackrealign -mstack-alignment=16
        ELSE("${CROSS_C_COMPILER}" STREQUAL "clang")
            # Old win32, for hack, not sure it really works with recent gcc
        		 # Not anymore xadd(--enable-memalign-hack)
        ENDIF("${CROSS_C_COMPILER}" STREQUAL "clang")
endif (ADM_CPU_X86_32)

xadd(--enable-w32threads)

#  Cross compiler override (win32 & win64)
xadd(--prefix ${CROSS})
xadd(--host-cc gcc)
xadd(--nm ${CMAKE_CROSS_PREFIX}-nm)
xadd(--extra-cflags  -I${CROSS}/include)
if (CMAKE_C_FLAGS)
	xadd(--extra-cflags ${CMAKE_C_FLAGS})
                    xadd(--extra-ldflags ${CMAKE_C_FLAGS})
endif (CMAKE_C_FLAGS)

set(CROSS_OS mingw32)

if (ADM_CPU_64BIT)
		set(CROSS_ARCH x86_64)
else (ADM_CPU_64BIT)
		set(CROSS_ARCH i386)
endif (ADM_CPU_64BIT)

message(STATUS "Using cross compilation flag: ${FFMPEG_FLAGS}")

ADM_FF_ADD_OPTIONS()


	xadd(--cc "${CMAKE_C_COMPILER}")
	xadd(--ld "${CMAKE_C_COMPILER}")
	xadd(--ar "${CMAKE_AR}")

ADM_FF_SET_EXTRA_FLAGS()
xadd(--enable-cross-compile)

set(CROSS_ARCH "${CROSS_ARCH}" CACHE STRING "")
xadd(--arch ${CROSS_ARCH})

set(CROSS_OS "${CROSS_OS}" CACHE STRING "")
xadd(--target-os ${CROSS_OS})

ADM_FF_SET_EXTRA_FLAGS()

IF(USE_DXVA2)
      xadd(--enable-dxva2)
      set(FFMPEG_DECODERS ${FFMPEG_DECODERS} h264_dxva2 hevc_dxva2)
      # We assume 32 bits mean windows XP; disable d3d11
      IF(NOT ADM_CPU_X86_64)
            xadd(--disable-d3d11va)
      ENDIF(NOT ADM_CPU_X86_64)
ENDIF(USE_DXVA2)

#@@
ADM_FF_BUILD_UNIX_STYLE()
ADM_FF_ADD_DUMMY_TARGET()
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
        INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/dxva2.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
        INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/dxva2_internal.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
ENDIF(USE_DXVA2)

#
#
#
