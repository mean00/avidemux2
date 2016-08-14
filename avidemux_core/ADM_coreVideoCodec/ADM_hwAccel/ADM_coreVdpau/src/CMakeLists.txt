
SET(ADM_vdpau_SRCS
ADM_coreVdpau.cpp
ADM_coreVdpauMixer.cpp
)	
ADD_DEFINITIONS("-std=gnu++98")
#*************************************************
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_coreVDPAU6 ${ADM_vdpau_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreVdpau/include)
TARGET_LINK_LIBRARIES(ADM_coreVDPAU6 ADM_core6 ADM_coreUI6 )
ADM_INSTALL_LIB(ADM_coreVDPAU6)
