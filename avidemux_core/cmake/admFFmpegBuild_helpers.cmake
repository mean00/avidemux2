MACRO(xadd opt)
  IF("${ARGV1}" STREQUAL "")
    SET(FFMPEG_FLAGS "${FFMPEG_FLAGS} ${opt}")
  ELSE()
    STRING(STRIP ${ARGV1} arg)
    SET(FFMPEG_FLAGS "${FFMPEG_FLAGS} ${opt}=\"${arg}\"")
  ENDIF()
ENDMACRO()
include(admFFmpegVersion)
option(FF_INHERIT_BUILD_ENV "" ON)
FIND_PACKAGE(Patch)

SET(FFMPEG_ROOT_DIR "${AVIDEMUX_CORE_SOURCE_DIR}/ffmpeg_package")
SET(FFMPEG_PATCH_DIR  "${FFMPEG_ROOT_DIR}/patches/")
SET(FFMPEG_SOURCE_ARCHIVE "ffmpeg-${FFMPEG_VERSION}.tar.bz2")
SET(FFMPEG_SOURCE_ARCHIVE_DIR "ffmpeg-${FFMPEG_VERSION}")
SET(FFMPEG_EXTRACT_DIR "${CMAKE_BINARY_DIR}")
SET(FFMPEG_BASE_DIR "${FFMPEG_EXTRACT_DIR}/ffmpeg")
SET(FFMPEG_SOURCE_DIR "${FFMPEG_BASE_DIR}/source")
SET(FFMPEG_BINARY_DIR "${FFMPEG_BASE_DIR}/build")

SET(FFMPEG_DECODERS     aac  ac3  av1  adpcm_ima_amv  amv  cinepak  cscd  cyuv
                        dca  dnxhd  dvvideo  eac3  ffv1  ffvhuff  fic
                        flac  flv  fraps  h263  h264  hevc  huffyuv  lagarith
                        mjpeg  mjpegb  mp2  mp3  mpeg2video  mpeg4  msmpeg4v2  msmpeg4v3
                        msvideo1  nellymoser  png  prores  qdm2  rawvideo  snow  svq3  theora
                        tscc  truehd  utvideo  v210 vc1  vorbis  vp3  vp6  vp6a  vp6f  vp8
                        vp9  wmapro  wmalossless  wmav2  wmv1  wmv2  wmv3)

SET(FFMPEG_ENCODERS     aac  ac3  dvvideo  ffv1  ffvhuff  flac  flv  h263  huffyuv
                        mjpeg  mp2  mpeg1video  mpeg2video  mpeg4  png  snow  utvideo)

SET(FFMPEG_MUXERS       flv  matroska  mpeg1vcd  mpeg2dvd  mpeg2svcd  mpegts  mov  mp4  psp  webm)

SET(FFMPEG_PARSERS      ac3  h263  h264  hevc  mpeg4video)

SET(FFMPEG_PROTOCOLS    file)

SET(FFMPEG_BSFS         h264_mp4toannexb hevc_mp4toannexb aac_adtstoasc extract_extradata)

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

  IF(NOT FFMPEG_PREPARED)
    include(admFFmpegPrepareGit)
  ENDIF()

ENDMACRO()


#
#
#
MACRO(ADM_FF_PATCH_IF_NEEDED)
  IF(FFMPEG_PERFORM_PATCH)
    # my patches
    file(GLOB patchFiles "${FFMPEG_PATCH_DIR}/*.patch")

    FOREACH(patchFile ${patchFiles})
      get_filename_component(short ${patchFile}  NAME)
      MESSAGE(STATUS "-- Mine, Applying patch <${short}> --")
      patch_file("${FFMPEG_SOURCE_DIR}" "${patchFile}")
    ENDFOREACH(patchFile)

    MESSAGE("")

    # upstream patches
    file(GLOB patchFiles "${FFMPEG_PATCH_DIR}/upstream/*.patch")
    FOREACH(patchFile ${patchFiles})
      get_filename_component(short ${patchFile}  NAME)
      MESSAGE(STATUS "-- Upstream, applying patch <${short}> --")
      patch_file_p1("${FFMPEG_SOURCE_DIR}" "${patchFile}")
    ENDFOREACH(patchFile)
    MESSAGE("")
  ENDIF()
ENDMACRO()
#
# There is some weird escaping happing when using mingw+msvc
# i.e. /D is escaped into d:\mingw\bin or something similiar
#
MACRO(ADM_FF_SPLIT flags type)
  SET(foo "")
  IF(MSVC)
    MESSAGE(STATUS "Separating args <${flags}>")
    SET(sep_flags ${flags})
    separate_arguments(sep_flags)
    MESSAGE(STATUS "Separating args <${sep_flags}>")
    FOREACH(i ${sep_flags})
      MESSAGE(STATUS "   ${i}")
      string(REGEX REPLACE "^/" "//" flags2 ${i})
      string(REGEX REPLACE " /" "//" flags3 ${flags2})
      SET(foo "${foo} ${flags3}")
    ENDFOREACH(i ${sep_flags})
  ELSE()
    SET(foo "${flags}")
  ENDIF()
  xadd(--extra-${type} "${foo}")
ENDMACRO()

MACRO(ADM_FF_SET_LD_FLAGS flags)
  ADM_FF_SPLIT(${flags} "ldflags")
ENDMACRO()
MACRO(ADM_FF_SET_C_FLAGS flags)
  ADM_FF_SPLIT(${flags} "cflags")
ENDMACRO()

#
#
#
MACRO(ADM_FF_ADD_OPTIONS)
  # Configure FFmpeg, if required
  FOREACH(decoder ${FFMPEG_DECODERS})
    xadd(--enable-decoder ${decoder})
  ENDFOREACH(decoder)

  FOREACH(encoder ${FFMPEG_ENCODERS})
    xadd(--enable-encoder ${encoder})
  ENDFOREACH(encoder)

  FOREACH(muxer ${FFMPEG_MUXERS})
    xadd(--enable-muxer ${muxer})
  ENDFOREACH(muxer)

  FOREACH(parser ${FFMPEG_PARSERS})
    xadd(--enable-parser ${parser})
  ENDFOREACH(parser)

  FOREACH(protocol ${FFMPEG_PROTOCOLS})
    xadd(--enable-protocol ${protocol})
  ENDFOREACH(protocol)

  FOREACH(bistream ${FFMPEG_BSFS})
    xadd(--enable-bsf ${bistream})
  ENDFOREACH(bistream)


  IF(NOT ADM_DEBUG)
    xadd(--disable-debug)
  ENDIF()
ENDMACRO()
#
#
#

MACRO(ADM_FF_INSTALL_LIBS_AND_HEADERS)

  # Add and INSTALL libraries
  include_directories("${FFMPEG_SOURCE_DIR}")
  include_directories("${FFMPEG_BINARY_DIR}")

  ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libswscale/${LIBSWSCALE_LIB}")
  ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libpostproc/${LIBPOSTPROC_LIB}")
  ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavutil/${LIBAVUTIL_LIB}")
  ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavcodec/${LIBAVCODEC_LIB}")
  ADM_INSTALL_LIB_FILES("${FFMPEG_BINARY_DIR}/libavformat/${LIBAVFORMAT_LIB}")

  INSTALL(FILES "${FFMPEG_BINARY_DIR}/libavutil/avconfig.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavutil" COMPONENT dev)

  INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavcodec/avcodec.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/bsf.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/codec.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/codec_desc.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/codec_id.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/codec_par.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/defs.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/packet.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/vdpau.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/videotoolbox.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/version.h"
            "${FFMPEG_SOURCE_DIR}/libavcodec/version_major.h"
            DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
  INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libavformat/avformat.h" "${FFMPEG_SOURCE_DIR}/libavformat/avio.h"
            "${FFMPEG_SOURCE_DIR}/libavformat/version.h"
            "${FFMPEG_SOURCE_DIR}/libavformat/version_major.h"
            "${FFMPEG_SOURCE_DIR}/libavformat/flv.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavformat" COMPONENT dev)
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
            "${FFMPEG_SOURCE_DIR}/libavutil/display.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/version.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/time.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/opt.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/intfloat.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/macros.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/samplefmt.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/hwcontext.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/hwcontext_vaapi.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/hwcontext_vdpau.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/hwcontext_dxva2.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/mastering_display_metadata.h"
            "${FFMPEG_SOURCE_DIR}/libavutil/hdr_dynamic_metadata.h"

            "${FFMPEG_SOURCE_DIR}/libavutil/rational.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavutil" COMPONENT dev)

  INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libpostproc/postprocess.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libpostproc" COMPONENT dev)
  INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libpostproc/version.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libpostproc" COMPONENT dev)
  INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libpostproc/version_major.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libpostproc" COMPONENT dev)
  INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libswscale/swscale.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libswscale" COMPONENT dev)
  INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libswscale/version.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libswscale" COMPONENT dev)
  INSTALL(FILES "${FFMPEG_SOURCE_DIR}/libswscale/version_major.h" DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libswscale" COMPONENT dev)
ENDMACRO()
#
#
#
MACRO(ADM_FF_SET_EXTRA_FLAGS)
  # nm should be ok if we do not cross compile
  if(CMAKE_LD_FLAGS)
    ADM_FF_SET_LD_FLAGS("${CMAKE_LD_FLAGS}")
  ENDIF()
  IF(CMAKE_C_FLAGS)
    ADM_FF_SET_C_FLAGS("${CMAKE_C_FLAGS}")
  ENDIF()

  IF(CMAKE_SHARED_LINKER_FLAGS)

    ADM_FF_SET_LD_FLAGS(${CMAKE_SHARED_LINKER_FLAGS})
  ENDIF()

  IF(VERBOSE)
    # for ffmpeg to use the same  compiler as others
    MESSAGE(STATUS "Building ffmpeg with CC=${CMAKE_C_COMPILER}")
    MESSAGE(STATUS "Building ffmpeg with LD=${CMAKE_C_COMPILER}")
    MESSAGE(STATUS "Building ffmpeg with AR=${CMAKE_AR}")
    MESSAGE(STATUS "Building ffmpeg with CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
    MESSAGE(STATUS "Building ffmpeg with CFLAGS=${FF_FLAGS}")
    MESSAGE(STATUS "Building ffmpeg with CFLAGS2=${FFMPEG_FLAGS}")
    MESSAGE("")
  ENDIF()

  IF(FF_FLAGS)
    SET(FF_FLAGS "${FF_FLAGS}" CACHE STRING "")
    xadd(${FF_FLAGS})
  ENDIF()

  IF(NOT "${LAST_FFMPEG_FLAGS}" STREQUAL "${FFMPEG_FLAGS}")
    SET(FFMPEG_PERFORM_BUILD 1)
  ENDIF()

  IF(NOT EXISTS "${FFMPEG_BINARY_DIR}/Makefile")
    SET(FFMPEG_PERFORM_BUILD 1)
  ENDIF()

ENDMACRO()

#
#
#
MACRO(ADM_FF_BUILD_UNIX_STYLE)

  find_package(Bourne)
  find_package(GnuMake)

  MESSAGE(STATUS "Configuring FFmpeg")
  SET(LAST_FFMPEG_FLAGS "${FFMPEG_FLAGS}" CACHE STRING "" FORCE)

  file(MAKE_DIRECTORY "${FFMPEG_BINARY_DIR}")
  file(REMOVE "${FFMPEG_BINARY_DIR}/ffmpeg${CMAKE_EXECUTABLE_SUFFIX}")
  file(REMOVE "${FFMPEG_BINARY_DIR}/ffmpeg_g${CMAKE_EXECUTABLE_SUFFIX}")

  SET(ffmpeg_bash_directory ${BASH_EXECUTABLE})
  convertPathToUnix(ffmpeg_bash_directory ${BASH_EXECUTABLE})
  get_filename_component(ffmpeg_bash_directory ${ffmpeg_bash_directory} PATH)
  configure_file("${AVIDEMUX_CORE_SOURCE_DIR}/cmake/ffmpeg_configure.sh.cmake" "${FFMPEG_BINARY_DIR}/ffmpeg_configure.sh")

  execute_process(COMMAND ${BASH_EXECUTABLE} ffmpeg_configure.sh WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}"
                    OUTPUT_VARIABLE FFMPEG_CONFIGURE_OUTPUT RESULT_VARIABLE FFMPEG_CONFIGURE_RESULT)

  IF(NOT (FFMPEG_CONFIGURE_RESULT EQUAL 0))
    MESSAGE(SEND_ERROR "configure returned <${FFMPEG_CONFIGURE_RESULT}>")
    MESSAGE(SEND_ERROR "configure output is <${FFMPEG_CONFIGURE_OUTPUT}>")
    MESSAGE(FATAL_ERROR "An error occured ")
  ENDIF()

  MESSAGE(STATUS "Configuring done, processing")

  IF(ADM_CPU_X86)
    file(READ ${FFMPEG_BINARY_DIR}/config.h FF_CONFIG_H)
    string(REGEX MATCH "#define[ ]+HAVE_X86ASM[ ]+1" FF_YASM "${FF_CONFIG_H}")

    IF(NOT FF_YASM)
      MESSAGE(FATAL_ERROR "Yasm was not found.")
    ENDIF()
  ENDIF()

  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "libavutil"
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/config")

  execute_process(COMMAND ${CMAKE_COMMAND} -E copy "./libavutil/avconfig.h" "${CMAKE_BINARY_DIR}/config/libavutil"
                    WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}")




  # Build FFmpeg
  getFfmpegLibNames("${FFMPEG_SOURCE_DIR}")

  SET(ffmpeg_gnumake_executable ${GNUMAKE_EXECUTABLE})
  convertPathToUnix(ffmpeg_gnumake_executable ${BASH_EXECUTABLE})
  configure_file("${AVIDEMUX_CORE_SOURCE_DIR}/cmake/ffmpeg_make.sh.cmake" "${FFMPEG_BINARY_DIR}/ffmpeg_make.sh")
  registerFFmpeg("${FFMPEG_SOURCE_DIR}" "${FFMPEG_BINARY_DIR}" 0)
ENDMACRO()
#
#
#
MACRO(ADM_FF_ADD_DUMMY_TARGET)
  #SET(PARRALLEL "-j1")
  if(NOT MSVC)
    include(ProcessorCount)
    ProcessorCount(NPROC)
    if(NOT NPROC EQUAL 0)
      SET(PARRALLEL "-j${NPROC}")
    ENDIF()
  ENDIF()
  add_custom_target(         libavutil_dummy ALL
                                       COMMAND ${GNUMAKE_EXECUTABLE} ${PARRALLEL}
                                       WORKING_DIRECTORY "${FFMPEG_BINARY_DIR}"
                                       COMMENT "Compiling FFmpeg")
ENDMACRO()
