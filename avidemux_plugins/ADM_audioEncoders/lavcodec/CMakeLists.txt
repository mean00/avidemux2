	INCLUDE(ae_plugin)
	include_directories(.)
#====
MACRO(DEFINE_LAV _ext)
	SET(ADM_ae_lav_${_ext}_SRCS audioencoder_lavcodec.cpp)

	ADD_AUDIO_ENCODER(ADM_ae_lav_${_ext} ${ADM_ae_lav_${_ext}_SRCS})
        IF(MSVC)
	        ADD_TARGET_CFLAGS(ADM_ae_lav_${_ext} "/FI ${CMAKE_CURRENT_SOURCE_DIR}/ADM_lav_${_ext}.h")
        ELSE(MSVC)
	        ADD_TARGET_CFLAGS(ADM_ae_lav_${_ext} "-include ADM_lav_${_ext}.h")
        ENDIF(MSVC)
	TARGET_LINK_LIBRARIES(ADM_ae_lav_${_ext}  ADM_libavcodec ADM_libavutil)
	if(AE_LAVCODEC_BUILD_QUIRK AND NOT MSVC)
            TARGET_LINK_LIBRARIES(ADM_ae_lav_${_ext} "c")
        endif(AE_LAVCODEC_BUILD_QUIRK AND NOT MSVC)

	INIT_AUDIO_ENCODER(ADM_ae_lav_${_ext})
	INSTALL_AUDIOENCODER(ADM_ae_lav_${_ext})

ENDMACRO(DEFINE_LAV _ext)
#====

	DEFINE_LAV(mp2)
	DEFINE_LAV(ac3)
	DEFINE_LAV(aac)
	# Does not support floatp yet DEFINE_LAV(dts)
