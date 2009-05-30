SET(ADM_muxerGate_SRCS
ADM_videoCopy.cpp
)
include_directories(../include)
ADD_LIBRARY(ADM_muxerGate STATIC ${ADM_muxerGate_SRCS})
