INCLUDE(av_plugin)


SET(ADM_av_audioCore_SRCS ADM_deviceAudioCore.cpp)

ADD_AUDIO_DEVICE(ADM_av_audioCore  ${ADM_av_audioCore_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
#TARGET_LINK_LIBRARIES(ADM_av_audioCore winmm ADM_core6 ADM_coreAudio6 ADM_coreAudioDevice6 ${SDL_LIBRARY})
TARGET_LINK_LIBRARIES(ADM_av_audioCore "-framework AudioToolbox -framework CoreAudio -framework AudioUnit -framework CoreServices")
INIT_AUDIO_DEVICE(ADM_av_audioCore)
INSTALL_AUDIO_DEVICE(ADM_av_audioCore)
