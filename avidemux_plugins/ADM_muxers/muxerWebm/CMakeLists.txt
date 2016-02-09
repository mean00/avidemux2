include(mx_plugin)
SET(ADM_Webm_SRCS 
	muxerWebm.cpp
	muxerWebmPlugin.cpp
        muxerWebmConfig.cpp
)

ADD_MUXER(ADM_mx_Webm ${ADM_Webm_SRCS})

TARGET_LINK_LIBRARIES(ADM_mx_Webm  ADM_libavutil ADM_libavcodec ADM_libavformat)
INIT_MUXER(ADM_mx_Webm)
INSTALL_MUXER(ADM_mx_Webm)

