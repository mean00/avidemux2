SET(ADM_video_SRCS 
ADM_vidPartial.cpp    
ADM_vidRaw.cpp        
ADM_vidNull.cpp)

ADD_LIBRARY(ADM_video6 ${ADM_video_SRCS})

IF (USE_FREETYPE)
	ADD_TARGET_CFLAGS(ADM_video6 ${FREETYPE2_CFLAGS})
ENDIF (USE_FREETYPE)

ADD_TARGET_CFLAGS(ADM_video6 "-I../ADM_inputs")
#ADM_vidFieldUnblend.cpp  
