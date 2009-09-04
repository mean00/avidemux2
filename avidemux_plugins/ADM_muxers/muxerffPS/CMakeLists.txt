include(mx_plugin)
SET(ADM_ffPS_SRCS 
	muxerffPS.cpp
	muxerffPSPlugin.cpp
        muxerffPSConfig.cpp
)

ADD_LIBRARY(ADM_mx_ffPS SHARED ${ADM_ffPS_SRCS})

INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")
TARGET_LINK_LIBRARIES(ADM_mx_ffPS  ADM_libavutil6 ADM_libavcodec6 ADM_libavformat6)
INIT_MUXER(ADM_mx_ffPS)
INSTALL_MUXER(ADM_mx_ffPS)

