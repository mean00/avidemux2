SET(ADM_videoCodec_SRCS
        ADM_ffmpeg_vdpau.cpp
        ADM_ffmpeg_xvba.cpp
        ADM_ffmpeg_libva.cpp
        ADM_videoCodec.cpp
        ADM_dynVideoDecoder.cpp
)

ADD_LIBRARY(ADM_videocodec6 STATIC ${ADM_videoCodec_SRCS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
