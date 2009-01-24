SET(ADM_editor_SRCS
ADM_edAudio.cpp 
ADM_edCache.cpp 
ADM_edIdentify.cpp 
ADM_edLoadSave.cpp 
ADM_edSearch.cpp 
ADMedAVIAUD.cpp
ADM_edFrameType.cpp 
ADM_edit.cpp 
ADM_edRender.cpp 
ADM_edStub.cpp)

ADD_ADM_LIB_ALL_TARGETS(ADM_editor ${ADM_editor_SRCS})
