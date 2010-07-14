
SET(ADM_vdpau_SRCS
ADM_coreVdpau.cpp
)	
#*************************************************
#*************************************************
ADD_LIBRARY(ADM_coreVDPAU6 SHARED ${ADM_vdpau_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreVdpau/include)
TARGET_LINK_LIBRARIES(ADM_coreVDPAU6 ADM_core6 ADM_coreUI6 )

ADM_INSTALL_LIB(ADM_coreVDPAU6)
