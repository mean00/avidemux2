	INCLUDE(ae_plugin)
	include_directories(.)
#====
MACRO(DEFINE_LAV _ext)
	SET(ADM_ae_lav_${_ext}_SRCS audioencoder_lavcodec.cpp)

	ADD_AUDIO_ENCODER(ADM_ae_lav_${_ext} ${ADM_ae_lav_${_ext}_SRCS})
	ADD_TARGET_CFLAGS(ADM_ae_lav_${_ext} "-include ADM_lav_${_ext}.h")
	TARGET_LINK_LIBRARIES(ADM_ae_lav_${_ext}  ADM_libavcodec ADM_libavutil)

	INIT_AUDIO_ENCODER(ADM_ae_lav_${_ext})
	INSTALL_AUDIOENCODER(ADM_ae_lav_${_ext})

ENDMACRO(DEFINE_LAV _ext)
#====

	DEFINE_LAV(mp2)
	DEFINE_LAV(ac3)
	DEFINE_LAV(aac)
