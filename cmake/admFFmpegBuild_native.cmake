

include(admFFmpegBuild_helpers)

ADM_FF_SET_DEFAULT()

IF(USE_NVENC)
   SET(FFMPEG_ENCODERS ${FFMPEG_ENCODERS} nvenc)
   xadd("--enable-nonfree")
   xadd("--enable-nvenc")
   set(FFMPEG_ENCODERS  ${FFMPEG_ENCODERS} nvenc_h264 nvenc_hevc)
ENDIF(USE_NVENC)


ADM_FF_PATCH_IF_NEEDED()

if (USE_VDPAU)
	xadd(--enable-vdpau)
	set(FFMPEG_DECODERS ${FFMPEG_DECODERS} h264_vdpau  vc1_vdpau  mpeg1_vdpau  mpeg_vdpau  wmv3_vdpau)
endif (USE_VDPAU)

if (USE_LIBVA)
	xadd(--enable-vaapi)
	set(FFMPEG_DECODERS ${FFMPEG_DECODERS} h264_vaapi hevc_vaapi)
endif (USE_LIBVA)


ADM_FF_ADD_OPTIONS()

xadd(--cc "${CMAKE_C_COMPILER}")
xadd(--ld "${CMAKE_C_COMPILER}")
xadd(--ar "${CMAKE_AR}")
	# nm should be ok if we do not cross compile
if(CMAKE_LD_FLAGS)
		xadd(--extra-ldflags ${CMAKE_LD_FLAGS})
endif(CMAKE_LD_FLAGS)
if (CMAKE_C_FLAGS)
	  xadd(--extra-cflags ${CMAKE_C_FLAGS})
endif (CMAKE_C_FLAGS)

if (CMAKE_SHARED_LINKER_FLAGS)
		xadd(--extra-ldflags ${CMAKE_SHARED_LINKER_FLAGS})
endif (CMAKE_SHARED_LINKER_FLAGS)

if (VERBOSE)
		# for ffmpeg to use the same  compiler as others
		MESSAGE(STATUS "Building ffmpeg with CC=${CMAKE_C_COMPILER}")
		MESSAGE(STATUS "Building ffmpeg with LD=${CMAKE_C_COMPILER}")
		MESSAGE(STATUS "Building ffmpeg with AR=${CMAKE_AR}")
		MESSAGE(STATUS "Building ffmpeg with CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
		MESSAGE(STATUS "Building ffmpeg with CFLAGS=${FF_FLAGS}")
		MESSAGE(STATUS "Building ffmpeg with CFLAGS2=${FFMPEG_FLAGS}")
		message("")
	endif (VERBOSE)

if (FF_FLAGS)
	set(FF_FLAGS "${FF_FLAGS}" CACHE STRING "")
	xadd(${FF_FLAGS})
endif (FF_FLAGS)

if (NOT "${LAST_FFMPEG_FLAGS}" STREQUAL "${FFMPEG_FLAGS}")
	set(FFMPEG_PERFORM_BUILD 1)
endif (NOT "${LAST_FFMPEG_FLAGS}" STREQUAL "${FFMPEG_FLAGS}")

if (NOT EXISTS "${FFMPEG_BINARY_DIR}/Makefile")
	set(FFMPEG_PERFORM_BUILD 1)
endif (NOT EXISTS "${FFMPEG_BINARY_DIR}/Makefile")

	find_package(Bourne)
	find_package(GnuMake)

	message(STATUS "Configuring FFmpeg")
	set(LAST_FFMPEG_FLAGS "${FFMPEG_FLAGS}" CACHE STRING "" FORCE)

	file(MAKE_DIRECTORY "${FFMPEG_BINARY_DIR}")
	file(REMOVE "${FFMPEG_BINARY_DIR}/ffmpeg${CMAKE_EXECUTABLE_SUFFIX}")
	file(REMOVE "${FFMPEG_BINARY_DIR}/ffmpeg_g${CMAKE_EXECUTABLE_SUFFIX}")

	set(ffmpeg_bash_directory ${BASH_EXECUTABLE})
	convertPathToUnix(ffmpeg_bash_directory ${BASH_EXECUTABLE})
	get_filename_component(ffmpeg_bash_directory ${ffmpeg_bash_directory} PATH)
	configure_file("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/ffmpeg_configure.sh.cmake" "${FFMPEG_BINARY_DIR}/ffmpeg_configure.sh")

	execute_process(COMMAND ${BASH_EXECUTABLE} ffmpeg_configure.sh WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}"
					OUTPUT_VARIABLE FFMPEG_CONFIGURE_OUTPUT RESULT_VARIABLE FFMPEG_CONFIGURE_RESULT)

    if (NOT (FFMPEG_CONFIGURE_RESULT EQUAL 0))
	    MESSAGE(ERROR "configure returned <${FFMPEG_CONFIGURE_RESULT}>")
	    MESSAGE(ERROR "configure output is <${FFMPEG_CONFIGURE_OUTPUT}>")
		MESSAGE(FATAL_ERROR "An error occured ")
    endif (NOT (FFMPEG_CONFIGURE_RESULT EQUAL 0))

	MESSAGE(STATUS "Configuring done, processing")

	if (ADM_CPU_X86)
		file(READ ${FFMPEG_BINARY_DIR}/config.h FF_CONFIG_H)
		string(REGEX MATCH "#define[ ]+HAVE_YASM[ ]+1" FF_YASM "${FF_CONFIG_H}")

		if (NOT FF_YASM)
			message(FATAL_ERROR "Yasm was not found.")
		endif (NOT FF_YASM)
	endif (ADM_CPU_X86)

	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "libavutil"
					WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/config")

	execute_process(COMMAND ${CMAKE_COMMAND} -E copy "./libavutil/avconfig.h" "${CMAKE_BINARY_DIR}/config/libavutil"
					WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}")

	find_package(Patch)
	MESSAGE(STATUS "Patching config.mak - linux (2)")
	patch_file("${FFMPEG_BINARY_DIR}" "${FFMPEG_PATCH_DIR}/config.mak.diff")
	message("")


# Build FFmpeg
getFfmpegLibNames("${FFMPEG_SOURCE_DIR}")

set(ffmpeg_gnumake_executable ${GNUMAKE_EXECUTABLE})
convertPathToUnix(ffmpeg_gnumake_executable ${BASH_EXECUTABLE})
configure_file("${AVIDEMUX_TOP_SOURCE_DIR}/cmake/ffmpeg_make.sh.cmake" "${FFMPEG_BINARY_DIR}/ffmpeg_make.sh")
registerFFmpeg("${FFMPEG_SOURCE_DIR}" "${FFMPEG_BINARY_DIR}" 0)

        add_custom_target(         libavutil_dummy
                                   COMMAND ${CMAKE_BUILD_TOOL}  -j 4 # We assume make or gnumake when host is unix
                                   WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}")
#
#
#
MACRO(FF_ADD_SUBLIB lib)
        	add_custom_command(
				   OUTPUT       "${lib}"
           DEPENDS 	libavutil_dummy
				   COMMAND ${BASH_EXECUTABLE} -c echo "placeHolder")

ENDMACRO(FF_ADD_SUBLIB lib)

FF_ADD_SUBLIB(     	"${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}"       )
FF_ADD_SUBLIB(     	"${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}"       )
FF_ADD_SUBLIB(      "${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}"   )
FF_ADD_SUBLIB(      "${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}"   )
FF_ADD_SUBLIB(      "${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}"     )


# Add and INSTALL libraries
include_directories("${FFMPEG_SOURCE_DIR}")
include_directories("${FFMPEG_BINARY_DIR}")

ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}")
ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}")

INSTALL(FILES "${FFMPEG_BINARY_DIR}/libavutil/avconfig.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavutil" COMPONENT dev)
IF(USE_LIBVA)
        INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/vaapi.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavcodec" COMPONENT dev)
        INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/vaapi_internal.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavcodec" COMPONENT dev)
ENDIF(USE_LIBVA)

INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/avcodec.h" "${FFMPEG_SOURCE_DIR}/libavcodec/vdpau.h"
		"${FFMPEG_SOURCE_DIR}/libavcodec/version.h"
		"${FFMPEG_SOURCE_DIR}/libavcodec/audioconvert.h"
		DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavcodec" COMPONENT dev)
INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavformat/avformat.h" "${FFMPEG_SOURCE_DIR}/libavformat/avio.h"
		"${FFMPEG_SOURCE_DIR}/libavformat/version.h"
		"${FFMPEG_SOURCE_DIR}/libavformat/flv.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavformat" COMPONENT dev)
INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavutil/attributes.h" "${FFMPEG_SOURCE_DIR}/libavutil/avutil.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/buffer.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/bswap.h" "${FFMPEG_SOURCE_DIR}/libavutil/common.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/cpu.h" "${FFMPEG_SOURCE_DIR}/libavutil/frame.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/log.h" "${FFMPEG_SOURCE_DIR}/libavutil/mathematics.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/mem.h" "${FFMPEG_SOURCE_DIR}/libavutil/pixfmt.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/pixdesc.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/channel_layout.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/error.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/dict.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/version.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/time.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/opt.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/intfloat.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/macros.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/samplefmt.h"
		"${FFMPEG_SOURCE_DIR}/libavutil/rational.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libavutil" COMPONENT dev)

INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libpostproc/postprocess.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libpostproc" COMPONENT dev)
INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libpostproc/version.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libpostproc" COMPONENT dev)
INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libswscale/swscale.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libswscale" COMPONENT dev)
INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libswscale/version.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/libswscale" COMPONENT dev)
# EOF
