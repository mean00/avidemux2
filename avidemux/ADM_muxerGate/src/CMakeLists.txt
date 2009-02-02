SET(ADM_muxerGate_SRCS
ADM_videoCopy.cpp
)
include_directories(../include)
ADD_ADM_LIB_ALL_TARGETS(ADM_muxerGate ${ADM_muxerGate_SRCS})
