
SET(ADM_libvaEnc_SRCS
ADM_coreLibVA_buffer.cpp
ADM_coreLibVA_encodingContext.cpp
ADM_coreLibVA_buffer.cpp
#ADM_coreLibVA_encoder.cpp
ADM_coreLibVA_bitstream.cpp
)

INCLUDE_DIRECTORIES(${LIBVA_INCLUDE_DIR})	
ADD_DEFINITIONS(${LIBVA_CFLAGS})
#*************************************************
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_coreLibVAEnc6 ${ADM_libvaEnc_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreVideoCodec/ADM_hwAccel/ADM_coreLibVA/include)
TARGET_LINK_LIBRARIES(ADM_coreLibVAEnc6 va va-x11)
TARGET_LINK_LIBRARIES(ADM_coreVideoEncoder6)
TARGET_LINK_LIBRARIES(ADM_coreLibVAEnc6 ADM_core6 ADM_coreUI6 ADM_coreImage6 ADM_coreLibVA6 ADM_coreVideoEncoder6)
TARGET_LINK_LIBRARIES(ADM_coreLibVAEnc6 "c")

ADM_INSTALL_LIB(ADM_coreLibVAEnc6)

