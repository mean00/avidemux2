INCLUDE(ae_plugin)
include_directories(.)
#====
MACRO(DEFINE_LAV _ext)
  SET(ADM_ae_lav_${_ext}_SRCS ADM_lav_${_ext}.cpp)

  ADD_AUDIO_ENCODER(ADM_ae_lav_${_ext} ${ADM_ae_lav_${_ext}_SRCS})
  TARGET_LINK_LIBRARIES(ADM_ae_lav_${_ext}  PRIVATE ADM_libavcodec ADM_libavutil)
  IF(AE_LAVCODEC_BUILD_QUIRK AND NOT MSVC)
    TARGET_LINK_LIBRARIES(ADM_ae_lav_${_ext} PRIVATE "c")
  ENDIF()

  INIT_AUDIO_ENCODER(ADM_ae_lav_${_ext})
  INSTALL_AUDIOENCODER(ADM_ae_lav_${_ext})

ENDMACRO()
#====

DEFINE_LAV(mp2)
DEFINE_LAV(ac3)
DEFINE_LAV(aac)
# Does not support floatp yet DEFINE_LAV(dts)
#DEFINE_LAV(dts)
