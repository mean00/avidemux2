SET(ADM_internalVideoFilter_SRCS 
	ADM_videoFilterBridge.cpp
        ADM_pluginLoad.cpp
        ADM_videoFilters.cpp
        ADM_filterThread.cpp
)
ADD_LIBRARY(ADM_internalVideoFilter6 STATIC ${ADM_internalVideoFilter_SRCS})
