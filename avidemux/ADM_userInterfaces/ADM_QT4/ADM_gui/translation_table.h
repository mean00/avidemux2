#define LIST_OF_OBJECTS     \
PROCESS(actionOpen,ACT_OpenAvi) \
PROCESS(actionClose,ACT_CLOSE) \
PROCESS(actionAppend,ACT_AppendAvi) \
PROCESS(actionQuit,ACT_Exit) \
PROCESS(actionCut,ACT_Cut) \
PROCESS(actionCopy,ACT_Copy) \
PROCESS(actionPaste,ACT_Paste) \
PROCESS(actionDelete,ACT_Delete) \
PROCESS(actionSet_marker_A,ACT_MarkA) \
PROCESS(actionSet_marker_B,ACT_MarkB) \
PROCESS(actionPreferences,ACT_Pref) \
PROCESS(actionProperties,ACT_AviInfo) \
PROCESS(actionSave_video,ACT_SaveAvi) \
PROCESS(actionSave_BMP,ACT_SaveImg) \
PROCESS(actionSave_jpeg,ACT_SaveJPG) \
PROCESS(actionLoad_run_project,ACT_RunScript) \
PROCESS(actionSave_project,ACT_SaveCurrentWork) \
PROCESS(actionSave_project_as,ACT_SaveWork) \
PROCESS(actionConnect_to_AvsProxy,ACT_AVS_PROXY) \
PROCESS(actionReset_Edits,ACT_ResetSegments) \
PROCESS(actionOCR_DVB_T_TS_files,ACT_DVB_Ocr) \
PROCESS(actionGlyphs_Edit,ACT_GLYPHEDIT) \
PROCESS(actionZoom_1_4,ACT_ZOOM_1_4) \
PROCESS(actionZoom_1_2,ACT_ZOOM_1_2) \
PROCESS(actionZoom_1_1,ACT_ZOOM_1_1) \
PROCESS(actionZoom_2_1,ACT_ZOOM_2_1) \
PROCESS(actionDecoder_options,ACT_DecoderOption) \
PROCESS(actionPostprocessing,ACT_SetPostProcessing) \
PROCESS(actionFrame_rate,ACT_ChangeFPS) \
PROCESS(actionEncoder,ACT_SelectEncoder) \
PROCESS(actionFilters,ACT_VideoParameter) \
PROCESS(actionMain_Track,ACT_SelectTrack1) \
PROCESS(actionSecondary_Track,ACT_SecondAudioTrack) \
PROCESS(actionBuild_VBR_time_map,ACT_AudioMap) \
PROCESS(actionSave_2,ACT_SaveWave) \
PROCESS(actionEncoder_2,ACT_SelectEncoder) \
PROCESS(actionFilters_2,ACT_AudioFilters) \
PROCESS(actionCalculator,ACT_Bitrate) \
PROCESS(actionRebuild_I_B_Frames,ACT_RebuildKF) \
PROCESS(actionBitrate_histogram,ACT_BitRate) \
PROCESS(actionScan_for_black_frames,ACT_AllBlackFrames) \
PROCESS(actionVob_to_vobsub,ACT_V2V) \
PROCESS(actionOCR,ACT_Ocr) \
PROCESS(actionPlay_Stop,ACT_StopAvi) \
PROCESS(actionPrevious_Frame,ACT_PreviousFrame) \
PROCESS(actionNext_Frame,ACT_NextFrame) \
PROCESS(actionPrevious_black_frame,ACT_PrevBlackFrame) \
PROCESS(actionNext_blak_frame,ACT_NextBlackFrame) \
PROCESS(actionFirst_Frame,ACT_Begin) \
PROCESS(actionLast_Frame,ACT_End) \
PROCESS(actionGo_to_Marker_A,ACT_GotoMarkA) \
PROCESS(actionGo_to_Marker_B,ACT_GotoMarkB) \
PROCESS(actionJump_to_Frame,ACT_Goto) \
PROCESS(actionJump_to_Time,ACT_GotoTime) \
PROCESS(actionShow_built_in_support,ACT_BUILT_IN) \
PROCESS(actionAbout_avidemux,ACT_About) \
PROCESS(actionPlay,ACT_PlayAvi) \
PROCESS(actionRecent0,ACT_RECENT0) \
PROCESS(actionRecent1,ACT_RECENT1) \
PROCESS(actionRecent2,ACT_RECENT2) \
PROCESS(actionRecent3,ACT_RECENT3) \
PROCESS(actionVCD,ACT_AUTO_VCD) \
PROCESS(actionSVCD,ACT_AUTO_SVCD) \
PROCESS(actionDVD,ACT_AUTO_DVD) \
PROCESS(actionPSP,ACT_AUTO_PSP) \
PROCESS(actionFLV,ACT_AUTO_FLV) \
PROCESS(actionPSP_H264,ACT_AUTO_PSP_H264)\
PROCESS(actionIPOD,ACT_AUTO_IPOD) \
PROCESS(actionAdd_to_joblist,ACT_ADD_JOB) \
PROCESS(actionShow_Joblist,ACT_HANDLE_JOB)  

#if 0
PROCESS(actionPrevious_intra_frame,ACT_PreviousKFrame) \
PROCESS(actionNext_intra_frame,ACT_NextKFrame) \
PROCESS(actionZoom_4_1,ACT_ZOOM_4_1) \
PROCESS(actionA_V_toolbar,ACT_DUMMY)
#endif

#define LIST_OF_BUTTONS     PROCESS(pushButtonVideoConf, ACT_VideoCodec)  \
PROCESS(pushButtonVideoFilter, ACT_VideoParameter) \
PROCESS(pushButtonAudioConf, ACT_AudioCodec) \
PROCESS(pushButtonAudioFilter, ACT_AudioFilters) \
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
PROCESS(pushButtonJumpToMarkerB, ACT_GotoMarkB)
