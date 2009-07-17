	INCLUDE(ad_plugin)
	
	SET(ADM_ad_ms_adpcm_SRCS ADM_ad_ms_adpcm.cpp)

	ADD_LIBRARY(ADM_ad_ms_adpcm SHARED ${ADM_ad_ms_adpcm_SRCS})

	INIT_AUDIO_PLUGIN(ADM_ad_ms_adpcm)
	INSTALL_AUDIODECODER(ADM_ad_ms_adpcm)
