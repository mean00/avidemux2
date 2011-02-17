	INCLUDE(ad_plugin)
	
	SET(ADM_ad_ms_adpcm_SRCS ADM_ad_ms_adpcm.cpp)

	ADD_AUDIO_DECODER( ADM_ad_ms_adpcm ${ADM_ad_ms_adpcm_SRCS})

	INIT_AUDIO_PLUGIN(ADM_ad_ms_adpcm)
	INSTALL_AUDIODECODER(ADM_ad_ms_adpcm)
