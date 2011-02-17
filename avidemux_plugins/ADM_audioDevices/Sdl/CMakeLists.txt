INCLUDE(av_plugin)


SET(ADM_av_sdl_SRCS ADM_deviceSDL.cpp)

ADD_AUDIO_DEVICE(ADM_av_sdl  ${ADM_av_sdl_SRCS})
TARGET_LINK_LIBRARIES(ADM_av_sdl ADM_core6 ADM_coreAudio6 ADM_coreAudioDevice6 ${SDL_LIBRARY})
SDLify(ADM_deviceSDL.cpp)

INIT_AUDIO_DEVICE(ADM_av_sdl)
INSTALL_AUDIO_DEVICE(ADM_av_sdl)
