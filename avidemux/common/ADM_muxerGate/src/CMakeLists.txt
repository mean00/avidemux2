SET(ADM_muxerGate_SRCS
ADM_videoCopy.cpp
ADM_videoProcess.cpp
ADM_videoCopyFromAnnexB.cpp
ADM_videoCopyAudRemover.cpp
ADM_videoCopySeiInjector.cpp
)
include_directories(../include)
ADD_LIBRARY(ADM_muxerGate6 STATIC ${ADM_muxerGate_SRCS})
TARGET_LINK_LIBRARIES(ADM_muxerGate6 PRIVATE  ADM_editor6_interface)
