SET(ADM_internalVideoFilter_SRCS
        ADM_pluginLoad.cpp
        ADM_videoFilters.cpp
        ADM_filterThread.cpp
        ADM_vidPartial.cpp
)
IF (USE_VDPAU)
        ADD_CORE_INCLUDE(ADM_coreVdpau)
ENDIF (USE_VDPAU)
IF (USE_LIBVA)
        ADD_CORE_INCLUDE(ADM_coreLibVA)
ENDIF (USE_LIBVA)

include_directories("${PTHREAD_INCLUDE_DIR}")
ADD_LIBRARY(ADM_internalVideoFilter6 STATIC ${ADM_internalVideoFilter_SRCS})
