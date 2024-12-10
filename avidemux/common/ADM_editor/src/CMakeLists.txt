if (NOT MSVC)
  add_definitions("-std=c++11")
endif (NOT MSVC)
include_directories(../include)
SET(ADM_editor_SRCS
ADM_edit.cpp
ADM_edRender.cpp
ADM_edRenderInternal.cpp
ADM_edStub.cpp
ADM_edVideoCopy.cpp
ADM_segment.cpp
ADM_edSearch.cpp
utils/ADM_edCheckForInvalidPts.cpp
utils/ADM_edMarker.cpp
utils/ADM_edPtsDts.cpp
utils/ADM_edIdentify.cpp
utils/ADM_editIface.cpp
utils/ADM_edScriptGenerator.cpp
utils/ADM_edFrameType.cpp
utils/ADM_edCache.cpp
utils/ADM_edUndoQueue.cpp
audio/ADM_edEditableAudioTrack.cpp
audio/ADM_edPoolOfAudioTracks.cpp
audio/ADM_edActiveAudioTracks.cpp
audio/ADM_edAudioTrackExternal.cpp
audio/ADM_edAudioPacket.cpp
audio/ADM_edAudioPcm.cpp
audio/ADM_edAudioTrackFromVideo.cpp
audio/ADM_edAudioTrack.cpp
)

ADD_LIBRARY(ADM_editor6_interface INTERFACE  )
TARGET_INCLUDE_DIRECTORIES( ADM_editor6_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADD_LIBRARY(ADM_editor6 STATIC ${ADM_editor_SRCS})
include_directories(../ADM_script2/include)
TARGET_LINK_LIBRARIES(ADM_editor6 PUBLIC ADM_editor6_interface ADM_audioFilter6_interface
  ADM_videoFilter6_interface ADM_videoEncoder6_interface
  ADM_commonUI6_interface)


