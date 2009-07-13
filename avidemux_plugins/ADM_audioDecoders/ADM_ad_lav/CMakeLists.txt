
	INCLUDE(ad_plugin)
	
	SET(ADM_ad_lav_SRCS ADM_ad_lav.cpp)

	ADD_LIBRARY(ADM_ad_lav SHARED ${ADM_ad_lav_SRCS})
	TARGET_LINK_LIBRARIES(ADM_ad_lav ADM_core ADM_libavcodec ADM_libavutil)

	INIT_AUDIO_PLUGIN(ADM_ad_lav)
	INSTALL_AUDIODECODER(ADM_ad_lav)
