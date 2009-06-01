SET(ADM_muxerGate_SRCS
ADM_videoCopy.cpp
ADM_videoProcess.cpp
)
include_directories(../include)
ADD_LIBRARY(ADM_muxerGate STATIC ${ADM_muxerGate_SRCS})
