include(mx_plugin)
SET(ADM_flv_SRCS 
	muxerFlv.cpp
	muxerFlvPlugin.cpp
        muxerFlvConfig.cpp
)

ADD_LIBRARY(ADM_mx_flv SHARED ${ADM_flv_SRCS})
TARGET_LINK_LIBRARIES(ADM_mx_flv  ADM_libavutil6 ADM_libavformat6)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")

INIT_MUXER(ADM_mx_flv)
INSTALL_MUXER(ADM_mx_flv)

