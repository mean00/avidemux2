SET(ADM_internalVideoFilter_SRCS
        ADM_pluginLoad.cpp
        ADM_videoFilters.cpp
        ADM_filterThread.cpp
)

include_directories("${PTHREAD_INCLUDE_DIR}")
ADD_LIBRARY(ADM_internalVideoFilter6 STATIC ${ADM_internalVideoFilter_SRCS})
