SET(ADM_toolkit_SRCS 
	ADM_audioQueue.cpp  ADM_packetQueue.cpp  automation.cpp  filesel.cpp)
	
ADD_ADM_LIB_ALL_TARGETS(ADM_toolkit ${ADM_toolkit_SRCS})
