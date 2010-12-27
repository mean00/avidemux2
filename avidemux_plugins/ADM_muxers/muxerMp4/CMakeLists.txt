include(mx_plugin)
SET(ADM_mp4_SRCS 
	muxerMP4.cpp
	muxerMP4Plugin.cpp
        muxerMP4Config.cpp
)

ADD_LIBRARY(ADM_mx_mp4 SHARED ${ADM_mp4_SRCS})

TARGET_LINK_LIBRARIES(ADM_mx_mp4  ADM_libavutil ADM_libavcodec ADM_libavformat)
INIT_MUXER(ADM_mx_mp4)
INSTALL_MUXER(ADM_mx_mp4)