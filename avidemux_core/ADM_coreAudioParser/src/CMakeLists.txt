
SET(ADMaudioParser_SRCS
ADM_aacinfo.cpp 
ADM_mp3info.cpp  
ADM_a52info.cpp 
ADM_dcainfo.cpp  
ADM_eac3info.cpp
)	
#*************************************************
ADD_DEFINITIONS( "-DADM_LEGACY_PROGGY")
ADD_DEFINITIONS( "-DHAVE_LRINT -DHAVE_LRINTF ")
ADD_DEFINITIONS( "-DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0")
#*************************************************
ADD_LIBRARY(ADM_audioParser6 SHARED ${ADMaudioParser_SRCS})
TARGET_LINK_LIBRARIES(ADM_audioParser6 ADM_core6 ADM_coreUtils6 ADM_coreUI6  ADM_libavcodec6 ADM_libavformat6 ADM_libavutil6)

ADM_INSTALL_LIB(ADM_audioParser6)
