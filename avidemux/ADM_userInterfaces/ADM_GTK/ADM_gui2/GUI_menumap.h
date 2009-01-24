
/**
	Defines the binding between menu entries and core
	Since we use a common callback we don't use the
	glade generated one


*/

CALLBACK( connect_to_avsproxy1                  ,ACT_AVS_PROXY);
CALLBACK( see_hex1                              ,ACT_HEX_DUMP);

CALLBACK( close1                                ,ACT_CLOSE);


CALLBACK( show_builtin_support1                 ,ACT_BUILT_IN);
CALLBACK( open_video1         			,ACT_OpenAvi); 
CALLBACK( append_video1     			,ACT_AppendAvi);
CALLBACK( save_audio1         			,ACT_SaveWave);
CALLBACK( save_as_avi1       			,ACT_SaveAvi);
CALLBACK(avi_muxer_options1 			,ACT_SetMuxParam);

//CALLBACK(load_project         			,ACT_OpenAvi); 
CALLBACK(save_project_as1              		,ACT_SaveWork);
CALLBACK(save_project1                          ,ACT_SaveCurrentWork);
//CALLBACK(save_current_project                	,ACT_SaveCurrentWork);
 
 CALLBACK(video_informations1	                 ,ACT_AviInfo);
 CALLBACK(save_image1               		 ,ACT_SaveImg);
 CALLBACK(save_jpg_image1              		 ,ACT_SaveJPG);
 CALLBACK(save_selection_as_jpegs1		  ,ACT_SaveBunchJPG);

 CALLBACK( play_video1         			,ACT_PlayAvi);
 CALLBACK( decoder_options1         		,ACT_DecoderOption);
 CALLBACK( set_postprocessing1			,ACT_SetPostProcessing);
 CALLBACK( next_frame1            		,ACT_NextFrame);
 CALLBACK( previous_frame1            		,ACT_PreviousFrame);
 CALLBACK( next_intra_frame1            	,ACT_NextKFrame);
 CALLBACK( previous_intra_frame1         	,ACT_PreviousKFrame);
 CALLBACK( jum_to_frame1               		,ACT_Goto);
 CALLBACK( jump_to_time1                        ,ACT_GotoTime);

 CALLBACK( copy1      				,ACT_Copy);
 CALLBACK( paste1      				,ACT_Paste);
 CALLBACK( delete1      			,ACT_Delete);
 CALLBACK( cut1					,ACT_Cut);
 CALLBACK( set_marker_a1      			,ACT_MarkA);
 CALLBACK( set_marker_b1      			,ACT_MarkB);
 CALLBACK( go_to_marker_a1  			,ACT_GotoMarkA);
 CALLBACK( go_to_marker_b1 			,ACT_GotoMarkB);

 CALLBACK( search_previous_black_frame1  	,ACT_PrevBlackFrame);
 CALLBACK( search_next_black_frame1		,ACT_NextBlackFrame);
 
CALLBACK(reset_edits1                		,ACT_ResetSegments);

CALLBACK(main_audio                                 ,ACT_SelectTrack1);
CALLBACK(filters2                                 ,ACT_AudioFilters);

CALLBACK(build_vbr_time_map1      		,ACT_AudioMap);

CALLBACK(videoencoder				,ACT_VideoConfigure);
CALLBACK(audio_encoder1           		,ACT_AudioConfigure);

CALLBACK(bitrate_histogram1		      	,ACT_BitRate);

 CALLBACK(rebuild_frames           		,ACT_RebuildKF);
 CALLBACK(change_fps           			,ACT_ChangeFPS);
 CALLBACK(about1	           			,ACT_About);
CALLBACK(calculator1                            ,ACT_Bitrate);
 CALLBACK(preferences1   			,ACT_Pref);
 CALLBACK(	check_frames			,ACT_VideoCheck);
 CALLBACK(quit1		   			,ACT_Exit);
 
 
 CALLBACK(run_script1		   	        ,ACT_RunScript);
 CALLBACK( ocr_vobsub_2_srt                     ,ACT_Ocr);
 
 CALLBACK( ocr_dvb                     ,ACT_DVB_Ocr);
 
 
CALLBACK(item1                                 ,ACT_AllBlackFrames);

CALLBACK(first_frame1                          ,ACT_Begin);
CALLBACK(last_frame1                           ,ACT_End);
CALLBACK(filters1                              ,ACT_VideoParameter);
CALLBACK(toolbar1                              ,ACT_ViewMain);
CALLBACK(sidebar1                              ,ACT_ViewSide);
//CALLBACK(preview1                              ,ACT_PreviewToggle);
//CALLBACK(display_output1                       ,ACT_OutputToggle);
CALLBACK(second_audio_track1                   ,ACT_SecondAudioTrack);

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

CALLBACK(zoom_1_4                                   ,ACT_ZOOM_1_4);
CALLBACK(zoom_1_2                                   ,ACT_ZOOM_1_2);
CALLBACK(zoom_1_1                                   ,ACT_ZOOM_1_1);
CALLBACK(zoom_2_1                                   ,ACT_ZOOM_2_1);
//CALLBACK(zoom_4_1                                   ,ACT_ZOOM_4_1);
CALLBACK(edit_glyph1                                ,ACT_GLYPHEDIT);
//CALLBACK(hscalVolume                                ,ACT_VOLUME);
CALLBACK(plugins1                                   ,ACT_PLUGIN_INFO);


