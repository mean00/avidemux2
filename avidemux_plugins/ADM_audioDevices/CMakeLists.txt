include(admCheckAudioDeviceLibs)
include(admSummary)
if(USE_ALSA)
ADD_SUBDIRECTORY(Alsa)
endif(USE_ALSA)

if(USE_PULSE_SIMPLE)
ADD_SUBDIRECTORY(PulseAudioSimple)
endif(USE_PULSE_SIMPLE)

if(USE_ESD)
ADD_SUBDIRECTORY(Esd)
endif(USE_ESD)

if(USE_SDL)
ADD_SUBDIRECTORY(Sdl)
endif(USE_SDL)

if(USE_OSS)
ADD_SUBDIRECTORY(Oss)
endif(USE_OSS)

#if(USE_ARTS)
#ADD_SUBDIRECTORY(Arts)
#endif(USE_ARTS)

if(USE_JACK)
ADD_SUBDIRECTORY(Jack)
endif(USE_JACK)

if(WIN32)
ADD_SUBDIRECTORY(Win32)
endif(WIN32)

MESSAGE("***  Audio Devices ***")
ADM_DISPLAY("SDL      " "${USE_SDL}")
ADM_DISPLAY("Pulse    " "${USE_PULSE_SIMPLE}")
ADM_DISPLAY("ESD      " "${USE_ESD}")
ADM_DISPLAY("OSS      " "${USE_OSS}")
ADM_DISPLAY("Alsa     " "${USE_ALSA}")
ADM_DISPLAY("Jack     " "${USE_JACK}")
if(WIN32)
ADM_DISPLAY("Win32    " "1")
endif(WIN32)
MESSAGE("***  /Audio Devices  ***")


