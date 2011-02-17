INCLUDE(av_plugin)


SET(ADM_av_pulseAudioSimple_SRCS ADM_devicePulseSimple.cpp)

ADD_AUDIO_DEVICE(ADM_av_pulseAudioSimple  ${ADM_av_pulseAudioSimple_SRCS})
ADD_DEFINITIONS(${PULSEAUDIOSIMPLE_DEFINITIONS})
TARGET_LINK_LIBRARIES(ADM_av_pulseAudioSimple ${PULSEAUDIOSIMPLE_LIBRARIES})
INIT_AUDIO_DEVICE(ADM_av_pulseAudioSimple)
INSTALL_AUDIO_DEVICE(ADM_av_pulseAudioSimple)
