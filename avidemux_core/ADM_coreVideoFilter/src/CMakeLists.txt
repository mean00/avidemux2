SET(ADM_CoreVideoFilter_SRCS
    ADM_videoFilterBridge.cpp
	ADM_coreVideoFilter.cpp
	ADM_coreVideoFilterFunc.cpp
        DIA_flyDialog.cpp
        ADM_videoFilterCache.cpp
)

include_directories("${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common")
ADM_ADD_SHARED_LIBRARY(ADM_coreVideoFilter6 ${ADM_CoreVideoFilter_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreVideoFilter6 ADM_core6 ADM_coreImage6)

ADM_INSTALL_LIB(ADM_coreVideoFilter6)

