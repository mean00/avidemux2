	INCLUDE(ae_plugin)

	SET(ADM_ae_vorbis_SRCS audioencoder_vorbis.cpp)

	ADD_LIBRARY(ADM_ae_vorbis SHARED ${ADM_ae_vorbis_SRCS})

	TARGET_LINK_LIBRARIES(ADM_ae_vorbis ${VORBIS_LIBRARY_DIR} ${VORBISENC_LIBRARY_DIR} ADM_core ADM_coreAudio ADM_coreUI)

	ADD_TARGET_CFLAGS(ADM_ae_vorbis "-I${VORBIS_INCLUDE_DIR}")

	INIT_AUDIO_ENCODER(ADM_ae_vorbis)
	INSTALL_AUDIOENCODER(ADM_ae_vorbis)
