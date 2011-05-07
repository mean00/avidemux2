
SET(ADMaudioParser_SRCS
ADM_aacinfo.cpp 
ADM_mp3info.cpp  
ADM_a52info.cpp 
ADM_dcainfo.cpp  
ADM_eac3info.cpp
ADM_aacadts.cpp
ADM_aacLatm.cpp
)	
#*************************************************
ADD_DEFINITIONS(-DADM_LEGACY_PROGGY)
ADD_DEFINITIONS(-DHAVE_LRINT -DHAVE_LRINTF)
ADD_DEFINITIONS(-DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0)
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_audioParser6 ${ADMaudioParser_SRCS})
TARGET_LINK_LIBRARIES(ADM_audioParser6 ADM_core6 ADM_coreUtils6 ADM_coreUI6  ADM_libavcodec ADM_libavformat ADM_libavutil)

if (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)
	add_dependencies(ADM_audioParser6 libavcodec libavutil libavformat)
endif (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)

ADM_INSTALL_LIB(ADM_audioParser6)
