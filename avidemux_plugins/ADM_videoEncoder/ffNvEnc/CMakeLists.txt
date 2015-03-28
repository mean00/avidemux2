INCLUDE(ve_plugin)

SET(ffNvenc_SRCS 
        ffNvEncPlugin.cpp
        ffNvEnc.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_VIDEO_ENCODER(ADM_ve_ffNvenc  ${ffNvenc_SRCS})

INIT_VIDEO_ENCODER(ADM_ve_ffNvenc)
INSTALL_VIDEO_ENCODER(ADM_ve_ffNvenc)
TARGET_LINK_LIBRARIES(ADM_ve_ffNvenc ADM_libavcodec ADM_libavutil)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
