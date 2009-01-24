SET(ADM_audio_SRCS
audioac3ex.cpp  
audiomp3ex.cpp   
audio_packetizer.cpp  
audiotimeline.cpp
audiogen.cpp    
audio_mpegidentify.cpp  
audiosource_ext.cpp   
audiowavex.cpp
)
	
ADD_ADM_LIB_ALL_TARGETS(ADM_audio ${ADM_audio_SRCS})
