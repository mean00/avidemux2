INCLUDE(av_plugin)


SET(ADM_av_oss_SRCS ADM_deviceoss.cpp)

ADD_AUDIO_DEVICE(ADM_av_oss  ${ADM_av_oss_SRCS})
include_directories(${OSS_INCLUDE_DIR})
INIT_AUDIO_DEVICE(ADM_av_oss)
INSTALL_AUDIO_DEVICE(ADM_av_oss)
