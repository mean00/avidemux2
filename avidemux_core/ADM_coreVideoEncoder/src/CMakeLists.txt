SET(ADM_CoreVideoEncoder_SRCS 
	ADM_coreVideoEncoder.cpp
        ADM_coreVideoEncoderFFmpeg.cpp
)

ADD_LIBRARY(ADM_coreVideoEncoder SHARED ${ADM_CoreVideoEncoder_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreVideoEncoder ADM_core ADM_coreImage ADM_libavcodec ADM_libavutil )

ADM_INSTALL_LIB(ADM_coreVideoEncoder)

