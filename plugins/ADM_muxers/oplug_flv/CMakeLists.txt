
SET(ADM_flv_SRCS oplug_flv.cpp)
 
ADD_LIBRARY(ADM_mx_flv SHARED ${ADM_flv_SRCS})

INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")
TARGET_LINK_LIBRARIES(ADM_mx_flv  ADM_libavutil ADM_libavcodec ADM_libavformat)
INIT_MUXER(ADM_mx_flv)
INSTALL_MUXER(ADM_mx_flv)
