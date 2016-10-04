	INCLUDE(ae_plugin)

	SET(ADM_ae_fdk_aac_SRCS ae_fdk.cpp)

	ADD_AUDIO_ENCODER(ADM_ae_fdk_aac ${ADM_ae_fdk_aac_SRCS})

        TARGET_LINK_LIBRARIES(ADM_ae_fdk_aac ${fdk_aac_LIBRARY_DIR})

	ADD_TARGET_CFLAGS(ADM_ae_fdk_aac "-I${fdk_aac_INCLUDE_DIR}")

	INIT_AUDIO_ENCODER(ADM_ae_fdk_aac)
	INSTALL_AUDIOENCODER(ADM_ae_fdk_aac)
