SET(ADM_LIB ADM_audioFilter6)
ADD_LIBRARY(${ADM_LIB} STATIC)
YASMIFY(dolbyAsm audiofilter_dolby_asm)
TARGET_SOURCES(${ADM_LIB} PRIVATE
  ${dolbyAsm}
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

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PUBLIC ADM_LEGACY_PROGGY=1 CPU_CLIPS_POSITIVE=0 CPU_CLIPS_NEGATIVE=0)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_editor6_interface)
#


IF(APPLE)
  SET(CMAKE_CXX_STANDARD 11)
  SET(CMAKE_CXX_STANDARD_REQUIRED ON)
  SET(CMAKE_CXX_EXTENSIONS OFF)
ENDIF()

IF(MSVC)
  remove_definitions(-W3)
  TARGET_COMPILE_OPTIONS( ${ADM_LIB} PRIVATE -W1 )
ELSE()
  TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PUBLIC HAVE_LRINT=1 HAVE_LRINTF=1)
ENDIF()

ADD_LIBRARY(ADM_audioFilter6_interface INTERFACE  )
TARGET_INCLUDE_DIRECTORIES( ADM_audioFilter6_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
# END
