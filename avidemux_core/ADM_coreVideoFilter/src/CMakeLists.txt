SET(ADM_CoreVideoFilter_SRCS 
	ADM_coreVideoFilter.cpp
)

ADD_LIBRARY(ADM_coreVideoFilter SHARED ${ADM_CoreVideoFilter_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreVideoFilter ADM_core)

INSTALL(TARGETS ADM_coreVideoFilter RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)

SET(ADM_internalVideoFilter_SRCS 
	ADM_videoFilterBridge.cpp
        ADM_filterChain.cpp
)
#ADD_LIBRARY(ADM_internalVideoFilter STATIC ${ADM_internalVideoFilter_SRCS})
