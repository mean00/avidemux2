#define LIST_OF_BUTTONS     PROCESS(pushButtonVideoConf, ACT_VIDEO_CODEC_CONFIGURE)  \
PROCESS(pushButtonVideoFilter, ACT_VIDEO_FILTERS) \
PROCESS(pushButtonAudioConf, ACT_AUDIO_CODEC_CONFIGURE) \
PROCESS(pushButtonAudioFilter, ACT_AUDIO_FILTERS) \
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
PROCESS(pushButtonTime, ACT_GotoTime) \
PROCESS(toolButtonBackOneMinute, ACT_Back1Mn) \
PROCESS(toolButtonForwardOneMinute, ACT_Forward1Mn) \

