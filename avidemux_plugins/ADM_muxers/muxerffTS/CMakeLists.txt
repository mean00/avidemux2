include(mx_plugin)
SET(ADM_ffTS_SRCS 
	muxerffTS.cpp
	muxerffTSPlugin.cpp
        muxerffTSConfig.cpp
)

ADD_LIBRARY(ADM_mx_ffTS SHARED ${ADM_ffTS_SRCS})

INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")
TARGET_LINK_LIBRARIES(ADM_mx_ffTS  ADM_libavutil6 ADM_libavcodec6 ADM_libavformat6)
INIT_MUXER(ADM_mx_ffTS)
INSTALL_MUXER(ADM_mx_ffTS)

