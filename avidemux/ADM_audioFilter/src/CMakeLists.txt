# Filters
SET(ADM_audioFilter_SRCS
audiofilter_bridge.cpp
audiofilter_access.cpp
audiofilter_mixer.cpp
audiofilter_dolby.cpp
audiofilter_film2pal.cpp
audiofilter_limiter.cpp
audiofilter_normalize.cpp
audiofilter_SRC.cpp
audio_encoderPlugin.cpp
ADM_audioResample.cpp
audiofilter.cpp
ADM_libsamplerate/samplerate.c  
ADM_libsamplerate/src_linear.c  
ADM_libsamplerate/src_sinc.c  
ADM_libsamplerate/src_zoh.c
)
# Needed by libsamplerate
ADD_DEFINITIONS(-DADM_LEGACY_PROGGY)
ADD_DEFINITIONS(-DHAVE_LRINT -DHAVE_LRINTF)
ADD_DEFINITIONS(-DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0)
ADD_DEFINITIONS(-I${CMAKE_CURRENT_SOURCE_DIR})

#
# API
SET(ADM_audioFilterAPI_SRCS
audiofilterAPI.cpp
)
ADD_LIBRARY(ADM_audioFilter STATIC ${ADM_audioFilter_SRCS})
# For the API part, needed by coreAudioEncoder
ADD_LIBRARY(ADM_coreAudioFilterAPI SHARED ${ADM_audioFilterAPI_SRCS})
INSTALL(TARGETS ADM_coreAudioFilterAPI RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
# END
