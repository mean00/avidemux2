SET(ADM_video_SRCS 
ADM_vidPartial.cpp    
ADM_vidRaw.cpp        
ADM_vidNull.cpp)

ADD_ADM_LIB_ALL_TARGETS(ADM_video ${ADM_video_SRCS})

IF (USE_FREETYPE)
	ADD_CFLAGS_ALL_TARGETS(ADM_video ${FREETYPE2_CFLAGS})
ENDIF (USE_FREETYPE)

ADD_CFLAGS_ALL_TARGETS(ADM_video "-I../ADM_inputs")
#ADM_vidFieldUnblend.cpp  
