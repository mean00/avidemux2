include(mx_plugin)
SET(ADM_flv_SRCS 
	muxerFlv.cpp
	muxerFlvPlugin.cpp
        muxerFlvConfig.cpp
)

ADD_MUXER(ADM_mx_flv ${ADM_flv_SRCS})
TARGET_LINK_LIBRARIES(ADM_mx_flv  ADM_libavutil ADM_libavformat ADM_libavcodec)

INIT_MUXER(ADM_mx_flv)
INSTALL_MUXER(ADM_mx_flv)

