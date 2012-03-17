include_directories(../include)
SET(ADM_editor_SRCS
ADM_edCache.cpp 
ADM_edIdentify.cpp 
ADM_edLoadSave.cpp 
ADM_edSearch.cpp 
ADM_edFrameType.cpp 
ADM_edit.cpp 
ADM_edRender.cpp 
ADM_edRenderInternal.cpp 
ADM_edMarker.cpp
ADM_edStub.cpp
ADM_edPtsDts.cpp
ADM_edVideoCopy.cpp
ADM_segment.cpp
ADM_edPySave.cpp
ADM_edAudioPacket.cpp
ADM_edAudioPcm.cpp
ADM_edAudioTrackFromVideo.cpp
ADM_edAudioTrack.cpp
ADM_editIface.cpp
)

ADD_LIBRARY(ADM_editor6 STATIC ${ADM_editor_SRCS})
include_directories(../ADM_script2/include)
