MACRO (xadd opt)
	if ("${ARGV1}" STREQUAL "")
		set(FFMPEG_FLAGS "${FFMPEG_FLAGS} ${opt}")
	else ("${ARGV1}" STREQUAL "")
		string(STRIP ${ARGV1} arg)
		set(FFMPEG_FLAGS "${FFMPEG_FLAGS} ${opt}=\"${arg}\"")
	endif ("${ARGV1}" STREQUAL "")
ENDMACRO (xadd)

option(FF_INHERIT_BUILD_ENV "" ON)

set(FFMPEG_VERSION "3.0.3")
set(FFMPEG_ROOT_DIR "${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ffmpeg_package")
set(FFMPEG_PATCH_DIR  "${FFMPEG_ROOT_DIR}/patches/")
set(FFMPEG_SOURCE_ARCHIVE "ffmpeg-${FFMPEG_VERSION}.tar.bz2")
set(FFMPEG_SOURCE_ARCHIVE_DIR "ffmpeg-${FFMPEG_VERSION}")
set(FFMPEG_EXTRACT_DIR "${CMAKE_BINARY_DIR}")
set(FFMPEG_BASE_DIR "${FFMPEG_EXTRACT_DIR}/ffmpeg")
set(FFMPEG_SOURCE_DIR "${FFMPEG_BASE_DIR}/source")
set(FFMPEG_BINARY_DIR "${FFMPEG_BASE_DIR}/build")

set(FFMPEG_DECODERS  aac ac3 eac3 adpcm_ima_amv  amv  bmp  cinepak  cyuv  dca  dvbsub  dvvideo  ffv1  ffvhuff  flv  fraps  h263  h264
                                         hevc  huffyuv  mjpeg
					 mjpegb  mpeg2video  mpeg4  msmpeg4v2  msmpeg4v3  msvideo1  nellymoser  png  qdm2  rawvideo  snow
					 svq3  theora  tscc  mp2 mp3 mp2_float mp3_float
					 vc1  vp3  vp6  vp6a  vp6f  vp8 vp9 wmapro wmav2  wmv1  wmv2  wmv3 cscd lagarith flac vorbis)
set(FFMPEG_ENCODERS  ac3  ac3_float dvvideo  ffv1  ffvhuff  flv  h263  huffyuv  mjpeg  mp2  mpeg1video  mpeg2video  mpeg4  snow aac dca flac)
set(FFMPEG_MUXERS  flv  matroska  mpeg1vcd  mpeg2dvd  mpeg2svcd  mpegts  mov  mp4  psp webm)
set(FFMPEG_PARSERS  ac3  h263  h264  hevc  mpeg4video)
set(FFMPEG_PROTOCOLS  file)
set(FFMPEG_BSFS h264_mp4toannexb aac_adtstoasc)

#
#
#
#
MACRO(ADM_FF_SET_DEFAULT)
	xadd("--enable-shared --disable-static --disable-everything --disable-avfilter --enable-hwaccels --enable-postproc --enable-gpl")
	xadd("--enable-runtime-cpudetect --disable-network ")
	xadd("--enable-swscale --disable-swresample")
	xadd("--disable-doc --disable-programs")

	FIND_HEADER_AND_LIB(_X265 x265.h)
	FIND_HEADER_AND_LIB(_X265_CONFIG x265_config.h)

	xadd("--disable-libx265")
	xadd("--disable-libx264")
	xadd(--prefix ${CMAKE_INSTALL_PREFIX})

	# Clean FFmpeg
	set_directory_properties(${CMAKE_CURRENT_BINARY_DIR} ADDITIONAL_MAKE_CLEAN_FILES "${FFMPEG_BASE_DIR}")

	# Prepare FFmpeg source
	include(admFFmpegUtil)
	include(admFFmpegPrepareTar)

ENDMACRO(ADM_FF_SET_DEFAULT)


#
#
#
MACRO(ADM_FF_PATCH_IF_NEEDED)
	if (FFMPEG_PERFORM_PATCH)
		find_package(Patch)
	  # my patches
		file(GLOB patchFiles "${FFMPEG_PATCH_DIR}/*.patch")

		foreach(patchFile ${patchFiles})
	      get_filename_component(short ${patchFile}  NAME)
	      MESSAGE(STATUS "-- Mine, Applying patch <${short}> --")
				patch_file("${FFMPEG_SOURCE_DIR}" "${patchFile}")
		endforeach(patchFile)

	  #
		if (UNIX )
				MESSAGE(STATUS "Patching Linux common.mak")
				patch_file("${FFMPEG_SOURCE_DIR}" "${FFMPEG_PATCH_DIR}/common.mak.diff")
		endif (UNIX )
	  IF(WIN32)
	      IF("${CROSS_C_COMPILER}" STREQUAL "clang")
				patch_file("${FFMPEG_SOURCE_DIR}" "${FFMPEG_PATCH_DIR}/clang_win32_workaround.diff")
	                ENDIF("${CROSS_C_COMPILER}" STREQUAL "clang")
	  ENDIF(WIN32)
		message("")
	endif (FFMPEG_PERFORM_PATCH)
ENDMACRO(ADM_FF_PATCH_IF_NEEDED)


#
#
#
MACRO(ADM_FF_ADD_OPTIONS)
		# Configure FFmpeg, if required
		foreach (decoder ${FFMPEG_DECODERS})
			xadd(--enable-decoder ${decoder})
		endforeach (decoder)

		foreach (encoder ${FFMPEG_ENCODERS})
			xadd(--enable-encoder ${encoder})
		endforeach (encoder)

		foreach (muxer ${FFMPEG_MUXERS})
			xadd(--enable-muxer ${muxer})
		endforeach (muxer)

		foreach (parser ${FFMPEG_PARSERS})
			xadd(--enable-parser ${parser})
		endforeach (parser)

		foreach (protocol ${FFMPEG_PROTOCOLS})
			xadd(--enable-protocol ${protocol})
		endforeach (protocol)

		foreach (bistream ${FFMPEG_BSFS})
			xadd(--enable-bsf ${bistream})
		endforeach (bistream)


		if (NOT ADM_DEBUG)
			xadd(--disable-debug)
		endif (NOT ADM_DEBUG)
ENDMACRO(ADM_FF_ADD_OPTIONS)
#
#
#
