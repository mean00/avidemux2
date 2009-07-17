	INCLUDE(ae_plugin)
        INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../avidemux/")
        INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../avidemux/ADM_libraries/ADM_ffmpeg/")

	SET(ADM_ae_lav_mp2_SRCS audioencoder_lavcodec.cpp)

	ADD_LIBRARY(ADM_ae_lav_mp2 SHARED ${ADM_ae_lav_mp2_SRCS})
	ADD_TARGET_CFLAGS(ADM_ae_lav_mp2 "-DADM_LAV_MP2")
	TARGET_LINK_LIBRARIES(ADM_ae_lav_mp2  ADM_libavcodec6)

	INIT_AUDIO_ENCODER(ADM_ae_lav_mp2)
	INSTALL_AUDIOENCODER(ADM_ae_lav_mp2)

	SET(ADM_ae_lav_ac3_SRCS audioencoder_lavcodec.cpp)

	ADD_LIBRARY(ADM_ae_lav_ac3 SHARED ${ADM_ae_lav_ac3_SRCS})
	ADD_TARGET_CFLAGS(ADM_ae_lav_ac3 "-DADM_LAV_AC3")
	TARGET_LINK_LIBRARIES(ADM_ae_lav_ac3  ADM_libavcodec6)

	INIT_AUDIO_ENCODER(ADM_ae_lav_ac3)
	INSTALL_AUDIOENCODER(ADM_ae_lav_ac3)
