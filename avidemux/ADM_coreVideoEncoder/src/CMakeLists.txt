SET(ADM_CoreVideoEncoder_SRCS 
	ADM_coreVideoEncoder.cpp
)

ADD_LIBRARY(ADM_coreVideoEncoder SHARED ${ADM_CoreVideoEncoder_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreVideoEncoder ADM_core ADM_coreImage)

INSTALL(TARGETS ADM_coreVideoEncoder RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)

SET(ADM_internalVideoEncoder_SRCS 
	ADM_VideoEncoders.cpp
        ADM_dynVideoEncoder.cpp
)
ADD_LIBRARY(ADM_internalVideoEncoder STATIC ${ADM_internalVideoEncoder_SRCS})
