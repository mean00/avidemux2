SET(ADM_LIB ADM_editor6)
ADD_LIBRARY(${ADM_LIB} STATIC)
TARGET_SOURCES(${ADM_LIB} PRIVATE
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

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_editor6_interface ADM_audioFilter6_interface
  ADM_videoFilter6_interface ADM_videoEncoder6_interface
  ADM_ui_interface ADM_core6 ADM_coreUtils6 ADM_script6 adm_pthread ADM_ui_interface)
#

ADD_LIBRARY(ADM_editor6_interface INTERFACE  )
TARGET_INCLUDE_DIRECTORIES( ADM_editor6_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES( ADM_editor6_interface INTERFACE ADM_ui_interface)


