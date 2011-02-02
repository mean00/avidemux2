set(FFMPEG_VERSION "8cb3c557a9f3b24bc55325e3f64a2150b983305c")	# http://git.ffmpeg.org/?p=ffmpeg;a=snapshot;h=2be4fa05c5528073bcfc472d1c23f2d77b679a9d;sf=tgz

set(LIBRARY_SOURCE_DIR "${AVIDEMUX_TOP_SOURCE_DIR}/cmake")
set(FFMPEG_SOURCE_ARCHIVE "ffmpeg_r${FFMPEG_VERSION}.tar.gz")

set(FFMPEG_EXTRACT_DIR "${CMAKE_BINARY_DIR}")
set(FFMPEG_BASE_DIR "${FFMPEG_EXTRACT_DIR}/ffmpeg")
set(FFMPEG_SOURCE_DIR "${FFMPEG_BASE_DIR}/source")
set(FFMPEG_BINARY_DIR "${FFMPEG_BASE_DIR}/build")

set(FFMPEG_DECODERS  aac ac3 eac3 adpcm_ima_amv  amv  bmp  cinepak  cyuv  dca  dvbsub  dvvideo  ffv1  ffvhuff  flv  fraps  h263  h264  huffyuv  mjpeg
					 mjpegb  mpeg2video  mpeg4  msmpeg4v2  msmpeg4v3  msvideo1  nellymoser  png  qdm2  rawvideo  snow
					 svq3  theora  tscc 
					 vc1  vp3  vp6  vp6a  vp6f  wmav2  wmv1  wmv2  wmv3)
set(FFMPEG_ENCODERS  ac3  dvvideo  ffv1  ffvhuff  flv  h263  huffyuv  mjpeg  mp2  mpeg1video  mpeg2video  mpeg4  snow aac)
set(FFMPEG_MUXERS  flv  matroska  mpeg1vcd  mpeg2dvd  mpeg2svcd  mpegts  mov  mp4  psp)
set(FFMPEG_PARSERS  ac3  h263  h264  mpeg4video)
set(FFMPEG_PROTOCOLS  file)
set(FFMPEG_FLAGS  --enable-shared --disable-static --disable-everything --disable-avfilter --enable-hwaccels --enable-postproc --enable-gpl 
				  --enable-runtime-cpudetect --disable-network --disable-ffplay --disable-ffprobe)

MACRO (xadd opt)
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} ${opt})
ENDMACRO (xadd opt)

if (NOT CROSS)
	xadd(--prefix=${CMAKE_INSTALL_PREFIX})
endif (NOT CROSS)

# Clean FFmpeg
set_directory_properties(${CMAKE_CURRENT_BINARY_DIR} ADDITIONAL_MAKE_CLEAN_FILES "${FFMPEG_BASE_DIR}")

# Prepare FFmpeg source
include(admFFmpegUtil)

#if ((NOT WIN32) OR CROSS)
	find_package(Tar)
	include(admFFmpegPrepareTar)
#endif ((NOT WIN32) OR CROSS)

if (NOT FFMPEG_PREPARED)
	include(admFFmpegPrepareGit)
endif (NOT FFMPEG_PREPARED)


message("")

if (FFMPEG_PERFORM_PATCH)
	find_package(Patch)
	file(GLOB patchFiles "${AVIDEMUX_TOP_SOURCE_DIR}/cmake/patches/*.patch")

	foreach(patchFile ${patchFiles})
		patch_file("${FFMPEG_SOURCE_DIR}" "${patchFile}")
	endforeach(patchFile)

	if (UNIX)
		patch_file("${FFMPEG_SOURCE_DIR}" "${AVIDEMUX_TOP_SOURCE_DIR}/cmake/patches/common.mak.diff")
	endif (UNIX)

	message("")
endif (FFMPEG_PERFORM_PATCH)

if (USE_VDPAU)
	xadd(--enable-vdpau)
	set(FFMPEG_DECODERS ${FFMPEG_DECODERS} h264_vdpau  vc1_vdpau  mpeg1_vdpau  mpeg2_vdpau  wmv3_vdpau)
endif (USE_VDPAU)

# Configure FFmpeg, if required
foreach (decoder ${FFMPEG_DECODERS})
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-decoder=${decoder})
endforeach (decoder)

foreach (encoder ${FFMPEG_ENCODERS})
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-encoder=${encoder})
endforeach (encoder)

foreach (muxer ${FFMPEG_MUXERS})
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-muxer=${muxer})
endforeach (muxer)

foreach (parser ${FFMPEG_PARSERS})
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-parser=${parser})
endforeach (parser)

foreach (protocol ${FFMPEG_PROTOCOLS})
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-protocol=${protocol})
endforeach (protocol)

if (WIN32)
	if (ADM_CPU_X86_32)
		set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-memalign-hack)
	endif (ADM_CPU_X86_32)

	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-w32threads)
else (WIN32)
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-pthreads)
endif (WIN32)

if (NOT ADM_DEBUG)
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --disable-debug)
endif (NOT ADM_DEBUG)

if (CMAKE_C_FLAGS)
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --extra-cflags=${CMAKE_C_FLAGS})
endif (CMAKE_C_FLAGS)

if (CMAKE_SHARED_LINKER_FLAGS)
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --extra-ldflags=${CMAKE_SHARED_LINKER_FLAGS})
endif (CMAKE_SHARED_LINKER_FLAGS)

#  Cross compiler override (win32 & win64)
if (CROSS)
	xadd(--prefix=/mingw)
	xadd(--host-cc=gcc)
	xadd(--cc=${CMAKE_CROSS_PREFIX}-gcc)
	xadd(--ld=${CMAKE_CROSS_PREFIX}-gcc) # Not an error !
	xadd(--ar=${CMAKE_CROSS_PREFIX}-ar) 
	xadd(--nm=${CMAKE_CROSS_PREFIX}-nm) 
	xadd(--sysroot=/mingw/include)

	set(CROSS_OS mingw32)	

	if (ADM_CPU_64BIT)
		set(CROSS_ARCH x86_64)
	else (ADM_CPU_64BIT)
		set(CROSS_ARCH i386)
	endif (ADM_CPU_64BIT)

	message(STATUS "Using cross compilation flag: ${FFMPEG_FLAGS}")
endif (CROSS)

if (CROSS_ARCH OR CROSS_OS)
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --enable-cross-compile)
endif (CROSS_ARCH OR CROSS_OS)

if (CROSS_ARCH)
	set(CROSS_ARCH "${CROSS_ARCH}" CACHE STRING "")
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --arch=${CROSS_ARCH})
endif (CROSS_ARCH)

if (CROSS_OS)
	set(CROSS_OS "${CROSS_OS}" CACHE STRING "")
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} --target-os=${CROSS_OS})
endif (CROSS_OS)

if (FF_FLAGS)
	set(FF_FLAGS "${FF_FLAGS}" CACHE STRING "")
	set(FFMPEG_FLAGS ${FFMPEG_FLAGS} ${FF_FLAGS})
endif (FF_FLAGS)

if (NOT "${LAST_FFMPEG_FLAGS}" STREQUAL "${FFMPEG_FLAGS}")
	set(FFMPEG_PERFORM_BUILD 1)
endif (NOT "${LAST_FFMPEG_FLAGS}" STREQUAL "${FFMPEG_FLAGS}")

if (NOT EXISTS "${FFMPEG_BINARY_DIR}/Makefile")
	set(FFMPEG_PERFORM_BUILD 1)
endif (NOT EXISTS "${FFMPEG_BINARY_DIR}/Makefile")
	

if (FFMPEG_PERFORM_BUILD)
	message(STATUS "Configuring FFmpeg")
	set(LAST_FFMPEG_FLAGS "${FFMPEG_FLAGS}" CACHE STRING "" FORCE)

	file(MAKE_DIRECTORY "${FFMPEG_BINARY_DIR}")
	file(REMOVE "${FFMPEG_BINARY_DIR}/ffmpeg${CMAKE_EXECUTABLE_SUFFIX}")
	file(REMOVE "${FFMPEG_BINARY_DIR}/ffmpeg_g${CMAKE_EXECUTABLE_SUFFIX}")

	execute_process(COMMAND sh ${FFMPEG_SOURCE_DIR}/configure ${FFMPEG_FLAGS}
					WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}"
	                                OUTPUT_VARIABLE FFMPEG_CONFIGURE_OUTPUT
	                                RESULT_VARIABLE FFMPEG_CONFIGURE_RESULT)
        IF(NOT (FFMPEG_CONFIGURE_RESULT EQUAL 0))
	        MESSAGE(ERROR "configure returned <${FFMPEG_CONFIGURE_RESULT}>")
	        MESSAGE(ERROR "configure output is <${FFMPEG_CONFIGURE_OUTPUT}>")
	        MESSAGE(ERROR "When configuing ffmpeg using < sh ${FFMPEG_SOURCE_DIR}/configure ${FFMPEG_FLAGS}>")
                MESSAGE(FATAL_ERROR "An error occured ")
        ENDIF(NOT (FFMPEG_CONFIGURE_RESULT EQUAL 0))
	MESSAGE(STATUS "Configuring done, processing")

	if (ADM_CPU_X86)
		file(READ ${FFMPEG_BINARY_DIR}/config.h FF_CONFIG_H)
		string(REGEX MATCH "#define[ ]+HAVE_YASM[ ]+1" FF_YASM "${FF_CONFIG_H}")

		if (NOT FF_YASM)
			message(FATAL_ERROR "Yasm was not found.")
		endif (NOT FF_YASM)

		if (WIN32 AND (NOT CROSS))
			string(REGEX MATCH "#define[ ]+CONFIG_DXVA2[ ]+1" FF_DXVA2 "${FF_CONFIG_H}")
			
			if (NOT FF_DXVA2)
				message(FATAL_ERROR "DXVA2 not detected.  Ensure the dxva2api.h system header exists (available from Microsoft or http://downloads.videolan.org/pub/videolan/testing/contrib/dxva2api.h).")
			endif (NOT FF_DXVA2)
		endif (WIN32 AND (NOT CROSS))
	endif (ADM_CPU_X86)

	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "libavutil"
					WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/config")

	execute_process(COMMAND ${CMAKE_COMMAND} -E copy "./libavutil/avconfig.h" "${CMAKE_BINARY_DIR}/config/libavutil"
					WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}")

	if (APPLE)
		find_package(Patch)
		patch_file("${FFMPEG_BINARY_DIR}" "${AVIDEMUX_TOP_SOURCE_DIR}/cmake/patches/config_macosx.mak.diff")
	elseif (UNIX)
		find_package(Patch)
		patch_file("${FFMPEG_BINARY_DIR}" "${AVIDEMUX_TOP_SOURCE_DIR}/cmake/patches/config.mak.diff")
	endif (APPLE)

	message("")
endif (FFMPEG_PERFORM_BUILD)

# Build FFmpeg
getFfmpegLibNames("${FFMPEG_SOURCE_DIR}")

add_custom_command(OUTPUT
						"${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}"
						"${FFMPEG_BINARY_DIR}/libavcore/${LIBAVCORE_LIB}"
						"${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}"
						"${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}"
						"${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}"
						"${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}"
						"${FFMPEG_BINARY_DIR}/ffmpeg${CMAKE_EXECUTABLE_SUFFIX}"
				   COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TOOL=${CMAKE_BUILD_TOOL} -P "${AVIDEMUX_TOP_SOURCE_DIR}/cmake/admFFmpegMake.cmake"
				   WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}")

# Add and install libraries
registerFFmpeg("${FFMPEG_SOURCE_DIR}" "${FFMPEG_BINARY_DIR}" 0)
include_directories("${FFMPEG_SOURCE_DIR}")

ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavcore/${LIBAVCORE_LIB}")

install(FILES "${FFMPEG_BINARY_DIR}/libavutil/avconfig.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavutil") 

install(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/avcodec.h" "${FFMPEG_SOURCE_DIR}/libavcodec/vdpau.h"
	DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavcodec")
install(FILES "${FFMPEG_SOURCE_DIR}/libavcore/audioconvert.h" "${FFMPEG_SOURCE_DIR}/libavcore/avcore.h" 
	"${FFMPEG_SOURCE_DIR}/libavcore/samplefmt.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavcore")
install(FILES "${FFMPEG_SOURCE_DIR}/libavformat/avformat.h" "${FFMPEG_SOURCE_DIR}/libavformat/avio.h" 
	"${FFMPEG_SOURCE_DIR}/libavformat/flv.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavformat")
install(FILES "${FFMPEG_SOURCE_DIR}/libavutil/attributes.h" "${FFMPEG_SOURCE_DIR}/libavutil/avutil.h" 
	"${FFMPEG_SOURCE_DIR}/libavutil/bswap.h" "${FFMPEG_SOURCE_DIR}/libavutil/common.h"
	"${FFMPEG_SOURCE_DIR}/libavutil/cpu.h" "${FFMPEG_SOURCE_DIR}/libavutil/intfloat_readwrite.h"
	"${FFMPEG_SOURCE_DIR}/libavutil/log.h" "${FFMPEG_SOURCE_DIR}/libavutil/mathematics.h"
	"${FFMPEG_SOURCE_DIR}/libavutil/mem.h" "${FFMPEG_SOURCE_DIR}/libavutil/pixfmt.h"
	"${FFMPEG_SOURCE_DIR}/libavutil/rational.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavutil")
install(FILES "${FFMPEG_SOURCE_DIR}/libpostproc/postprocess.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libpostproc")
install(FILES "${FFMPEG_SOURCE_DIR}/libswscale/swscale.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libswscale")
