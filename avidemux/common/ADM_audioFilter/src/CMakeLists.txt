# Filters
SET(ADM_audioFilter_SRCS

audio_encoderPlugin.cpp

audiofilter_bridge.cpp
audiofilter_access.cpp
audiofilter_mixer.cpp
audiofilter_dolby.cpp
audiofilter_film2pal.cpp
audiofilter_limiter.cpp
audiofilter_normalize.cpp
audiofilter_SRC.cpp
audiofilter_encoder.cpp
audiofilter_interface.cpp
audiocopy.cpp
ADM_audioResample.cpp
audiofilter.cpp
audiofilter_thread.cpp
ADM_libsamplerate/samplerate.cpp  
ADM_libsamplerate/src_linear.cpp 
ADM_libsamplerate/src_sinc.cpp 
ADM_libsamplerate/src_zoh.cpp
)
# Needed by libsamplerate
ADD_DEFINITIONS(-DADM_LEGACY_PROGGY)
ADD_DEFINITIONS(-DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0)
ADD_DEFINITIONS(-I${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
	remove_definitions(-W3)
	add_definitions(-W1 -Dinline=__inline)
else (MSVC)
	add_definitions(-DHAVE_LRINT -DHAVE_LRINTF)
endif (MSVC)

include_directories("${PTHREAD_INCLUDE_DIR}")
include_directories("../../ADM_audiocodec/include")
ADD_LIBRARY(ADM_audioFilter6 STATIC ${ADM_audioFilter_SRCS})
# END
