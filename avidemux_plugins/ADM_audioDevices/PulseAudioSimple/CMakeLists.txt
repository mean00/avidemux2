INCLUDE(av_plugin)


SET(ADM_av_pulseAudioSimple_SRCS ADM_devicePulseSimple.cpp)

ADD_AUDIO_DEVICE(ADM_av_pulseAudioSimple  ${ADM_av_pulseAudioSimple_SRCS})
ADD_DEFINITIONS(${PULSEAUDIOSIMPLE_DEFINITIONS})
TARGET_LINK_LIBRARIES(ADM_av_pulseAudioSimple ${PULSEAUDIOSIMPLE_LIBRARIES} pulse)
MESSAGE(STATUS "HACK : we need both libpulse and libpulse-simple when using -W-l,-z,defs")
MESSAGE("")
INIT_AUDIO_DEVICE(ADM_av_pulseAudioSimple)
INSTALL_AUDIO_DEVICE(ADM_av_pulseAudioSimple)
