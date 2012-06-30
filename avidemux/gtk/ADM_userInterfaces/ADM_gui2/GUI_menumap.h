
/**
	Defines the binding between menu entries and core
	Since we use a common callback we don't use the
	glade generated one


*/

CALLBACK( connect_to_avsproxy1                  ,ACT_AVS_PROXY);
CALLBACK( see_hex1                              ,ACT_HEX_DUMP);
CALLBACK( see_size                              ,ACT_SIZE_DUMP);

CALLBACK( close1                                ,ACT_CLOSE);


CALLBACK( show_builtin_support1                 ,ACT_BUILT_IN);
CALLBACK( open_video1         			,ACT_OPEN_VIDEO);
CALLBACK( append_video1     			,ACT_APPEND_VIDEO);
CALLBACK( save_audio1         			,ACT_SAVE_AUDIO);
CALLBACK( save_as_avi1       			,ACT_SAVE_VIDEO);
CALLBACK(avi_muxer_options1 			,ACT_ContainerConfigure);

 CALLBACK(video_informations1	             ,ACT_VIDEO_PROPERTIES);
 CALLBACK(save_image1               		 ,ACT_SAVE_BMP);
 CALLBACK(save_jpg_image1              		 ,ACT_SAVE_JPG);
 CALLBACK(save_selection_as_jpegs1		     ,ACT_SAVE_BUNCH_OF_JPG);

 CALLBACK( play_video1         			    ,ACT_PlayAvi);
 CALLBACK( decoder_options1         		,ACT_DecoderOption);
 CALLBACK( set_postprocessing1			    ,ACT_SetPostProcessing);
 CALLBACK( next_frame1            		    ,ACT_NextFrame);
 CALLBACK( previous_frame1            		,ACT_PreviousFrame);
 CALLBACK( next_intra_frame1            	,ACT_NextKFrame);
 CALLBACK( previous_intra_frame1         	,ACT_PreviousKFrame);
 CALLBACK( jum_to_frame1               		,ACT_Goto);
 CALLBACK( jump_to_time1                        ,ACT_GotoTime);

 CALLBACK( copy1      				,ACT_Copy);
 CALLBACK( paste1      				,ACT_Paste);
 CALLBACK( delete1      			,ACT_Delete);
 CALLBACK( cut1					    ,ACT_Cut);
 CALLBACK( set_marker_a1      		,ACT_MarkA);
 CALLBACK( set_marker_b1      		,ACT_MarkB);
 CALLBACK( go_to_marker_a1  		,ACT_GotoMarkA);
 CALLBACK( go_to_marker_b1 			,ACT_GotoMarkB);

 CALLBACK( search_previous_black_frame1 ,ACT_PrevBlackFrame);
 CALLBACK( search_next_black_frame1		,ACT_NextBlackFrame);

CALLBACK(reset_edits1                		,ACT_ResetSegments);

CALLBACK(main_audio                                 ,ACT_AUDIO_SELECT_TRACK);
CALLBACK(filters2                                 ,ACT_AUDIO_FILTERS);

//CALLBACK(build_vbr_time_map1      		,ACT_AudioMap);

CALLBACK(videoencoder				    ,ACT_VIDEO_CODEC_CONFIGURE);
CALLBACK(audio_encoder1           		,ACT_AUDIO_CODEC_CONFIGURE);

//CALLBACK(bitrate_histogram1		      	,ACT_BitRate);

// CALLBACK(rebuild_frames           		,ACT_RebuildKF);
// CALLBACK(change_fps           			,ACT_ChangeFPS);
 CALLBACK(about1	           			,ACT_ABOUT);
//CALLBACK(calculator1                            ,ACT_Bitrate);
 CALLBACK(preferences1   			,ACT_PREFERENCES);
// CALLBACK(	check_frames			,ACT_VideoCheck);
 CALLBACK(quit1		   			,ACT_EXIT);


// CALLBACK( ocr_vobsub_2_srt                     ,ACT_Ocr);

// CALLBACK( ocr_dvb                     ,ACT_DVB_Ocr);


//CALLBACK(item1                                 ,ACT_AllBlackFrames);

CALLBACK(first_frame1                          ,ACT_Begin);
CALLBACK(last_frame1                           ,ACT_End);
CALLBACK(filters1                              ,ACT_VIDEO_FILTERS);
//CALLBACK(toolbar1                              ,ACT_ViewMain);
//CALLBACK(sidebar1                              ,ACT_ViewSide);
//CALLBACK(preview1                              ,ACT_PreviewToggle);
//CALLBACK(display_output1                       ,ACT_OutputToggle);
//CALLBACK(second_audio_track1                   ,ACT_SecondAudioTrack);
#if 0
CALLBACK(vcd1                                   ,ACT_AUTO_VCD);
CALLBACK(svcd1                                  ,ACT_AUTO_SVCD);
CALLBACK(dvd1                                   ,ACT_AUTO_DVD);
CALLBACK(psp1                                   ,ACT_AUTO_PSP);
CALLBACK(psp__h264_1,                           ACT_AUTO_PSP_H264);
CALLBACK(flv1,                                  ACT_AUTO_FLV);
CALLBACK(Ipod,                                  ACT_AUTO_IPOD);

CALLBACK(add_to_joblist1                       ,ACT_ADD_JOB);
CALLBACK(joblist1                              ,ACT_HANDLE_JOB);
CALLBACK(v2v                                   ,ACT_V2V);
#endif
CALLBACK(zoom_1_4                                   ,ACT_ZOOM_1_4);
CALLBACK(zoom_1_2                                   ,ACT_ZOOM_1_2);
CALLBACK(zoom_1_1                                   ,ACT_ZOOM_1_1);
CALLBACK(zoom_2_1                                   ,ACT_ZOOM_2_1);
//CALLBACK(zoom_4_1                                   ,ACT_ZOOM_4_1);
//CALLBACK(edit_glyph1                                ,ACT_GLYPHEDIT);
//CALLBACK(hscalVolume                                ,ACT_VOLUME);
CALLBACK(plugins1                                   ,ACT_PLUGIN_INFO);


