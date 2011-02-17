	INCLUDE(ae_plugin)

	SET(ADM_ae_aften_SRCS audioencoder_aften.cpp)

	ADD_AUDIO_ENCODER(ADM_ae_aften ${ADM_ae_aften_SRCS})

	TARGET_LINK_LIBRARIES(ADM_ae_aften ${AFTEN_LIBRARY_DIR})

	ADD_TARGET_CFLAGS(ADM_ae_aften "-I${AFTEN_INCLUDE_DIR}")

        if(USE_AFTEN_08)
                ADD_DEFINITIONS(-DAFTEN_08)
        endif(USE_AFTEN_08)

	INIT_AUDIO_ENCODER(ADM_ae_aften)
	INSTALL_AUDIOENCODER(ADM_ae_aften)
