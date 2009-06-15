SET(ADM_internalVideoEncoder_SRCS 
	ADM_VideoEncoders.cpp
        ADM_dynVideoEncoder.cpp
)
ADD_LIBRARY(ADM_videoEncoder STATIC ${ADM_internalVideoEncoder_SRCS})
