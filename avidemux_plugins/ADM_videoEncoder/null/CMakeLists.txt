INCLUDE(ve_plugin)

SET(null_SRCS 
        nullEncoder.cpp  
        nullPlugin.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(ADM_ve_null SHARED ${null_SRCS})

INIT_VIDEO_ENCODER(ADM_ve_null)
INSTALL_VIDEO_ENCODER(ADM_ve_null)
