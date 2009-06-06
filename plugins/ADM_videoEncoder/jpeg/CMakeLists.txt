INCLUDE(ve_plugin)

SET(jpeg_SRCS 
        jpegPlugin.cpp
        ADM_jpegEncoder.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(ADM_ve_jpeg SHARED ${jpeg_SRCS})

INIT_VIDEO_ENCODER(ADM_ve_jpeg)
INSTALL_VIDEO_ENCODER(ADM_ve_jpeg)
TARGET_LINK_LIBRARIES(ADM_ve_jpeg ADM_libavcodec ADM_libavutil)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")
