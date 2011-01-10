	INCLUDE(ae_plugin)
	include_directories(.)
#====
	SET(ADM_ae_lav_mp2_SRCS audioencoder_lavcodec.cpp)

	ADD_LIBRARY(ADM_ae_lav_mp2 SHARED ${ADM_ae_lav_mp2_SRCS})
	ADD_TARGET_CFLAGS(ADM_ae_lav_mp2 "-DADM_AE_SET='\"ADM_lav_mp2.h\"'")
	TARGET_LINK_LIBRARIES(ADM_ae_lav_mp2  ADM_libavcodec ADM_libavutil)

	INIT_AUDIO_ENCODER(ADM_ae_lav_mp2)
	INSTALL_AUDIOENCODER(ADM_ae_lav_mp2)
#====
	SET(ADM_ae_lav_ac3_SRCS audioencoder_lavcodec.cpp)

	ADD_LIBRARY(ADM_ae_lav_ac3 SHARED ${ADM_ae_lav_ac3_SRCS})
	ADD_TARGET_CFLAGS(ADM_ae_lav_ac3 "-DADM_AE_SET='\"ADM_lav_ac3.h\"'")
	TARGET_LINK_LIBRARIES(ADM_ae_lav_ac3  ADM_libavcodec ADM_libavutil)

	INIT_AUDIO_ENCODER(ADM_ae_lav_ac3)
	INSTALL_AUDIOENCODER(ADM_ae_lav_ac3)
#====
	SET(ADM_ae_lav_aac_SRCS audioencoder_lavcodec.cpp)

	ADD_LIBRARY(ADM_ae_lav_aac SHARED ${ADM_ae_lav_aac_SRCS})
	ADD_TARGET_CFLAGS(ADM_ae_lav_aac "-DADM_AE_SET='\"ADM_lav_aac.h\"'")
	TARGET_LINK_LIBRARIES(ADM_ae_lav_aac  ADM_libavcodec ADM_libavutil)

	INIT_AUDIO_ENCODER(ADM_ae_lav_aac)
	INSTALL_AUDIOENCODER(ADM_ae_lav_aac)
#====
