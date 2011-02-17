	INCLUDE(ae_plugin)

	SET(ADM_ae_pcm_SRCS audioencoder_pcm.cpp)

	ADD_AUDIO_ENCODER(ADM_ae_pcm ${ADM_ae_pcm_SRCS})

	INIT_AUDIO_ENCODER(ADM_ae_pcm)
	INSTALL_AUDIOENCODER(ADM_ae_pcm)
