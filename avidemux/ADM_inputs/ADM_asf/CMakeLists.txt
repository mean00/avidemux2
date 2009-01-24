SET(ADM_asf_SRCS 
	ADM_asf_audio.cpp  ADM_asfChunk.cpp  ADM_asf.cpp  ADM_asfPacket.cpp)

ADD_ADM_LIB_ALL_TARGETS(ADM_asf ${ADM_asf_SRCS})