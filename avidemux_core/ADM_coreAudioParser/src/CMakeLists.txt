SET(ADM_LIB ADM_audioParser6)
SET(ADMaudioParser_SRCS
  ADM_aacinfo.cpp
  ADM_mp3info.cpp
  ADM_a52info.cpp
  ADM_dcainfo.cpp
  ADM_eac3info.cpp
  ADM_aacadts.cpp
  ADM_aacLatm.cpp
)
ADM_ADD_SHARED_LIBRARY(${ADM_LIB} ${ADMaudioParser_SRCS})
#*************************************************
TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE ADM_LEGACY_PROGGY)
TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE HAVE_LRINT HAVE_LRINTF)
TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE CPU_CLIPS_POSITIVE=0 CPU_CLIPS_NEGATIVE=0)
#*************************************************

TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_coreUtils6 ADM_coreUI6  ADM_libavcodec ADM_libavformat ADM_libavutil)
ADD_DEPENDENCIES(${ADM_LIB} libavcodec libavutil libavformat)

ADM_INSTALL_LIB(${ADM_LIB})
