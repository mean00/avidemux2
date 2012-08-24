SET(ADM_CoreVideoEncoder_SRCS 
	ADM_coreVideoEncoder.cpp
        ADM_coreVideoEncoderFFmpeg.cpp
)

add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreVideoEncoder6 ${ADM_CoreVideoEncoder_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreVideoEncoder6 ADM_core6 ADM_coreUtils6 ADM_coreImage6 ADM_libavcodec ADM_libavutil)

if (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)
	add_dependencies(ADM_coreVideoEncoder6 libavcodec libavutil)
endif (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)

ADM_INSTALL_LIB(ADM_coreVideoEncoder6)
