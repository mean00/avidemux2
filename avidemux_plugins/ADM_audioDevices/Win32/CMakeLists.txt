INCLUDE(av_plugin)


SET(ADM_av_win32_SRCS ADM_deviceWin32.cpp)

ADD_LIBRARY(ADM_av_win32 SHARED ${ADM_av_win32_SRCS})
TARGET_LINK_LIBRARIES(ADM_av_win32 winmm ADM_core ADM_coreAudio ADM_coreAudioDevice)
INIT_AUDIO_DEVICE(ADM_av_win32)
INSTALL_AUDIODEVICE(ADM_av_win32)
