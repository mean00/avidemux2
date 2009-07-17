SET(ADM_toolkit_SRCS 
	ADM_audioQueue.cpp  ADM_packetQueue.cpp  automation.cpp  filesel.cpp)
	
ADD_LIBRARY(ADM_toolkit6 STATIC ${ADM_toolkit_SRCS})
