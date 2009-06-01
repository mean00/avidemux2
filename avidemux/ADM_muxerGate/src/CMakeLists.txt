SET(ADM_muxerGate_SRCS
ADM_videoCopy.cpp
ADM_videoProcess.cpp
ADM_createStreams.cpp
)
include_directories(../include)
ADD_LIBRARY(ADM_muxerGate STATIC ${ADM_muxerGate_SRCS})
