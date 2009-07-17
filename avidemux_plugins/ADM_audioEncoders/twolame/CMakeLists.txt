	INCLUDE(ae_plugin)
        subdirs(PREORDER ADM_libtwolame)

	SET(ADM_ae_twolame_SRCS audioencoder_twolame.cpp)

	ADD_LIBRARY(ADM_ae_twolame SHARED ${ADM_ae_twolame_SRCS})

	TARGET_LINK_LIBRARIES(ADM_ae_twolame ADM_libtwolame )
        include_directories(ADM_libtwolame)


	INIT_AUDIO_ENCODER(ADM_ae_twolame)
	INSTALL_AUDIOENCODER(ADM_ae_twolame)
