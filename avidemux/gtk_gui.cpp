/***************************************************************************
                          gtk_gui.cpp  -  description
                             -------------------
    begin                : Mon Dec 10 2001
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "ADM_lavcodec.h"
#include "fourcc.h"
#include "avi_vars.h"
#include "ADM_assert.h"
#include "DIA_fileSel.h"
#include "prototype.h"
#include "DIA_coreToolkit.h"
#include "ADM_bitmap.h"

#include "ADM_audio/aviaudio.hxx"
#include "ADM_audio/audioex.h"
#include "ADM_audiofilter/audioprocess.hxx"
#include "gui_action.hxx"
#include "gtkgui.h"

//#include "ADM_outputs/oplug_avi/GUI_mux.h"
//#include "ADM_outputs/oplug_mpegFF/oplug_vcdff.h"
#include "ADM_audiofilter/audioeng_buildfilters.h"
#include "prefs.h"
#include "ADM_encoder/adm_encConfig.h"
#include "ADM_userInterfaces/ADM_render/GUI_render.h"
#include "ADM_userInterfaces/ADM_commonUI/GUI_ui.h"

#include "audio_out.h"

#include "ADM_userInterfaces/ADM_commonUI/DIA_busy.h"
#include "DIA_working.h"
#include "DIA_factory.h"
#include "DIA_enter.h"

#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_codecs/ADM_ffmpeg.h"
#include "ADM_libraries/ADM_libmpeg2enc/ADM_mpeg2enc.h"
#include "ADM_video/ADM_vidMisc.h"
#include "ADM_preview.h"
static AudioSource currentAudioSource = AudioAvi;
static AudioSource secondAudioSource = AudioNone;
static char *currentAudioName = NULL;
static char *secondAudioName = NULL;

void A_handleSecondTrack (int tracktype);
int A_delete(uint32_t start, uint32_t end);
void A_saveImg (const char *name);
void A_saveBunchJpg( const char *name);
void A_requantize(void);
int A_saveJpg (char *name);
int A_loadWave (char *name);
int A_loadAC3 (char *name);
int A_loadMP3 (char *name);
int A_loadNone( void );
void A_saveAudioDecodedTest (char *name);
void A_openBrokenAvi (const char *name);
int A_openAvi2 (const char *name, uint8_t mode);
int A_appendAvi (const char *name);
void A_externalAudioTrack( void );
uint8_t GUI_getFrameContent(ADMImage *image, uint32_t frame);
void HandleAction (Action action);
uint8_t A_rebuildKeyFrame (void);
extern int GUI_handleVFilter (void);
extern void filterCleanUp (void);
int A_audioSave(char *name);
static void ReSync (void);
static void cleanUp (void);
extern void GUI_setName (char *n);
extern void DIA_properties( void);
extern uint8_t DIA_Preferences(void);
extern void  GUI_displayBitrate( void );
void test_mpeg (char *name);
uint8_t GUI_getDoubleValue (double *valye, float min, float max,
			    const char *title);
extern void GUI_setMarks (uint32_t a, uint32_t b);
extern void saveMpegFile (char *name);
//static void A_selectEncoder ( void );
extern uint8_t A_SaveAudioDualAudio (const char *a);

extern uint8_t ADM_aviUISetMuxer(  void );
void A_Resync(void);
void A_addJob(void);
static void updateSecondAudioTrack (void);
void A_audioTrack(void);
extern int A_Save(const char *name);
int A_SaveWrapper( char *name);
static uint32_t getAudioByteCount( uint32_t start, uint32_t end);
extern void mpegToIndex (char *name);
static void A_mpegIndexer (void);
extern uint8_t indexMpeg (char *mpeg, char *file, uint8_t aid);
void ADM_cutWizard (void);
uint8_t  ADM_saveRaw (const char *name);
char * actual_workbench_file;
void A_saveWorkbench (const char *name);
void updateLoaded (void);
extern void encoderSetLogFile (char *name);
extern void videoCodecSelect (void);
extern uint8_t DIA_about( void );
extern uint8_t DIA_RecentFiles( char **name );
extern uint8_t mpeg_passthrough(const char *name,ADM_OUT_FORMAT format );
static void A_videoCheck( void);
extern uint8_t parseScript(char *scriptname);
static void	A_setPostproc( void );
extern uint8_t ogmSave(const char  *name);
//
static uint8_t A_pass(char *name);
uint8_t A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms);
//__________
extern uint8_t DIA_gotoTime(uint16_t *hh, uint16_t *mm, uint16_t *ss);
extern uint8_t GUI_getFrame(uint32_t frameno,  uint32_t *flags);
extern int A_SaveUnpackedVop(const char *name);
extern int A_SavePackedVop(const char *name);
extern void videoCodecConfigureUI(int codecIndex = -1);
extern void audioCodecChanged(int newcodec);
extern void videoCodecChanged(int newcodec);
extern void DIA_Calculator(uint32_t *sizeInMeg, uint32_t *avgBitrate );
extern uint8_t A_autoDrive(Action action);


extern uint8_t DIA_ocrGen(void);
extern uint8_t DIA_ocrDvb(void);
uint8_t A_TimeShift(void);
//PARAM_MUX muxMode = MUX_REGULAR;
int muxParam = 0;

extern uint8_t UI_getPhysicalScreenSize(void* window, uint32_t *w,uint32_t *h);
extern uint8_t GUI_jobs(void);
extern bool parseECMAScript(const char *name);
void A_parseECMAScript(const char *name);
//static int A_vob2vobsub(void);
uint8_t DIA_builtin(void);
renderZoom currentZoom=ZOOM_1_1;
uint8_t A_setSecondAudioTrack(const AudioSource nw,char *name);
extern const char * GUI_getCustomScript(uint32_t nb);


void GUI_showCurrentFrameHex(void);
void GUI_avsProxy(void);
uint8_t GUI_close(void);
extern void A_jog(void);
extern void DIA_glyphEdit(void);
extern uint8_t DIA_pluginsInfo(void);
extern bool ADM_mux_configure(int index);
void HandleAction_Navigate(Action action);
//___________________________________________
// serialization of user event through gui
//
// Independant from the toolkit used
// QT or other port should be easy
//___________________________________________

void HandleAction (Action action)
{
  static int recursive = 0;

  uint32_t nf = 0;
  uint32_t old;
  // handle out of band actions
  // independant load not loaded
//------------------------------------------------
int nw;
  if(action>=ACT_CUSTOM_BASE && action <ACT_CUSTOM_END)
  {
      int i=action-ACT_CUSTOM_BASE;
      const char *custom=GUI_getCustomScript(i);
      A_parseECMAScript(custom);
      return ;
  }
  switch (action)
    {
        case ACT_AVS_PROXY:
                                GUI_avsProxy();
                                return;
        case ACT_BUILT_IN:
                                DIA_builtin();
                                return;
#if BAZOOKA
        case ACT_GLYPHEDIT:
                                DIA_glyphEdit();
                                return;

        case ACT_V2V:
                                A_vob2vobsub();
                                return;
#endif
        case ACT_HANDLE_JOB:
                                GUI_jobs();
                                return;
        case ACT_RECENT0:
        case ACT_RECENT1:
        case ACT_RECENT2:
        case ACT_RECENT3:
                        const char **name;
						char* fileName;
                        int rank;
                                name=prefs->get_lastfiles();
                                rank=(int)action-ACT_RECENT0;
                                ADM_assert(name[rank]);
                                A_openAvi2 (name[rank], 0);

                return;
        case ACT_ViewMain: UI_toogleMain();return;
        case ACT_ViewSide: UI_toogleSide();return;
#ifdef BAZOOKA
        case ACT_DVB_Ocr:
        		DIA_ocrDvb();
        		return;

      case ACT_Ocr:
                DIA_ocrGen(); //
                return;
#endif
      case ACT_AudioConfigure:
    		audioCodecSelect();
		return;
	case ACT_VideoConfigure:
    		videoCodecSelect();
		return;
    case ACT_VideoCodecChanged:
    		nw=UI_getCurrentVCodec();
    		videoCodecChanged(nw);
		return;
   case ACT_AudioCodecChanged:
                nw=UI_getCurrentACodec();
                audioCodecSetByIndex(nw);

		return;
    case ACT_PLUGIN_INFO:
            DIA_pluginsInfo();
            return;
    case ACT_RunScript:
                GUI_FileSelRead (QT_TR_NOOP("Select ECMAScript to Run"),(SELFILE_CB *) A_parseECMAScript);

                        //
    		return;

    case ACT_RecentFiles:
    	char *file;
		if(		DIA_RecentFiles(&file))
		{
			A_openAvi2 (file, 0);
		}
		return;
    case ACT_About :
    		 DIA_about( );
		 return;
    case ACT_VideoCodec:
      //videoCodecSelect ();
      videoCodecConfigureUI();
      return;
    case ACT_AudioCodec:
      //audioCodecSelect ();
      audioCodecConfigure();
      return;

    case ACT_AudioFilters:
      audioFilter_configureFilters ();
      return;
    case ACT_Pref:
        if(playing) return;
    	if(DIA_Preferences())
	{
	saveEncoderConfig ();
 	#ifdef HAVE_AUDIO
      		AVDM_audioSave();
 	#endif
      	prefs->save ();
	}
	return;
    case ACT_SavePref:
      saveEncoderConfig ();
 #ifdef HAVE_AUDIO
      AVDM_audioSave();
 #endif
      prefs->save ();
      return;
    case ACT_SetLogFile:
//      GUI_FileSelWrite ("Select log File to use", encoderSetLogFile);
      GUI_Error_HIG(QT_TR_NOOP("Obsolete"), NULL);
      return;
      break;
    case ACT_SetMuxParam:
        {
        int index=UI_GetCurrentFormat();
        ADM_mux_configure(index);
        }
      return;
      break;
    case ACT_Fast:
      ADM_assert(0);
      break;
    case ACT_Exit:
      { uint32_t saveprefsonexit;
         prefs->get(FEATURE_SAVEPREFSONEXIT,&saveprefsonexit);
         if( saveprefsonexit )
            prefs->save ();
      }
      cleanUp ();
      exit (0);
      break;
/*			case ACT_SelectEncoder:

						A_selectEncoder();
  	  			return;
						break;*/
    case ACT_SelectEncoder:
      GUI_Error_HIG (QT_TR_NOOP("Obsolete"), NULL);
      break;

    default:
      break;

    }
//------------------------------------------------

  if (playing)			// only allow some action
    {
      switch (action)
	{
	case ACT_PlayAvi:

	case ACT_StopAvi:
	  A_playAvi ();
	  break;
	default:
	  return;
	}
    }
  // not playing
  // allow all actions

  // restict disabled uncoded actions
  if ((int) action >= ACT_DUMMY)
    {
      GUI_Error_HIG (QT_TR_NOOP("Not coded in this version"), NULL);
      return;

    }
  // allow only if avi loaded
  if (!avifileinfo)
    {
      switch (action)
	{
          case ACT_JOG:
                break;
	case ACT_OpenAvi:
          GUI_FileSelRead (QT_TR_NOOP("Select AVI File..."), (SELFILE_CB *)A_openAvi);
	  break;

	case ACT_BrokenAvi:
	  printf ("\n Opening in broken mode...\n");
          GUI_FileSelRead (QT_TR_NOOP("Select AVI File..."), A_openBrokenAvi);
	  break;


	default:
	  ;
	}
      return;
    }

//#define TEST_UNPACK
  // we have an AVI loaded
  if(action>ACT_NAVIGATE_BEGIN && action < ACT_NAVIGATE_END)
  {
    HandleAction_Navigate(action);
  }
  else switch (action)
    {
       case ACT_JOG:
                A_jog();
                break;

       case ACT_CLOSE:
              GUI_close();
              break;

        case ACT_ZOOM_1_4:
        case ACT_ZOOM_1_2:
        case ACT_ZOOM_1_1:
        case ACT_ZOOM_2_1:
        case ACT_ZOOM_4_1:
                currentZoom=(renderZoom)((action-ACT_ZOOM_1_4)+ZOOM_1_4);
                changePreviewZoom(currentZoom);
                admPreview::samePicture();
                break;


        case ACT_AUTO_VCD:
        case ACT_AUTO_SVCD:
        case ACT_AUTO_DVD:
        case ACT_AUTO_PSP:
        case ACT_AUTO_IPOD:
        case ACT_AUTO_FLV:
        case ACT_AUTO_PSP_H264:
                A_autoDrive( action);
                break;

   case ACT_SelectTrack1:
                A_audioTrack();
                break;

    case ACT_Bitrate:
    			{
				uint32_t a,b;
//				DIA_Calculator(&a,&b );
			}
    			break;
    case ACT_SaveUnpackedMpeg4:
      if(GUI_Question(QT_TR_NOOP("This is to be used to undo packed VOP on MPEG-4.\nContinue ?")))
			{
                          GUI_FileSelWrite (QT_TR_NOOP("Select AVI File to Write"), (SELFILE_CB *)A_SaveUnpackedVop);

			}
    			break;

    case ACT_SaveOGM:
//                        GUI_FileSelWrite (QT_TR_NOOP("Select OGM File to Write"), (SELFILE_CB *)ogmSave);
    			break;

    case ACT_SaveWork:
      GUI_FileSelWrite (QT_TR_NOOP("Select Workbench to Save"), A_saveWorkbench);
	  UI_refreshCustomMenu();
      break;
    case ACT_ADD_JOB:
        A_addJob();
        break;
    case ACT_SaveCurrentWork:
      if( actual_workbench_file ){
        char *tmp = ADM_strdup(actual_workbench_file);
         A_saveWorkbench( tmp ); // will write "actual_workbench_file" itself
         ADM_dealloc(tmp);
      }else{
        GUI_FileSelWrite (QT_TR_NOOP("Select Workbench to Save"), A_saveWorkbench);
		UI_refreshCustomMenu();
      }
      break;
      
    case ACT_SaveRaw:
      GUI_FileSelWrite (QT_TR_NOOP("Select Raw File to Save"), (SELFILE_CB *)ADM_saveRaw);
      break;
    case ACT_CutWizard:
      ADM_cutWizard ();
      break;
    case ACT_SecondAudioTrack:
                A_externalAudioTrack();
                break;

    case ACT_OpenAvi:
      GUI_FileSelRead (QT_TR_NOOP("Select AVI File..."),(SELFILE_CB *) A_openAvi);
      break;
    case ACT_BrokenAvi:
      GUI_FileSelRead (QT_TR_NOOP("Select AVI File..."), A_openBrokenAvi);
      break;
    case ACT_AppendAvi:
      GUI_FileSelRead (QT_TR_NOOP("Select AVI File to Append..."),(SELFILE_CB *) A_appendAvi);
      break;
    case ACT_SaveWave:
      	{
          GUI_FileSelWrite (QT_TR_NOOP("Select File to Save Audio"),(SELFILE_CB *)A_audioSave);

	}
      break;


    case ACT_AviInfo:
      DIA_properties ();
      break;
  
	case ACT_BitRate:
		 GUI_displayBitrate(  );
	break;

 

    case ACT_PlayAvi:
      GUI_PlayAvi ();
      break;

    case ACT_SaveDualAudio:
      //GUI_FileSelWrite ("Select AVI to save ",
      A_SaveAudioDualAudio(NULL);
      break;

    case ACT_SaveBunchJPG:
      GUI_FileSelWrite (QT_TR_NOOP("Select JPEG Sequence to Save"), A_saveBunchJpg);
    	break;
    case ACT_SaveImg:
      GUI_FileSelWrite (QT_TR_NOOP("Select BMP to Save"), A_saveImg);
      //GUI_FileSelWrite ("Select Jpg to save ", A_saveJpg);
      break;
    case ACT_SaveJPG :
      GUI_FileSelWrite (QT_TR_NOOP("Select JPEG to Save"), (SELFILE_CB *)A_saveJpg);
      	//GUI_FileSelWrite ("Select Jpg to save ", A_saveJpg);
      	break;

  
#define TOGGLE_PREVIEW ADM_PREVIEW_OUTPUT
    case ACT_PreviewChanged:
    {
        ADM_PREVIEW_MODE oldpreview=getPreviewMode(),newpreview=(ADM_PREVIEW_MODE)UI_getCurrentPreview();
          printf("Old preview %d, New preview mode : %d\n",oldpreview,newpreview);

          if(oldpreview==newpreview)
          {
            return;
          }
            admPreview::stop();
            setPreviewMode(newpreview);
            admPreview::start();
//            admPreview::update(curframe);
      }
      break;
    case ACT_StopAvi:
      if (playing)
	GUI_PlayAvi ();
      break;
    case ACT_SetPostProcessing:
      A_setPostproc();
      break;

   
    case ACT_AllBlackFrames:
      GUI_FileSelWrite (QT_TR_NOOP("Select File to Save"), (SELFILE_CB *)A_ListAllBlackFrames);
        break;

 

 
    case ACT_AudioSourceAvi:
      //currentaudiostream=aviaudiostream;
      A_changeAudioStream (aviaudiostream, AudioAvi,NULL);
      //wavinfo= currentaudiostream->getInfo();
      break;

    case ACT_AudioSourceMP3:
      GUI_FileSelRead (QT_TR_NOOP("Select MP3 to Load"), (SELFILE_CB *)A_loadMP3);
      break;
    case ACT_AudioSourceAC3:
      GUI_FileSelRead (QT_TR_NOOP("Select AC3 to Load"), (SELFILE_CB *)A_loadAC3);
      break;
    case ACT_AudioSourceWAV:
      GUI_FileSelRead (QT_TR_NOOP("Select WAV to Load"),(SELFILE_CB *) A_loadWave);
      break;
    case ACT_AudioSourceNone:
      //currentaudiostream=(AVDMGenericAudioStream *)NULL;
//       A_changeAudioStream((AVDMGenericAudioStream *) NULL, AudioNone,NULL);
      break;


    case ACT_MarkA:
    case ACT_MarkB:
      uint32_t swapit;
      if( prefs->get(FEATURE_SWAP_IF_A_GREATER_THAN_B, &swapit) != RC_OK )
         swapit = 1;
      if (action == ACT_MarkA)
            frameStart = video_body->getCurrentFrame();
      else
            frameEnd = video_body->getCurrentFrame();
      if (frameStart > frameEnd && swapit )	// auto swap
	{
	  uint32_t y;
	  y = frameStart;
	  frameStart = frameEnd;
	  frameEnd = y;
	}
      UI_setMarkers (frameStart, frameEnd);
      break;
  
//----------------------test-----------------------
    case ACT_SaveAvi:
      GUI_FileSelWrite (QT_TR_NOOP("Select File to Save"),(SELFILE_CB *)A_SaveWrapper); // A_SaveAudioNVideo);
      break;
//---------------------------------------------------
    case ACT_Copy:
      		if( frameEnd < frameStart ){
                  GUI_Error_HIG(QT_TR_NOOP("Marker A > B"), QT_TR_NOOP("Cannot copy."));
      		}else{
    		   video_body->copyToClipBoard (frameStart,frameEnd);
      		}
		break;
    case ACT_Paste:
#if 0
      		video_body->pasteFromClipBoard(curframe);
		 old=curframe;
      		ReSync ();
	  	if (!video_body->updateVideoInfo (avifileinfo))
		{
                  GUI_Error_HIG (QT_TR_NOOP("Something bad happened (II))"), NULL);
		}
     		 GUI_setAllFrameAndTime ();
      		UI_setMarkers (frameStart, frameEnd);
 		curframe=old;
        	GUI_GoToFrame (curframe);
#endif
		break;
      break;

    case ACT_VideoCheck:
    		A_videoCheck();
		break;
    case ACT_ResetSegments:
       if(avifileinfo)
         if(GUI_Question(QT_TR_NOOP("Are you sure?")))
	{
		video_body->resetSeg();
  		video_body->getVideoInfo (avifileinfo);
		frameEnd=avifileinfo->nb_frames-1;
      		frameStart=0;
      		GUI_setAllFrameAndTime ();
      		UI_setMarkers (frameStart, frameEnd);
      		ReSync ();

		// forget last project file
		if( actual_workbench_file ){
			ADM_dealloc(actual_workbench_file);
			actual_workbench_file = NULL;
		}
	}
	break;

    case ACT_Delete:
    case ACT_Cut:

#if 0
      old=frameStart;
      if( A_delete(frameStart,frameEnd))
      {
      	if (old >= avifileinfo->nb_frames)
	{			// we removed too much
	  old = avifileinfo->nb_frames - 1;
	}
      	curframe=old;
      	GUI_GoToFrame (old);
      }

#endif
      break;

    case ACT_AudioMap:
      printf ("\n making audio timeline\n");
      if (currentaudiostream)
	{

//	  currentaudiostream->buildAudioTimeLine ();

	}
      break;

    case ACT_ChangeFPS:
    	{
         float  fps;
         aviInfo info;
         uint32_t useDefined=1;
         uint32_t defaultFps[3]={25000,23976,29970};
         uint32_t index=0;
         video_body->getVideoInfo (&info);
         fps=info.fps1000;
         fps/=1000.;



        diaElemToggle togUsePredefined(&useDefined,QT_TR_NOOP("Use custom value"));
        diaElemFloat  fpsFloatValue(&fps,QT_TR_NOOP("Frame Rate"),1.,200.,QT_TR_NOOP("_Frames per second"));

        diaMenuEntry menuFps[]={
              {0,QT_TR_NOOP("PAL - 25 FPS")},
              {1,QT_TR_NOOP("FILM- 24 FPS")},
              {2,QT_TR_NOOP("NTSC- 30 FPS")}};

         diaElemMenu      stdFps(&index,QT_TR_NOOP("Standard FrameRate:"),3,menuFps);

        togUsePredefined.link(1,&fpsFloatValue);
        togUsePredefined.link(0,&stdFps);

        diaElem *elems[3]={&togUsePredefined,&fpsFloatValue,&stdFps};
        if(diaFactoryRun(QT_TR_NOOP("Change FrameRate"),3,elems))
        {
          if(useDefined)
          {
            info.fps1000 = (uint32_t) (floor (fps * 1000.+0.49));
          }else
          {
            info.fps1000=defaultFps[index];
          }
          video_body->updateVideoInfo (&info);
          printf("[MainUI] New framerate :%u\n",info.fps1000);
          // update display
          video_body->getVideoInfo (avifileinfo);
          GUI_setAllFrameAndTime();

        }
	}
      break;
      // set decoder option (post processing ...)
    case ACT_DecoderOption:
      video_body->setDecodeParam ( video_body->getCurrentFrame());

      break;
    case ACT_VideoParameter:
      // first remove current viewer
      if (getPreviewMode()!=ADM_PREVIEW_NONE)
        {
	         admPreview::stop();
        }
      GUI_handleVFilter();
      if( getLastVideoFilter()->getInfo()->width % 8 ){
        GUI_Error_HIG(QT_TR_NOOP("Width is not a multiple of 8"),
                      QT_TR_NOOP("This will make trouble for AVI files."));
      }
      if (getPreviewMode()!=ADM_PREVIEW_NONE)
      {
         admPreview::start();
//         admPreview::update (curframe);
      }
      break;

    case ACT_RebuildKF:
      if (GUI_Question (QT_TR_NOOP("Rebuild all Keyframes?")))
	{
	  A_rebuildKeyFrame ();
	  //GUI_Info_HIG ("Done", "Save your file and restart Avidemux.");
	}
      break;

   case ACT_HEX_DUMP:
      GUI_showCurrentFrameHex();
      break;

    default:
      printf ("\n unhandled action %d\n", action);
      ADM_assert (0);
      return;

    }


  // gtk_widget_grab_focus(sb_frame);
}



//_____________________________________________________________
//
// Open AVI File
//    mode 0: normal
//    mode 1: Suspicious
//_____________________________________________________________

void
A_openBrokenAvi (const char *name)
{
  A_openAvi2 (name, 1);
}

int
A_openAvi (const char *name)
{
  return A_openAvi2 (name, 0);
}
extern void GUI_PreviewEnd (void);
int A_openAvi2 (const char *name, uint8_t mode)
{
  uint8_t res;
  char *longname;
  uint32_t magic[4];
  uint32_t id = 0;

  if (playing)
    return 0;
  /// check if name exists
  FILE *fd;
  fd = fopen (name, "rb");
  if (!fd){
    if( errno == EACCES ){
      GUI_Error_HIG(QT_TR_NOOP("Permission error"), QT_TR_NOOP("Cannot open \"%s\"."), name);
    }
    if( errno == ENOENT ){
      GUI_Error_HIG(QT_TR_NOOP("File error"), QT_TR_NOOP("\"%s\" does not exist."), name);
    }
    return 0;
  }
  if( 4 == fread(magic,4,4,fd) )
     id=R32(magic[0]);
  fclose (fd);


  GUI_close(); // Cleanup

  DIA_StartBusy ();
  /*
  ** we may get a relative path by cmdline
  */
  longname = ADM_PathCanonize(name);
  if (mode)
    res = video_body->addFile (longname, 1);
  else
  {
    res = video_body->addFile (longname);
  }
  DIA_StopBusy ();

  // forget last project file
  if( actual_workbench_file ){
     ADM_dealloc(actual_workbench_file);
     actual_workbench_file = NULL;
  }

  if (res!=ADM_OK)			// an error occured
    {
		delete[] longname;
    	if(ADM_IGN==res)
	{
		return 0;
	}

	if( fourCC::check(id,(uint8_t *)"//AD") ){
          GUI_Error_HIG(QT_TR_NOOP("Cannot open project using the video loader."),
                        QT_TR_NOOP(  "Try 'File' -> 'Load/Run Project...'"));
	}else{
          GUI_Error_HIG (QT_TR_NOOP("Could not open the file"), NULL);
	}
	return 0;
    }

    { int i;
      FILE *fd=NULL;
      char magic[4];

	/* check myself it is a project file (transparent detected and read
        ** by video_body->addFile (name);
	*/
	if( (fd = fopen(longname,"rb"))  ){
		if( fread(magic,4,1,fd) == 4 ){
			/* remember a workbench file */
			if( !strncmp(magic,"ADMW",4) ){
				actual_workbench_file = ADM_strdup(longname);
			}
		}
		fclose(fd);
	}

	/* remember any video or workbench file to "recent" */
	prefs->set_lastfile(longname);
        UI_updateRecentMenu();
	updateLoaded ();
        if(currentaudiostream)
        {
            uint32_t nbAudio;
            audioInfo *infos=NULL;
            if(video_body->getAudioStreamsInfo(video_body->getCurrentFrame()+1,&nbAudio,&infos))
            {
                if(nbAudio>1)
                {   // Multiple track warn user
                  GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Multiple Audio Tracks"),QT_TR_NOOP("The file you just loaded contains several audio tracks.\n"
                      "Go to Audio->MainTrack to select the active one."));
                }
            }
            if(infos) delete [] infos;
            // Revert mixer to copy
            setCurrentMixerFromString("NONE");
        }
	for(i=strlen(longname);i>=0;i--)
#ifdef __WIN32
		if( longname[i] == '\\' || longname[i] == '/' ){
#else
		if( longname[i] == '/' ){
#endif
			i++;
			break;
		}
	UI_setTitle(longname+i);
    }

	delete[] longname;
	return 1;
}
/**
    \fn updateLoaded

*/
void  updateLoaded ()
{
  avifileinfo = new aviInfo;
  if (!video_body->getVideoInfo (avifileinfo))
    {
//      err1:
      printf ("\n get info failed...cancelling load...\n");
      delete avifileinfo;
      avifileinfo = NULL;

      return;
    }


  getFirstVideoFilter(); // reinit first filter

  // now get audio information if exists
  wavinfo = video_body->getInfo ();	//wavinfo); // will be null if no audio
  if (!wavinfo)
    {
      printf ("\n *** NO AUDIO ***\n");
      wavinfo = (WAVHeader *) NULL;
    }
  else
    {
	  video_body->getAudioStream (&aviaudiostream);
      A_changeAudioStream (aviaudiostream, AudioAvi,NULL);
#if 0
      if (aviaudiostream)
	if (!aviaudiostream->isDecompressable ())
	  {
            GUI_Error_HIG (QT_TR_NOOP("No audio decoder found for this file"),
                           QT_TR_NOOP( "Save (A+V) will generate bad AVI. Save audio will work."));
	  }
#endif
    }

  // Init renderer
    admPreview::setMainDimension(avifileinfo->width, avifileinfo->height);


  // Draw first frame
  GUI_setAllFrameAndTime();

  video_body->getMarkers(&frameStart,&frameEnd);
  UI_setMarkers (frameStart, frameEnd);

  getFirstVideoFilter(); // Rebuild filter if needed

  /* Zoom out if needed */
  uint32_t phyW,phyH;
  UI_getPhysicalScreenSize(NULL, &phyW,&phyH);
  if(phyW<avifileinfo->width || phyH<avifileinfo->height)
  {
      if(phyW<avifileinfo->width/2 || phyH<avifileinfo->height/2)
      {
                currentZoom=ZOOM_1_4;
      }else
      {
                currentZoom=ZOOM_1_2;
      }
     changePreviewZoom(currentZoom);
  }
  else
  {
      currentZoom=ZOOM_1_1;
      changePreviewZoom(currentZoom);
  }



      admPreview::seekToIntra (0);
      admPreview::samePicture();
      GUI_setCurrentFrameAndTime();

   printf("\n** conf updated **\n");
}

//___________________________________________
//  Append an AVI to the existing one
//___________________________________________
int
A_appendAvi (const char *name)
{


  if (playing)
    return 0;
  DIA_StartBusy ();
  if (!video_body->addFile (name))
    {
      DIA_StopBusy ();
      GUI_Error_HIG (QT_TR_NOOP("Something failed when appending"), NULL);
      return 0;
    }
  DIA_StopBusy ();


  video_body->dumpSeg ();
  if (!video_body->updateVideoInfo (avifileinfo))
    {
      GUI_Error_HIG (QT_TR_NOOP("Something bad happened (II)"), NULL);
      return 0;
    }

  ReSync ();
  UI_setMarkers (frameStart, frameEnd);
  return 1;
}

//
//      Whenever a changed happened in the the stream, resync
//  related infos including audio & video filters

void ReSync (void)
{
  uint8_t isaviaud;

  frameStart = 0;
  frameEnd = avifileinfo->nb_frames - 1;
  // update audio stream
  // If we were on avi , mark it...
  if (currentaudiostream == aviaudiostream)
    {
      isaviaud = 1;
//      A_changeAudioStream ((AVDMGenericAudioStream *) NULL, AudioNone,NULL);

    }
  else
    isaviaud = 0;
  GUI_setAllFrameAndTime ();

  // Since we modified avi stream, rebuild audio stream accordingly
  video_body->getAudioStream (&aviaudiostream);
  if (isaviaud)
    {
      A_changeAudioStream (aviaudiostream, AudioAvi,NULL);
    }
  	//updateVideoFilters ();
	getFirstVideoFilter();

}

//_____________________________________________________________
//    Save current stream (generally avi...)
//     in raw mode
//_____________________________________________________________
void
A_saveAudio (char *name)
{

// debug audio seek
  uint32_t len2;
  uint32_t written, max;
  uint64_t dts;
  DIA_workingBase *work;
  FILE *out;

#define ONE_STRIKE (64*1024)
  uint8_t *buffer=NULL;

  if (!currentaudiostream)
    return;



  out = fopen (name, "wb");
  if (!out) return;

  work=createWorking(QT_TR_NOOP("Saving audio"));

  uint32_t timeEnd,timeStart,sample,hold,len;
  uint64_t tgt_sample,cur_sample;
  double   duration;

  // compute start position and duration in samples

   timeStart=video_body->getTime (frameStart);
   timeEnd=video_body->getTime (frameEnd+1);

   currentaudiostream->goToTime (timeStart);
   duration=timeEnd-timeStart;
   printf("Duration:%f ms\n",duration);
   if(duration<0) duration=-duration;

   duration/=1000;
   duration*=currentaudiostream->getInfo()->frequency;

   tgt_sample=(uint64_t)floor(duration);

   cur_sample=0;
   written = 0;
   hold=0;
   buffer=new uint8_t[ONE_STRIKE*2];
   while (1)
    {
    	if(!currentaudiostream->getPacket(buffer+hold,&len,64*1024,&sample,&dts)) break;
	hold+=len;
	written+=len;
	cur_sample+=sample;
	if(hold>ONE_STRIKE)
	{
		fwrite(buffer,hold,1,out);
		hold=0;
	}
	if(cur_sample>tgt_sample)
		break;
      work->update(cur_sample>>10, tgt_sample>>10);
      if(!work->isAlive()) break;
    };
  if(hold)
  {
  	fwrite(buffer,hold,1,out);
	hold=0;
  }

  fclose (out);
  delete work;
  delete[] buffer;
  printf ("\n wanted %"LLU" samples, goto %"LLU" samples, written %"LU" bytes\n", tgt_sample,cur_sample, written);


}



//____________________________________________________________
//
//      Play AVI  + Audio if possible
//_____________________________________________________________
void
A_playAvi (void)
{

}

#ifndef TEST_MP2
/**________________________________________________________
 Save a Jpg image from current display buffer
________________________________________________________*/
int A_saveJpg (char *name)
{
  uint8_t fl;
    ADMImage image(avifileinfo->width, avifileinfo->height);
    if(!GUI_getFrameContent(&image, curframe))
    {
      GUI_Error_HIG(QT_TR_NOOP("Get Frame"),QT_TR_NOOP("Cannot get this frame to save"));
      return 0;
    }
    return (int) image.saveAsJpg (name);
}
#else
/**
      \fn A_saveJpg
      \brief Save current image as jpeg 95% qual

*/
int A_saveJpg (char *name)
{
static int b=1;
         video_body->changeAudioStream(0,b);
        b^=1;
        return 1;

}
#endif


/**
      \fn A_saveBunchJpg
      \brief Save the selection  as a bunch of jpeg 95% qual

*/
void A_saveBunchJpg(const char *name)
{
  ADMImage *src=NULL;
  uint32_t curImg;
  char	 fullName[2048],*ext;
  char *baseName;
  DIA_workingBase *working;
  uint8_t success=0;

        if(frameStart>frameEnd)
                {
                  GUI_Error_HIG(QT_TR_NOOP("Mark A > B"), QT_TR_NOOP("Set your markers correctly."));
                        return;
                }
        // Split name into base + extension
        ADM_PathSplit(name,&baseName,&ext);

        src=new ADMImage(avifileinfo->width,avifileinfo->height);
        ADM_assert(src);

        working=createWorking(QT_TR_NOOP("Saving as set of jpegs"));
        for(curImg=frameStart;curImg<=frameEnd;curImg++)
        {
                working->update(curImg-frameStart,frameEnd-frameStart);
                if (!GUI_getFrameContent (src,curImg ))
                {
                  GUI_Error_HIG(QT_TR_NOOP("Cannot decode frame"), QT_TR_NOOP("Aborting."));
                        goto _bunch_abort;
                }
                if(!working->isAlive()) goto _bunch_abort;
                sprintf(fullName,"%s%04d.jpg",baseName,curImg-frameStart);
                if(!src->saveAsJpg(fullName)) goto _bunch_abort;
        }
        success=1;

_bunch_abort:
        if(success)
            GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Done"),QT_TR_NOOP( "Saved %d images."), curImg-frameStart);
        else
            GUI_Error_HIG(QT_TR_NOOP("Error"),QT_TR_NOOP( "Could not save all images."));
        delete working	;
        delete src;
        return ;


}
/**
      \fn A_saveImg
      \brief Save current displayed image as a BMP file
*/
void A_saveImg (const char *name)
{

  ADMImage image(avifileinfo->width,avifileinfo->height);
  GUI_getFrameContent(&image, video_body->getCurrentFrame());
  if(image.saveAsBmp(name))
        GUI_Info_HIG (ADM_LOG_INFO,QT_TR_NOOP("Done"),QT_TR_NOOP( "Saved \"%s\"."), ADM_GetFileName(name));
  else
        GUI_Error_HIG (QT_TR_NOOP("BMP op failed"),QT_TR_NOOP( "Saving %s as a BMP file failed."), ADM_GetFileName(name));
}

//_____________________________________________________________
//
//              Load AC3
//
//
//_____________________________________________________________
int
A_loadAC3 (char *name)
{

}
int A_loadNone( void )
{
// 	A_changeAudioStream ((AVDMGenericAudioStream *) NULL, AudioNone,NULL);
}

//_____________________________________________________________
//
//              Load MP3 and identify wavfmt infos to fill avi header
//              -> use mad ?
//
//_____________________________________________________________
int A_loadMP3(char *name)
{
  }
//_____________________________________________________________
//
//              Load wave
//
//
//_____________________________________________________________

int A_loadWave (char *name)
{

}
AudioSource getCurrentAudioSource(char **name)
{
        *name=currentAudioName;
        return currentAudioSource;
}
//________________________________________________________
// Change audio stream and delete the old one if needed
//________________________________________________________
uint8_t A_changeAudioStream (ADM_audioStream * newaudio, AudioSource nwsource,char *myname)
{
	if (currentaudiostream)
	{
		if (currentaudiostream != aviaudiostream)
			delete currentaudiostream;

		if(currentAudioName)
			ADM_dealloc(currentAudioName);

		currentAudioName = NULL;
		currentAudioSource = AudioNone;
	}

  currentaudiostream = newaudio;

  if (currentaudiostream)
  {
    wavinfo = currentaudiostream->getInfo ();
    currentAudioSource=nwsource;
    if(myname)
        currentAudioName=ADM_strdup(myname);
  }
  return 1;
}

//_____________________________________________________________
//    Save current stream (generally avi...)
//     in decoded mode (assuming MP3)
//_____________________________________________________________
void
A_saveAudioDecodedTest (char *name)
{
#if 0
// debug audio seek
  uint32_t len, gauge = 0;
  uint32_t written = 0;
  FILE *out;
  AVDMGenericAudioStream *saveFilter;

  uint64_t sampleTarget,sampleCurrent;

#undef BITT
#define BITT 4*1152
#define OUTCHUNK 1024*1024
  uint8_t *outbuffer;

  if (!currentaudiostream)
    return;


  if (!(out = fopen (name, "wb")))
    {
      GUI_Error_HIG (QT_TR_NOOP("File error"), QT_TR_NOOP("Cannot open \"%s\" for writing."), name);
      return;
    }

  outbuffer = (uint8_t *) ADM_alloc (2 * OUTCHUNK);	// 1Meg cache;
  if (!outbuffer)
    {
      GUI_Error_HIG (QT_TR_NOOP("Memory Error"), NULL);
      return;
    }



// re-ignite first filter...




  // Write Wav header

  /* Sat Nov 09 06:11:52 CET 2002 Fixes from Maik Broemme <mbroemme@plusserver.de> */
  /* If you set negative delay and save the audio stream, the saved stream was shorter than the video stream. */

  /* Example: video stream is 10 minutes long, audio stream perhaps 20 minutes, you need the audio stream from */
  /*          minute 1 until 11, so you setup an audio delay from -60 seconds, but this 60 seconds were removed */
  /*          from begin and end of the audio stream. That was not good :) Now it runs correctly also if you use */
  /*          audio stream with same length then video, therefore is premature ending :) */



//        saveFilter =  buildAudioFilter (currentaudiostream,video_body->getTime (frameStart));

		if (saveFilter == NULL)
		{
			fclose(out);
			ADM_dealloc(outbuffer);
			return;
		}

    	DIA_working *work=new DIA_working(QT_TR_NOOP("Saving audio"));


//
//  Create First filter that is null filter
//
  saveFilter->writeHeader (out);
  uint32_t tstart,tend,samples;
  double duration;
  tstart=video_body->getTime(frameStart);
  tend=video_body->getTime(frameEnd+1);
  duration=(tend-tstart);
  duration*=saveFilter->getInfo()->frequency;
  duration/=1000.;

  sampleTarget=(uint64_t)floor(duration);
  sampleCurrent=0;
  gauge=0;

  if( frameStart == frameEnd ){
     /* JSC: we will write some bytes, but nobody should expect useful data */
    GUI_Error_HIG(QT_TR_NOOP("No frames to encode"),QT_TR_NOOP("Please check markers. Is \"A>\" == \">B\"?"));
  }

  while ((sampleCurrent<sampleTarget))
    {
      if(!saveFilter->getPacket(outbuffer + gauge,&len,&samples))
      {
        printf("Audio save:Read error\n");
      	break;
      }
      //      printf("Got : %lu\n",len2);
      gauge += len;
      sampleCurrent+=samples;
      // update GUI
	// JSC: if "A>" == ">B" we will get >100% here => assert in work->update()
	if (work->update ((sampleCurrent>>10 > sampleTarget>>10 ? sampleTarget>>10 : sampleCurrent>>10), sampleTarget>>10))	// abort request ?
	    break;;
      if (gauge > OUTCHUNK)	// either out buffer is full
	{
	  fwrite (outbuffer, 1, gauge, out);
	  written += gauge;
	  gauge = 0;
	}
    };
// Clean up
	if(gauge)
	{
		fwrite (outbuffer,  gauge,1, out);
		written += gauge;
		gauge = 0;
	}
  saveFilter->endWrite (out, written);
  fclose (out);
  ADM_dealloc (outbuffer);
  delete work;
//  deleteAudioFilter (saveFilter);
//  currentaudiostream->endDecompress ();
  printf ("AudioSave: actually written %u\n", written);
  printf ("Audiosave: target sample:%llu, got :%llu\n",sampleTarget,sampleCurrent);

#endif
}




//      Clean up
//      free all pending stuff, make leakchecker happy
//
void cleanUp (void)
{
	if (avifileinfo)
	{
		delete avifileinfo;
		avifileinfo=NULL;
	}

	if (aviaudiostream)
	{
		delete aviaudiostream;
		aviaudiostream=NULL;
	}

	if (secondaudiostream)
	{
		delete secondaudiostream;
		secondaudiostream=NULL;
	}

	if (currentAudioName)
	{
		ADM_dealloc(currentAudioName);
		currentAudioName = NULL;
	}

	if (secondAudioName)
	{
		ADM_dealloc(secondAudioName);
		secondAudioName = NULL;
	}

	if (actual_workbench_file)
	{
		ADM_dealloc(actual_workbench_file);
		actual_workbench_file = NULL;
	}

	if (video_body)
	{
		delete video_body;
		video_body=NULL;
	}

	currentaudiostream=NULL;
	filterCleanUp();
	admPreview::cleanUp();
}

#warning fixme


extern void getCutPoints (int br);
void
ADM_cutWizard (void)
{
#if 0
  int br;
  if (currentaudiostream)
    {
      br = currentaudiostream->getInfo ()->byterate * 8 / 1000;
    }
  else
    br = 128;

  getCutPoints (br);
#endif
}


/*
	Save a raw video stream without any container
	Usefull to cut mpeg stream or extract raw h263/mpeg4 stream

*/
uint8_t ADM_saveRaw (const char *name)
{
  uint32_t len, flags;
  FILE *fd, *fi;
  uint8_t *buffer = new uint8_t[avifileinfo->width * avifileinfo->height * 3],ret=0;
  char *idx;
  DIA_workingBase *work;
  uint8_t seq;
  idx = new char[strlen (name) + 8];
  strcpy (idx, name);
  strcat (idx, ".idx");
  fd = fopen (name, "wb");
  fi = fopen (idx, "wt");
  if (!fd)
    return 0;
  work=createWorking(QT_TR_NOOP("Saving raw video stream"));
  ADMCompressedImage image;
  image.data=buffer;
  image.dataLength=avifileinfo->width * avifileinfo->height * 3;
  // preamble
#if 0
  video_body->getRawStart (frameStart, buffer, &len);
  fwrite (buffer, len, 1, fd);
#endif
  for (uint32_t i = frameStart; i < frameEnd; i++)
    {
      work->update (i - frameStart, frameEnd - frameStart);
      if(!work->isAlive())
      {
                 ret=0;
                 goto _abt;
      }
      if(!video_body->getFlags (i, &flags))
        {
                if(i==frameEnd-1)
                {
                         ret=1;
                         goto _abt;
                }
                ADM_assert (video_body->getFlags (i, &flags));
        }

      if (flags & AVI_B_FRAME)	// oops
	{
	  // se search for the next i /p
	  uint32_t found = 0;

	  for (uint32_t j = i + 1; j < frameEnd; j++)
	    {
	      ADM_assert (video_body->getFlags (j, &flags));
	      if (!(flags & AVI_B_FRAME))
		{
		  found = j;
		  break;
		}

	    }
	  if (!found)
          {
            if(abs(i-frameEnd)>2)
                ret=0;
            else
                ret=1;  // Good enough
	    goto _abt;
          }
	  // Write the found frame

	  video_body->getFrame (found, &image, &seq);
	  fwrite (buffer, len, 1, fd);
	  // and the B frames
	  for (uint32_t j = i; j < found; j++)
	    {
	      video_body->getFrame (j, &image,&seq);
	      fwrite (buffer, len, 1, fd);
	    }
	  i = found;		// Will be plussed by for
	}
      else			// P or I frame
	{
	  video_body->getFrame (i, &image, &seq);
	  fwrite (buffer, len, 1, fd);
	  fprintf (fi, "%u,\n", len);
	}

    }
    ret=1;
_abt:
  fclose (fd);
  fclose (fi);
  delete work;
  return ret;

}

void
A_saveWorkbench (const char *name)
{
#if 0
  video_body->saveWorbench (name);
#else
  video_body->saveAsScript(name,NULL);
#endif
  if( actual_workbench_file )
     ADM_dealloc(actual_workbench_file);
  actual_workbench_file = ADM_strdup(name);
}

void A_parseECMAScript(const char *name){
  bool ret;
  char *longname = ADM_PathCanonize(name);
   if (playing){
      GUI_PlayAvi();
      curframe = 0;
   }
   ret = parseECMAScript(longname);
   if( ret == 0 ){
      if( actual_workbench_file )
         ADM_dealloc(actual_workbench_file);
      actual_workbench_file = ADM_strdup(longname);
   }
   ADM_dealloc(longname);
}
/**---------------------------------------------------
		Pipe to toolame
-------------------------------------------------------**/
extern  void audioCodecSetcodec(AUDIOENCODER codec);

/**
	Return the # of byte to go from start frame to end frame

*/
uint32_t getAudioByteCount( uint32_t start, uint32_t end)
{
uint32_t max=0;
// compute max bytes to read (with little margin)
  max = video_body->getTime (end + 1) - video_body->getTime (start);
  //convert time in ms to bytes

  double db;
  db = max;
  // 01/10/2002 Fix for 5.1->stereo conversion
  if (wavinfo->channels > 2)
    db *= 2;
  else
    db *= wavinfo->channels;
  db *= wavinfo->frequency;
  db /= 500;			// 1000 because ms, 2 bytes / sample
  max = (uint32_t) floor (db);

  return max;
}
/*
	Unpack all frames without displaying them to check for error

*/
void A_videoCheck( void)
{
uint32_t nb=0;
//uint32_t buf[720*576*2];
uint32_t error=0;
ADMImage *aImage;
DIA_workingBase *work;

	nb = avifileinfo->nb_frames;
	work=createWorking(QT_TR_NOOP("Checking video"));
	aImage=new ADMImage(avifileinfo->width,avifileinfo->height);
  for(uint32_t i=0;i<nb;i++)
  {
	work->update(i, nb);
      	if(!work->isAlive()) break;
	if(!GUI_getFrameContent (aImage,i))
	{
		error ++;
		printf("Frame %u has error\n",i);
	}

    };
  delete work;
  delete aImage;
  if(error==0)
    GUI_Info_HIG(ADM_LOG_IMPORTANT,QT_TR_NOOP("No error found"), NULL);
else
	{
		char str[400];
                sprintf(str,QT_TR_NOOP("Errors found in %u frames"),error);
		GUI_Info_HIG(ADM_LOG_IMPORTANT,str, NULL);

	}
	GUI_GoToFrame(curframe);

}
//___________ save audio _____________
int A_audioSave(char *name)
{
	if (!currentaudiostream)	// yes it is checked 2 times so what ?
	return 0;
	if (audioProcessMode())
	{

		// if we get here, either not compressed
		// or decompressable
		A_saveAudioDecodedTest(name);
	    }
	else			// copy mode...
	    {
	       A_saveAudio(name);
	    }
	return 1;
}
uint8_t A_pass(char *name)
{
//        mpeg_passthrough(name,ADM_PS);
        return 1;
}
int A_delete(uint32_t start, uint32_t end)
{
uint32_t count;

      aviInfo info;
      ADM_assert (video_body->getVideoInfo (&info));
      count = end - start;

      if( end < start ){
        GUI_Error_HIG(QT_TR_NOOP("Marker A > B"), QT_TR_NOOP("Cannot delete the selection."));
         return 0;
      }
      if (count >= info.nb_frames - 1)
	{
          GUI_Error_HIG (QT_TR_NOOP("You can't remove all frames"), NULL);
	  return 0;
	}

      video_body->dumpSeg ();
      if (!video_body->removeFrames (start, end))
	{
          GUI_Error_HIG (QT_TR_NOOP("Something bad happened"), NULL);
	  return 0;
	}
      video_body->dumpSeg ();
      //resync GUI and video
      if (!video_body->updateVideoInfo (avifileinfo))
	{
          GUI_Error_HIG (QT_TR_NOOP("Something bad happened (II)"), NULL);
	}



      frameEnd=avifileinfo->nb_frames-1;
      frameStart=0;
      GUI_setAllFrameAndTime ();
      UI_setMarkers (frameStart, frameEnd);
      ReSync ();
     return 1;



}
extern int DIA_getMPParams( uint32_t *pplevel, uint32_t *ppstrength,uint32_t *swap);
//

//
void	A_setPostproc( void )
{
uint32_t type,strength,swap;
	if(!avifileinfo) return;

	video_body->getPostProc(&type,&strength,&swap);

 	if(DIA_getMPParams( &type, &strength,&swap))
 	{
		video_body->setPostProc(type,strength,swap);
 	}

}
extern uint8_t DIA_audioTrack(AudioSource *source, uint32_t *track,uint32_t nbTrack, audioInfo *infos);
extern const char *getStrFromAudioCodec( uint32_t codec);
/**
      \fn A_audioTrack
      \brief Allow to select audio track
*/
void A_audioTrack( void )
{
        uint32_t nb;
        audioInfo *infos=NULL;

        uint32_t old,nw;
        uint8_t r=0;
        uint32_t oldtrack,newtrack;
        char *newtrackname=ADM_strdup(currentAudioName);

        if(!video_body->getAudioStreamsInfo(0,&nb, &infos))
        {
          GUI_Error_HIG(QT_TR_NOOP("Could not get tracks info"), NULL);
                return ;
        }
        newtrack=oldtrack=(uint32_t)video_body->getCurrentAudioStreamNumber(0);
        nw=old=currentAudioSource;

        /* Build dialog factory widget */
        diaMenuEntry sourcesStream[]={
            {AudioAvi,QT_TR_NOOP("Video"),QT_TR_NOOP("Take audio from video file")},
            {AudioNone,QT_TR_NOOP("None"),QT_TR_NOOP("No audio")},
            {AudioAC3,QT_TR_NOOP("External AC3"),QT_TR_NOOP("Take audio from external AC3 file")},
            {AudioMP3,QT_TR_NOOP("External MP3"),QT_TR_NOOP("Take audio from external MP3 file")},
            {AudioWav,QT_TR_NOOP("External WAV"),QT_TR_NOOP("Take audio from external WAV file")}
        };


        diaElemMenu   sourceMenu(&nw,QT_TR_NOOP("_Audio source:"),5,sourcesStream,NULL);



        diaElemFile  sourceName(0,&newtrackname,QT_TR_NOOP("_External file:"), NULL, QT_TR_NOOP("Select file"));

        // Now build the list of embedded track
#define MAX_AUDIO_TRACK 10
#define MAX_AUDIO_TRACK_NAME 100
        diaMenuEntryDynamic *sourceavitracks[MAX_AUDIO_TRACK];
        char string[MAX_AUDIO_TRACK_NAME];
        for(int i=0;i<nb;i++)
        {
          sprintf(string,"Audio track %d (%s/%d channels/%d kbit per s/%d ms shift)",i,getStrFromAudioCodec(infos[i].encoding),
                        infos[i].channels,infos[i].bitrate,infos[i].av_sync);
           sourceavitracks[i]=new diaMenuEntryDynamic(i,string,NULL);
        }
         if(infos) delete [] infos;

         diaElemMenuDynamic   sourceFromVideo(&newtrack,QT_TR_NOOP("_Track from video:"),nb,sourceavitracks);
         diaElem *allWidgets[]={&sourceMenu,&sourceFromVideo,&sourceName};

         /* Link..*/
         sourceMenu.link(&(sourcesStream[0]),1,&sourceFromVideo);
         sourceMenu.link(&(sourcesStream[2]),1,&sourceName);
         sourceMenu.link(&(sourcesStream[3]),1,&sourceName);
         sourceMenu.link(&(sourcesStream[4]),1,&sourceName);


         if( diaFactoryRun(QT_TR_NOOP("Main Audio Track"),3,allWidgets))
         {
           if(nw!=AudioNone && nw!=AudioAvi)
           {
              if( !ADM_fileExist(newtrackname))
              {
                GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Cannot load"),QT_TR_NOOP("The selected audio file does not exist."));
                goto roger_and_out;
              }
           }
            switch( nw)
            {
                    case AudioMP3:
                            if( ADM_fileExist(newtrackname))
                              A_loadMP3(newtrackname);
                            break;
                    case AudioAC3:
                            if( ADM_fileExist(newtrackname))
                              A_loadAC3(newtrackname);
                            break;
                    case AudioWav:
                            if( ADM_fileExist(newtrackname))
                              A_loadWave(newtrackname);
                            break;
                    case AudioNone:
//                              A_changeAudioStream((AVDMGenericAudioStream *) NULL, AudioNone,NULL);
                            break;
                    case AudioAvi:
                            //printf("New :%d old:%d\n",newtrack,oldtrack);
                            if(oldtrack!=newtrack)
                            {
                                    video_body->changeAudioStream(0,newtrack);
                                    //
                                    if(aviaudiostream==currentaudiostream)
                                            currentaudiostream=NULL;
                                    delete aviaudiostream;
                                    aviaudiostream=NULL;
                                    video_body->getAudioStream(&aviaudiostream);
                            }
                            A_changeAudioStream (aviaudiostream, AudioAvi,NULL);
                            break;
                    default:
                            ADM_assert(0);
        }
         }
roger_and_out:
         /* Clean up */
         for(int i=0;i<nb;i++)
            delete sourceavitracks[i];
        return;

}
/**
        \fn A_externalAudioTrack
        \brief Select external audio track (for 2nd track)
*/
void A_externalAudioTrack( void )
{
        uint32_t old,nw;
        uint32_t oldtrack,newtrack;
        char  *newtrackname=ADM_strdup(secondAudioName);

   diaMenuEntry sourcesStream[]={
            {AudioNone,QT_TR_NOOP("None"),QT_TR_NOOP("No audio")},
            {AudioAC3,QT_TR_NOOP("External AC3"),QT_TR_NOOP("Take audio from external AC3 file")},
            {AudioMP3,QT_TR_NOOP("External MP3"),QT_TR_NOOP("Take audio from external MP3 file")},
            {AudioWav,QT_TR_NOOP("External WAV"),QT_TR_NOOP("Take audio from external WAV file")}
        };

        old=nw=secondAudioSource;

        diaElemMenu     sourceMenu(&nw,QT_TR_NOOP("_Audio source:"),4,sourcesStream,NULL);
        diaElemFile     sourceName(0,&newtrackname,QT_TR_NOOP("_External file:"), NULL, QT_TR_NOOP("Select file"));
        diaElem *allWidgets[]={&sourceMenu,&sourceName};

  /* Link..*/

         sourceMenu.link(&(sourcesStream[2]),1,&sourceName);
         sourceMenu.link(&(sourcesStream[3]),1,&sourceName);
         sourceMenu.link(&(sourcesStream[1]),1,&sourceName);

         if( !diaFactoryRun(QT_TR_NOOP("Second Audio Track"),2,allWidgets)) return;
         if(!ADM_fileExist(newtrackname))
         {
           GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Cannot load"),QT_TR_NOOP("The selected audio file does not exist."));
           return;
         }
        if(secondAudioSource!=AudioNone)
        {
                 delete secondaudiostream;
                 secondAudioSource=AudioNone;
                 secondaudiostream=NULL;
                 if(secondAudioName) ADM_dealloc(secondAudioName);
                 secondAudioName=NULL;
        }
       secondAudioSource=(AudioSource)nw;
        A_setSecondAudioTrack(secondAudioSource,newtrackname);
        if(newtrackname) ADM_dealloc(newtrackname);
}

uint8_t A_setSecondAudioTrack(const AudioSource nw,char *name)
{
#if 0
        switch(nw)
        {
                case AudioNone:break;
                case AudioMP3:
                        {
                        AVDMMP3AudioStream *tmp;
                        if(!name) break;
                        tmp = new AVDMMP3AudioStream ();
                        if (!tmp->open (name))
                        {
                                delete tmp;
                                GUI_Error_HIG(QT_TR_NOOP("Error loading the MP3 file"), NULL);

                        }
                        else
                        {
/*                                secondaudiostream = tmp;
                                secondAudioSource=AudioMP3;
                                secondAudioName=ADM_strdup(name);
                                printf ("\n MP3 loaded\n");
                                GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Second track loaded"), NULL);
*/
                                return 1;
                        }
                        }
                        break;
                case AudioAC3:
                          {
                        AVDMAC3AudioStream *tmp;
                        if(!name) break;

                        tmp = new AVDMAC3AudioStream ();
                        if (!tmp->open (name))
                        {
                                delete tmp;
                                GUI_Error_HIG(QT_TR_NOOP("Error loading the AC3 file"), NULL);
                        }
                        else
                        {
/*
                                secondaudiostream = tmp;
                                secondAudioSource=AudioAC3;
                                secondAudioName=ADM_strdup(name);
                                printf ("\n AC3 loaded\n");
                                GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Second track loaded"), NULL);
*/
                                return 1;
                        }
                        }
                        break;
                case AudioWav:
                         {
                        AVDMWavAudioStream *tmp;
                        if(!name) break;

                        tmp = new AVDMWavAudioStream ();
                        if (!tmp->open (name))
                        {
                                delete tmp;
                                GUI_Error_HIG(QT_TR_NOOP("Error loading the WAV file"), NULL);
                        }
                        else
                        {
/*
                                secondaudiostream = tmp;
                                secondAudioSource=AudioAC3;
                                secondAudioName=ADM_strdup(name);
                                printf ("\n AC3 loaded\n");
                                GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Second track loaded"), NULL);
*/
                                return 1;
                        }}
                        break;
                default:
                ADM_assert(0);
        }
        return 0;
#endif
}


void A_Resync(void)
{
// Just in case update file info
        if(!avifileinfo) return;
        GUI_setAllFrameAndTime();

        if(curframe>avifileinfo->nb_frames) curframe=frameEnd;
        UI_setMarkers (frameStart, frameEnd);
        GUI_GoToFrame(curframe);
}
uint8_t  DIA_job_select(char **jobname, char **filename);
void A_addJob(void)
{
        char *name=NULL,*fullname,*base,*final=NULL;

        if(!DIA_job_select(&name,&final)) return;
        if(!name || !final) return;
        if(!*name || !*final) return;

        base=ADM_getJobDir();
        fullname=new char[strlen(name)+strlen(base)+2+4];

        strcpy(fullname,base);
        strcat(fullname,"/");
        strcat(fullname,name);
        strcat(fullname,".js");

        if(!video_body->saveAsScript(fullname,final))
        {
          GUI_Error_HIG(QT_TR_NOOP("Saving failed"),QT_TR_NOOP("Saving the job failed. Maybe you have permission issue with ~/.avidemux"));
        }

        delete fullname;
        ADM_dealloc(name);
        ADM_dealloc(final);
}
int A_SaveWrapper(char *name)
{

        if(A_Save(name))
        {
          GUI_Info_HIG (ADM_LOG_INFO,QT_TR_NOOP("Done"),QT_TR_NOOP( "File %s has been successfully saved."),ADM_GetFileName(name));
        }
        else
        {
          GUI_Error_HIG (QT_TR_NOOP("Failed"), QT_TR_NOOP("File %s was NOT saved correctly."),ADM_GetFileName(name));
        }
        return 1;
}
uint8_t  DIA_v2v(char **vobname, char **ifoname,char **vobsubname);
uint8_t ADM_vob2vobsub(char *nameVob, char *nameVobSub, char *nameIfo);
#if BAZOOKA
int A_vob2vobsub(void)
{
        char *vob=NULL;
        char *ifo=NULL;
        char *vsub=NULL;
        int r=0;
        if(DIA_v2v(&vob,&ifo,&vsub))
        {
                r=ADM_vob2vobsub(vob,vsub,ifo);
                ADM_dealloc(vob);
                ADM_dealloc(ifo);
                ADM_dealloc(vsub);
        }
        return r;

}
#endif
///
///	Return the frame # corresponding to the position of the scale/slider
///	Rougth estimation in fact
///
uint32_t GUI_GetScale(void)
{

    double  percent;
    float tg;

    percent = UI_readScale();
    tg= (avifileinfo->nb_frames-1) * percent / 100.;

    return (uint32_t)floor(tg);;


}




/**
      \fn GUI_getFrameContent
      \brief fill image with content of frame frame
*/
uint8_t GUI_getFrameContent(ADMImage *image, uint32_t frame)
{
  uint32_t flags;
  if(!video_body->getUncompressedFrame(frame,image,&flags)) return 0;
  return 1;
}
/**
    \fn GUI_close
    \brief Close opened file and cleanup filters etc..
*/
uint8_t GUI_close(void)
{
  if (avifileinfo)		// already opened ?
    {				// delete everything
      // if preview is on
      admPreview::setMainDimension(0, 0);
      if(getPreviewMode()!=ADM_PREVIEW_NONE)
      {
	admPreview::stop();
        setPreviewMode(ADM_PREVIEW_NONE);
      }
      delete avifileinfo;
      //delete wavinfo;
      wavinfo = NULL;
      avifileinfo = NULL;
      video_body->cleanup ();
      curframe = 0;
      A_changeAudioStream (NULL, AudioNone,NULL);

	  if (aviaudiostream)
		delete aviaudiostream;

	  if (secondaudiostream)
		delete secondaudiostream;

	  aviaudiostream=NULL;
	  secondaudiostream=NULL;

      filterCleanUp ();
	  UI_setTitle(NULL);

      return 1;
    }
    return 0;
}
/**
      \fn GUI_avsProxy
      \brief Shortcut to connect to avsProxy
*/

void GUI_avsProxy(void)
{
  uint8_t res;


  GUI_close();
  res = video_body->addFile ("avsproxy.avs",0);
  // forget last project file
  if( actual_workbench_file ){
     ADM_dealloc(actual_workbench_file);
     actual_workbench_file = NULL;
  }

  if (res!=ADM_OK)			// an error occured
    {
        currentaudiostream = NULL;
        avifileinfo = NULL;
        GUI_Error_HIG (QT_TR_NOOP("AvsProxy"), QT_TR_NOOP("Failed to connect to avsproxy.\nIs it running ?"));
        return ;
    }

       updateLoaded ();
       UI_setTitle(QT_TR_NOOP("avsproxy"));
       return ;
}
/**
      \fn GUI_showCurrentFrameHex
      \brief Display the first 32 bytes of the current frame in hex
*/

void GUI_showCurrentFrameHex(void)
{
 uint8_t *buffer;
 uint32_t fullLen,flags;
 char sType[5];
 char sSize[15];
 ADMCompressedImage image;
 uint8_t seq;
 if (!avifileinfo) return;

 buffer=new uint8_t [avifileinfo->width*avifileinfo->height*3];
 image.data=buffer;


 video_body->getFrame (curframe,&image,&seq);
 fullLen=image.dataLength;
 video_body->getFlags (curframe, &flags);

 diaElemHex binhex("*****",fullLen,buffer);

 if(flags==AVI_KEY_FRAME) sprintf(sType,"I");
  else if(flags==AVI_B_FRAME) sprintf(sType,"B");
    else sprintf(sType,"P");
 sprintf(sSize,"%d bytes",fullLen);

 diaElemReadOnlyText Type(sType,QT_TR_NOOP("Frame type:"));
 diaElemReadOnlyText Size(sSize,QT_TR_NOOP("Frame size:"));
 diaElem *elems[]={&Type,&Size,&binhex   };
 if(diaFactoryRun(QT_TR_NOOP("Frame Hex Dump"),3,elems))

 delete [] buffer;
}
// Temporary place...

/**
    \fn saveAsBmp
    \brief save current image into filename, into bmp format
*/
uint8_t  ADMImage::saveAsBmp(const char *filename)
{
  ADM_BITMAPFILEHEADER bmfh;
  ADM_BITMAPINFOHEADER bmph;
  FILE *fd;
  uint32_t sz;
  uint16_t s16;
  uint32_t s32;

  sz = _width* _height * 3;

  bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
  bmfh.bfOffBits = sizeof (bmfh) + sizeof (bmph);
//_________________________________________
  bmph.biSize = sizeof (bmph);
  bmph.biWidth = _width;
  bmph.biHeight = _height;
  bmph.biPlanes = 1;
  bmph.biBitCount = 24;
  bmph.biCompression = 0;	// COMPRESSION NONE
  bmph.biSizeImage = sz;
  bmph.biXPelsPerMeter = 0;
  bmph.biYPelsPerMeter = 0;
  bmph.biClrUsed = 0;
  bmph.biClrImportant = 0;
/*
	bmph.resolutionUnits=0;
	bmph.origin=0;
	bmph.colorEncoding=0;
*/

  ADMImage image(_width,_height);


  printf ("\n %u x %u=%u\n", bmph.biWidth, bmph.biHeight, sz);

  uint8_t *out;

        out=(uint8_t *)ADM_alloc(sz);
        if(!out)
        {
            GUI_Error_HIG(QT_TR_NOOP("Memory error"), NULL);
//            ADM_dealloc(out);
            return 0;
        }

        if(!COL_yv12rgbBMP(bmph.biWidth, bmph.biHeight,data, out))
        {
              GUI_Error_HIG(QT_TR_NOOP("Error converting to BMP"), NULL);
              ADM_dealloc(out);
              return 0;
        }
        fd = fopen (filename, "wb");
        if (!fd)
        {
                GUI_Error_HIG (QT_TR_NOOP("Something bad happened"), NULL);
                ADM_dealloc(out);
                return 0;
        }

	// Bitmpap file header, not using tructure due to gcc padding it
#ifdef ADM_BIG_ENDIAN
	s16 = 0x424D;
#else
  	s16 = 0x4D42;
#endif
  	s32 = 14 + sizeof (bmph) + sz;
#ifdef ADM_BIG_ENDIAN
	#define SWAP32(x) x=R32(x)
#else
	#define SWAP32(x) ;
#endif
        SWAP32(s32);
        fwrite (&s16, 2, 1, fd);
        fwrite (&s32, 4, 1, fd);
        s32 = 0;
        fwrite (&s32, 4, 1, fd);
        s32 = 14 + sizeof (bmph);
        SWAP32(s32);
        fwrite (&s32, 4, 1, fd);
#ifdef ADM_BIG_ENDIAN
	Endian_BitMapInfo(&bmph);
#endif
        fwrite (&bmph, sizeof (bmph), 1, fd);
        fwrite (out, sz, 1, fd);

        fclose(fd);
        ADM_dealloc(out);
        return 1;
}
/**
    \fn saveAsJpg
    \brief save current image into filename, into jpg format
*/
uint8_t  ADMImage::saveAsJpg(const char *filename)
{
 ffmpegEncoderFFMjpeg *codec=NULL;
  FILE *fd;
  uint8_t *buffer=NULL;
  uint32_t sz;


        sz = _width*_height*3;
        ADMBitstream bitstream(sz);
        buffer=new uint8_t[sz];
        bitstream.data=buffer;
        codec=new  ffmpegEncoderFFMjpeg(_width,_height,FF_MJPEG)  ;
        codec->init( 95,25000);
        if(!codec->encode(this,&bitstream))
        {
                GUI_Error_HIG(QT_TR_NOOP("Cannot encode the frame"), NULL);
                delete [] buffer;
                delete codec;
                return 0;
        }
        delete codec;
        fd=fopen(filename,"wb");
        if(!fd)
        {
                GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP( "Cannot open \"%s\" for writing."), filename);
                delete [] buffer;
                return 0;
        }
        fwrite (buffer, bitstream.len, 1, fd);
        fclose(fd);
        delete [] buffer;
        return 1;
}
/**
 *      \fn UI_getPreferredRender
 *      \brief Returns to render lib the user preferred rendering method
 *
 */
ADM_RENDER_TYPE UI_getPreferredRender(void)
{
  char *displ;
  unsigned int renderI;
  ADM_RENDER_TYPE render;

#if !defined __WIN32 && !defined(__APPLE__)
        // First check if local
        // We do it in a very wrong way : If DISPLAY!=:0.0 we assume remote display
        // in that case we do not even try to use accel

        // Win32 and Mac/Qt4 don't have DISPLAY
        displ=getenv("DISPLAY");
        if(!displ)
        {
                return RENDER_GTK;
        }
        if(strcmp(displ,":0") && strcmp(displ,":0.0"))
        {
                printf("Looks like remote display, no Xv :%s\n",displ);
                return RENDER_GTK;
        }
#endif

        if(prefs->get(DEVICE_VIDEODEVICE,&renderI)!=RC_OK)
        {
                render=RENDER_GTK;
        }else
        {
                render=(ADM_RENDER_TYPE)renderI;
        }

        return render;
}

const char *getStrFromAudioCodec( uint32_t codec)
{
      switch(codec)
      {
              case WAV_DTS: return QT_TR_NOOP("DTS");
              case WAV_PCM: return QT_TR_NOOP("PCM");
              case WAV_MP2: return QT_TR_NOOP("MP2");
              case WAV_MP3: return QT_TR_NOOP("MP3");
              case WAV_WMA:  return QT_TR_NOOP("WMA");
              case WAV_LPCM: return QT_TR_NOOP("LPCM");
              case WAV_AC3:  return QT_TR_NOOP("AC3");
              case WAV_OGG: return QT_TR_NOOP("Ogg Vorbis");
              case WAV_MP4: return QT_TR_NOOP("MP4");
              case WAV_AAC: return QT_TR_NOOP("AAC");
              case WAV_QDM2: return QT_TR_NOOP("QDM2");
              case WAV_AMRNB: return QT_TR_NOOP("AMR-NB");
              case WAV_MSADPCM: return QT_TR_NOOP("MSADPCM");
              case WAV_ULAW: return QT_TR_NOOP("ULAW");
              case WAV_IMAADPCM: return QT_TR_NOOP("IMA ADPCM");
              case WAV_8BITS_UNSIGNED:return QT_TR_NOOP("8-bit PCM");
      }
      return QT_TR_NOOP("Unknown codec");
}

//
// EOF
