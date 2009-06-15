SET(ADM_internalVideoFilter_SRCS 
	ADM_videoFilterBridge.cpp
        ADM_filterChain.cpp
)
ADD_LIBRARY(ADM_internalVideoFilter STATIC ${ADM_internalVideoFilter_SRCS})
