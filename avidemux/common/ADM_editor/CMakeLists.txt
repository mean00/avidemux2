SET(ADM_editor_SRCS
ADM_edCache.cpp 
ADM_edIdentify.cpp 
ADM_edLoadSave.cpp 
ADM_edSearch.cpp 
ADMedAVIAUD.cpp
ADM_edFrameType.cpp 
ADM_edit.cpp 
ADM_edRender.cpp 
ADM_edRenderInternal.cpp 
ADM_edMarker.cpp
ADM_edStub.cpp
ADM_edPtsDts.cpp
ADM_segment.cpp
)

ADD_LIBRARY(ADM_editor6 STATIC ${ADM_editor_SRCS})
