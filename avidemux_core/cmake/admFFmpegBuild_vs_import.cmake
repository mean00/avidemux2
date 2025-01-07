# # ########################################################################################"
# VS Style management of ffmpeg libs
# We raw import binaries from somewhere else as they have been compiled with mingw
#
# Add -DIMPORT_FOLDER=foobar on cmake
#
#
# # ########################################################################################"
IF(NOT IMPORT_FOLDER)
  SET(IMPORT_FOLDER "d://import//") #Default value
ENDIF()
MESSAGE(STATUS "Importing ffmpeg binaries from ${IMPORT_FOLDER}")

SET(LIB_IMPORT_FOLDER "${IMPORT_FOLDER}\\lib")
INCLUDE_DIRECTORIES(${IMPORT_FOLDER}/include)
LINK_DIRECTORIES(${IMPORT_FOLDER}/lib)
SET(LIBAVCODEC_LIB ${IMPORT_FOLDER}/lib/avcodec.lib)
SET(LIBAVFORMAT_LIB ${IMPORT_FOLDER}/lib/avformat.lib)
SET(LIBAVUTIL_LIB ${IMPORT_FOLDER}/lib/avutil.lib)
SET(LIBPOSTPROC_LIB ${IMPORT_FOLDER}/lib/postproc.lib)
SET(LIBSWSCALE_LIB ${IMPORT_FOLDER}/lib/swscale.lib)

# Actual lib

ADD_LIBRARY(ADM_libswscale UNKNOWN IMPORTED)
ADD_LIBRARY(ADM_libpostproc UNKNOWN IMPORTED)
ADD_LIBRARY(ADM_libavutil UNKNOWN IMPORTED)
ADD_LIBRARY(ADM_libavcodec UNKNOWN IMPORTED)
ADD_LIBRARY(ADM_libavformat UNKNOWN IMPORTED)

SET_PROPERTY(TARGET ADM_libswscale PROPERTY IMPORTED_LOCATION "${LIBSWSCALE_LIB}")
SET_PROPERTY(TARGET ADM_libpostproc PROPERTY IMPORTED_LOCATION "${LIBPOSTPROC_LIB}")
SET_PROPERTY(TARGET ADM_libavutil PROPERTY IMPORTED_LOCATION "${LIBAVUTIL_LIB}")
SET_PROPERTY(TARGET ADM_libavcodec PROPERTY IMPORTED_LOCATION "${LIBAVCODEC_LIB}")
SET_PROPERTY(TARGET ADM_libavformat PROPERTY IMPORTED_LOCATION "${LIBAVFORMAT_LIB}")

# install .dll.a
ADM_INSTALL_LIB_FILES("${LIBAVCODEC_LIB}")
ADM_INSTALL_LIB_FILES("${LIBAVFORMAT_LIB}")
ADM_INSTALL_LIB_FILES("${LIBAVUTIL_LIB}")
ADM_INSTALL_LIB_FILES("${LIBPOSTPROC_LIB}")
ADM_INSTALL_LIB_FILES("${LIBSWSCALE_LIB}")
#INSTALL .dll => TODO

# install header
INSTALL(FILES "${IMPORT_FOLDER}/include/libavutil/avconfig.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavutil" COMPONENT dev)

INSTALL(FILES "${IMPORT_FOLDER}/include/libavcodec/avcodec.h" "${IMPORT_FOLDER}/include/libavcodec/vdpau.h"
        "${IMPORT_FOLDER}/include/libavcodec/version.h"
        #"${IMPORT_FOLDER}/include/libavcodec/audioconvert.h"

        DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)

INSTALL(FILES "${IMPORT_FOLDER}/include/libavformat/avformat.h" "${IMPORT_FOLDER}/include/libavformat/avio.h"
        "${IMPORT_FOLDER}/include/libavformat/version.h"
        "${IMPORT_FOLDER}/include/libavformat/flv.h"
        DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavformat" COMPONENT dev)

INSTALL(FILES "${IMPORT_FOLDER}/include/libavutil/attributes.h"
        "${IMPORT_FOLDER}/include/libavutil/avutil.h"
        "${IMPORT_FOLDER}/include/libavutil/buffer.h"
        "${IMPORT_FOLDER}/include/libavutil/bswap.h"
        "${IMPORT_FOLDER}/include/libavutil/common.h"
        "${IMPORT_FOLDER}/include/libavutil/cpu.h"
        "${IMPORT_FOLDER}/include/libavutil/frame.h"
        "${IMPORT_FOLDER}/include/libavutil/log.h"
        "${IMPORT_FOLDER}/include/libavutil/mathematics.h"
        "${IMPORT_FOLDER}/include/libavutil/mem.h"
        "${IMPORT_FOLDER}/include/libavutil/pixfmt.h"
        "${IMPORT_FOLDER}/include/libavutil/pixdesc.h"
        "${IMPORT_FOLDER}/include/libavutil/channel_layout.h"
        "${IMPORT_FOLDER}/include/libavutil/error.h"
        "${IMPORT_FOLDER}/include/libavutil/dict.h"
        "${IMPORT_FOLDER}/include/libavutil/version.h"
        "${IMPORT_FOLDER}/include/libavutil/time.h"
        "${IMPORT_FOLDER}/include/libavutil/opt.h"
        "${IMPORT_FOLDER}/include/libavutil/intfloat.h"
        "${IMPORT_FOLDER}/include/libavutil/macros.h"
        "${IMPORT_FOLDER}/include/libavutil/samplefmt.h"
        "${IMPORT_FOLDER}/include/libavutil/rational.h"
        "${IMPORT_FOLDER}/include/libavutil/hwcontext_dxva2.h"
        "${IMPORT_FOLDER}/include/libavutil/hwcontext.h"
        "${IMPORT_FOLDER}/include/libavutil/mastering_display_metadata.h"
        "${IMPORT_FOLDER}/include/libavutil/hdr_dynamic_metadata.h"
        DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavutil" COMPONENT dev)

INSTALL(FILES "${IMPORT_FOLDER}/include/libpostproc/postprocess.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libpostproc" COMPONENT dev)
INSTALL(FILES "${IMPORT_FOLDER}/include/libpostproc/version.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libpostproc" COMPONENT dev)
INSTALL(FILES "${IMPORT_FOLDER}/include/libswscale/swscale.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libswscale" COMPONENT dev)
INSTALL(FILES "${IMPORT_FOLDER}/include/libswscale/version.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libswscale" COMPONENT dev)


IF(USE_DXVA2)
  INSTALL(FILES "${IMPORT_FOLDER}/include/libavcodec/dxva2.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
  INSTALL(FILES "${IMPORT_FOLDER}/include/libavcodec/d3d11va.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
  INSTALL(FILES "${IMPORT_FOLDER}/include/libavcodec/dxva2_internal.h" DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_MAJOR_MINOR}/libavcodec" COMPONENT dev)
ENDIF()

#The above install the .libs
# also install the .dll manually

FILE(GLOB ffmpegDll "${IMPORT_FOLDER}/lib/*-*.dll")
FOREACH(one ${ffmpegDll})
  ADM_INSTALL_LIB_FILES(${one})
ENDFOREACH(one ${ffmpegDll})
# EOF
