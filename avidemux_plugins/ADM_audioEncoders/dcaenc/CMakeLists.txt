	INCLUDE(ae_plugin)

	SET(ADM_ae_dcaenc_SRCS audioencoder_dcaenc.cpp )

	ADD_AUDIO_ENCODER(ADM_ae_dcaenc ${ADM_ae_dcaenc_SRCS})

	TARGET_LINK_LIBRARIES(ADM_ae_dcaenc ${DCAENC_LIBRARY_DIR})

	ADD_TARGET_CFLAGS(ADM_ae_dcaenc "-I${DCAENC_INCLUDE_DIR}")

	INIT_AUDIO_ENCODER(ADM_ae_dcaenc)
	INSTALL_AUDIOENCODER(ADM_ae_dcaenc)
