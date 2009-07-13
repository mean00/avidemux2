	INCLUDE(ad_plugin)
	
	SET(ADM_ad_ulaw_SRCS ADM_ad_ulaw.cpp)

	ADD_LIBRARY(ADM_ad_ulaw SHARED ${ADM_ad_ulaw_SRCS})
	TARGET_LINK_LIBRARIES(ADM_ad_ulaw ADM_core )

	INIT_AUDIO_PLUGIN(ADM_ad_ulaw)
	INSTALL_AUDIODECODER(ADM_ad_ulaw)
