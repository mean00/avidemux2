SET(ADM_audiocodec_SRCS
	ADM_8bits.cpp  ADM_codecimaadpcm.cpp  ADM_codecmsadpcm.cpp  ADM_codeculaw.cpp  ADM_codecwma.cpp  ADM_lpcm.cpp
	ADM_audiocodec.cpp  ADM_codecwav.cpp  ADM_pluginLoad.cpp)

ADD_ADM_LIB_ALL_TARGETS(ADM_audiocodec ${ADM_audiocodec_SRCS})