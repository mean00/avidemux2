	INCLUDE(ae_plugin)

	SET(ADM_ae_lame_SRCS audioencoder_lame.cpp)

	ADD_LIBRARY(ADM_ae_lame SHARED ${ADM_ae_lame_SRCS})

	TARGET_LINK_LIBRARIES(ADM_ae_lame ${LAME_LIBRARY_DIR} ADM_core ADM_coreAudio ADM_coreUI)

	ADD_TARGET_CFLAGS(ADM_ae_lame "-I${LAME_INCLUDE_DIR}")

	INIT_AUDIO_ENCODER(ADM_ae_lame)
	INSTALL_AUDIOENCODER(ADM_ae_lame)
