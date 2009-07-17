	INCLUDE(ae_plugin)

	SET(ADM_ae_aften_SRCS audioencoder_aften.cpp)

	ADD_LIBRARY(ADM_ae_aften SHARED ${ADM_ae_aften_SRCS})

	TARGET_LINK_LIBRARIES(ADM_ae_aften ${AFTEN_LIBRARY_DIR})

	ADD_TARGET_CFLAGS(ADM_ae_aften "-I${AFTEN_INCLUDE_DIR}")

	INIT_AUDIO_ENCODER(ADM_ae_aften)
	INSTALL_AUDIOENCODER(ADM_ae_aften)
