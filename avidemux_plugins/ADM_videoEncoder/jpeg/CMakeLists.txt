INCLUDE(ve_plugin)

SET(jpeg_SRCS
        jpegPlugin.cpp
        ADM_jpegEncoder.cpp
)

ADD_VIDEO_ENCODER(ADM_ve_jpeg  ${jpeg_SRCS})
TARGET_INCLUDE_DIRECTORIES(ADM_ve_jpeg PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

INIT_VIDEO_ENCODER(ADM_ve_jpeg)
INSTALL_VIDEO_ENCODER(ADM_ve_jpeg)
TARGET_LINK_LIBRARIES(ADM_ve_jpeg PRIVATE ADM_libavcodec ADM_libavutil)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
