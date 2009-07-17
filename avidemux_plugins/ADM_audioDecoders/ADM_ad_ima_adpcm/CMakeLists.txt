	INCLUDE(ad_plugin)
	
	SET(ADM_ad_ima_adpcm_SRCS ADM_ad_ima_adpcm.cpp)

	ADD_LIBRARY(ADM_ad_ima_adpcm SHARED ${ADM_ad_ima_adpcm_SRCS})

	INIT_AUDIO_PLUGIN(ADM_ad_ima_adpcm)
	INSTALL_AUDIODECODER(ADM_ad_ima_adpcm)
