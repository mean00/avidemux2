INCLUDE(av_plugin)


SET(ADM_av_jack_SRCS ADM_deviceJack.cpp)

ADD_AUDIO_DEVICE(ADM_av_jack  ${ADM_av_jack_SRCS})
TARGET_LINK_LIBRARIES(ADM_av_jack ${JACK_LIBRARY_DIR})
include_directories(${JACK_INCLUDE_DIR})

INIT_AUDIO_DEVICE(ADM_av_jack)
INSTALL_AUDIO_DEVICE(ADM_av_jack)
