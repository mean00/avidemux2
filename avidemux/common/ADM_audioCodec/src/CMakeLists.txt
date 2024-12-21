INCLUDE_DIRECTORIES(../include)
SET(ADM_audiocodec_SRCS
ADM_8bits.cpp  
ADM_lpcm.cpp
ADM_audiocodec.cpp  
ADM_codecpcm16.cpp  
ADM_codecpcmfloat.cpp  
ADM_pluginLoad.cpp)

ADD_LIBRARY(ADM_audiocodec6 STATIC ${ADM_audiocodec_SRCS})
