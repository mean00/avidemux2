SET(ADM_CoreVideoFilter_SRCS 
	ADM_coreVideoFilter.cpp
        DIA_flyDialog.cpp
        ADM_videoFilterCache.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreVideoFilter6 ${ADM_CoreVideoFilter_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreVideoFilter6 ADM_core6 ADM_coreImage6)

ADM_INSTALL_LIB(ADM_coreVideoFilter6)

