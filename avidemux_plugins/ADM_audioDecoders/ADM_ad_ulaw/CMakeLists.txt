	INCLUDE(ad_plugin)
	
	SET(ADM_ad_ulaw_SRCS ADM_ad_ulaw.cpp)

	ADD_LIBRARY(ADM_ad_ulaw SHARED ${ADM_ad_ulaw_SRCS})

	INIT_AUDIO_PLUGIN(ADM_ad_ulaw)
	INSTALL_AUDIODECODER(ADM_ad_ulaw)
