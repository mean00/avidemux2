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
audiofilter_channels.cpp
audiofilter_eq.cpp
audiocopy.cpp
ADM_audioResample.cpp
audiofilter.cpp
audiofilter_thread.cpp
ADM_libsamplerate/samplerate.cpp  
ADM_libsamplerate/src_linear.cpp 
ADM_libsamplerate/src_sinc.cpp 
ADM_libsamplerate/src_zoh.cpp
ADM_soundtouch/AAFilter.cpp
ADM_soundtouch/FIFOSampleBuffer.cpp
ADM_soundtouch/FIRFilter.cpp
ADM_soundtouch/InterpolateCubic.cpp
ADM_soundtouch/InterpolateLinear.cpp
ADM_soundtouch/InterpolateShannon.cpp
ADM_soundtouch/mmx_optimized.cpp
ADM_soundtouch/PeakFinder.cpp
ADM_soundtouch/RateTransposer.cpp
ADM_soundtouch/SoundTouch.cpp
ADM_soundtouch/sse_optimized.cpp
ADM_soundtouch/TDStretch.cpp
ADM_audioStretch.cpp
audiofilter_stretch.cpp
)
YASMIFY(dolbyAsm audiofilter_dolby_asm)

# Needed by libsamplerate
ADD_DEFINITIONS(-DADM_LEGACY_PROGGY)
ADD_DEFINITIONS(-DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0)
ADD_DEFINITIONS(-I${CMAKE_CURRENT_SOURCE_DIR})

if (APPLE)
	SET(CMAKE_CXX_STANDARD 11)
	SET(CMAKE_CXX_STANDARD_REQUIRED ON)
	SET(CMAKE_CXX_EXTENSIONS OFF)
endif (APPLE)

if (MSVC)
	remove_definitions(-W3)
	add_definitions(-W1 )
else (MSVC)
	add_definitions(-DHAVE_LRINT -DHAVE_LRINTF)
endif (MSVC)

include_directories("${PTHREAD_INCLUDE_DIR}")
ADD_LIBRARY(ADM_audioFilter6 STATIC ${ADM_audioFilter_SRCS} ${dolbyAsm})
# END
