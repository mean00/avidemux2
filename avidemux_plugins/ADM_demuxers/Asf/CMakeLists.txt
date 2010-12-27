include(dm_plugin)
SET(ADM_asf_SRCS 
	ADM_asfAudio.cpp
        ADM_asfChunk.cpp  
        ADM_asf.cpp  
        ADM_asfPacket.cpp
        ADM_asfPlugin.cpp
        ADM_asfHeaders.cpp
)

ADD_LIBRARY(ADM_dm_asf SHARED ${ADM_asf_SRCS})

INIT_DEMUXER(ADM_dm_asf)
INSTALL_DEMUXER(ADM_dm_asf)
