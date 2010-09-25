SET(ADM_muxerGate_SRCS
ADM_videoCopy.cpp
ADM_videoProcess.cpp
ADM_createStreams.cpp
ADM_videoCopyFromAnnexB.cpp
)
include_directories(../include)
ADD_LIBRARY(ADM_muxerGate6 STATIC ${ADM_muxerGate_SRCS})
