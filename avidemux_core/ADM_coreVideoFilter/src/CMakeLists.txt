SET(ADM_CoreVideoFilter_SRCS 
	ADM_coreVideoFilter.cpp
)

ADD_LIBRARY(ADM_coreVideoFilter SHARED ${ADM_CoreVideoFilter_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreVideoFilter ADM_core)

ADM_INSTALL_LIB(ADM_coreVideoFilter)

