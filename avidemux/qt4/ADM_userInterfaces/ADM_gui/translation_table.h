#define LIST_OF_OBJECTS     \
PROCESS(actionZoom_1_4,ACT_ZOOM_1_4) \
PROCESS(actionZoom_1_2,ACT_ZOOM_1_2) \
PROCESS(actionZoom_1_1,ACT_ZOOM_1_1) \
PROCESS(actionZoom_2_1,ACT_ZOOM_2_1) \

#define LIST_OF_BUTTONS     PROCESS(pushButtonVideoConf, ACT_VideoCodec)  \
PROCESS(pushButtonVideoFilter, ACT_VideoParameter) \
PROCESS(pushButtonAudioConf, ACT_AudioCodec) \
PROCESS(pushButtonAudioFilter, ACT_AudioFilters) \
PROCESS(pushButtonDecoderConf, ACT_DecoderOption) \
PROCESS(pushButtonFormatConfigure, ACT_ContainerConfigure) \
PROCESS(toolButtonPlay, ACT_PlayAvi) \
PROCESS(toolButtonStop, ACT_StopAvi) \
PROCESS(toolButtonPreviousFrame, ACT_PreviousFrame) \
PROCESS(toolButtonNextFrame, ACT_NextFrame) \
PROCESS(toolButtonPreviousIntraFrame, ACT_PreviousKFrame) \
PROCESS(toolButtonNextIntraFrame, ACT_NextKFrame) \
PROCESS(toolButtonSetMarkerA, ACT_MarkA) \
PROCESS(toolButtonSetMarkerB, ACT_MarkB) \
PROCESS(toolButtonPreviousBlackFrame, ACT_PrevBlackFrame) \
PROCESS(toolButtonNextBlackFrame, ACT_NextBlackFrame) \
PROCESS(toolButtonFirstFrame, ACT_Begin) \
PROCESS(toolButtonLastFrame, ACT_End) \
PROCESS(pushButtonJumpToMarkerA, ACT_GotoMarkA) \
PROCESS(pushButtonJumpToMarkerB, ACT_GotoMarkB) \
PROCESS(pushButtonTime, ACT_GotoTime)
