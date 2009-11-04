/***************************************************************************
                          \file gtk_gui.cpp  
                          \brief Main UI even loop
                             
    copyright            : (C) 2001-2009 by mean, fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include <math.h>
#include <errno.h>

#include "fourcc.h"

#include "DIA_fileSel.h"
#include "DIA_coreToolkit.h"

#include "gui_action.hxx"
#include "gtkgui.h"

#include "prefs.h"
#include "ADM_render/GUI_render.h"
#include "ADM_commonUI/GUI_ui.h"

#include "DIA_working.h"
#include "DIA_factory.h"

#include "ADM_vidMisc.h"
#include "ADM_preview.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_audioFilter/include/ADM_audioFilterInterface.h"

#include "avi_vars.h"
#include "prototype.h" // FIXME
char * actual_workbench_file;
renderZoom currentZoom=ZOOM_1_1;
//***********************************
//******** A Function ***************
//***********************************
int     A_delete(uint32_t start, uint32_t end);
void    A_externalAudioTrack( void );
uint8_t A_rebuildKeyFrame (void);
void    A_openBrokenAvi (const char *name);
int     A_openAvi2 (const char *name, uint8_t mode);
int     A_appendAvi (const char *name);
void    A_saveWorkbench (const char *name);
static void A_videoCheck( void);
static void	A_setPostproc( void );
void    A_Resync(void);
void    A_addJob(void);
void    A_audioTrack(void);
extern int A_Save(const char *name);
int     A_SaveWrapper( char *name);
void    A_parseECMAScript(const char *name);
extern uint8_t A_autoDrive(Action action);
uint8_t A_TimeShift(void);
void    A_ResetMarkers(void);
extern void A_jog(void);
uint8_t A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms);
//***********************************
//******** GUI Function**************
//***********************************

extern uint8_t UI_getPhysicalScreenSize(void* window, uint32_t *w,uint32_t *h);
extern uint8_t GUI_jobs(void);
extern const char * GUI_getCustomScript(uint32_t nb);

extern uint8_t GUI_getFrameContent(ADMImage *image, uint32_t frame);
extern int     GUI_handleVFilter (void);
extern void    GUI_setMarks (uint32_t a, uint32_t b);
extern void    GUI_displayBitrate( void );
// Debug functions
       void    GUI_showCurrentFrameHex(void);
       void    GUI_showSize(void);

       void    GUI_avsProxy(void);
       uint8_t GUI_close(void);
extern int     GUI_GoToFrame(uint32_t frame);;
//***********************************
//******** DIA Function**************
//***********************************
extern uint8_t DIA_about( void );
extern uint8_t DIA_RecentFiles( char **name );
extern void    DIA_properties( void);
extern uint8_t DIA_Preferences(void);
extern uint8_t DIA_gotoTime(uint16_t *hh, uint32_t *mm, uint32_t *ss);
extern uint8_t DIA_builtin(void);
extern void    DIA_Calculator(uint32_t *sizeInMeg, uint32_t *avgBitrate );
extern uint8_t DIA_pluginsInfo(void);

extern void filterCleanUp (void);
static void ReSync (void);
static void cleanUp (void);
void        updateLoaded (void);
extern void encoderSetLogFile (char *name);

//__________
extern bool parseECMAScript(const char *name);

extern void videoCodecConfigureUI(int codecIndex = -1);
extern void audioCodecChanged(int newcodec);
extern void videoCodecChanged(int newcodec);
extern bool ADM_mux_configure(int index);
//
//  Sub gui files...
//
void HandleAction (Action action);
void HandleAction_Navigate(Action action);
void HandleAction_Save(Action action);
//
//
/**
    \fn HandleAction
    \brief  serialization of user event through gui

*/
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
      case ACT_AudioConfigure:
//    		audioCodecSelect();
		return;
	case ACT_VideoConfigure:
    		videoEncoder6Configure();
            return;
    case ACT_VideoCodecChanged:
    		nw=UI_getCurrentVCodec();
    		videoEncoder6_SetCurrentEncoder(nw);
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
      videoEncoder6Configure();
      return;
    case ACT_AudioCodec:
      audioCodecConfigure();
      return;
    case ACT_AudioFilters:
      audioFilterConfigure();
      return;
    case ACT_Pref:
        if(playing) return;
    	if(DIA_Preferences())
        {
            prefs->save ();
        }
        return;
    case ACT_SavePref:
        prefs->save ();
        return;
    case ACT_SetMuxParam:
        {
        int index=UI_GetCurrentFormat();
        ADM_mux_configure(index);
        return;
        }
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
    default:
      break;

    }

  if (playing)			// only allow some action
    {
      switch (action)
        {
        case ACT_PlayAvi:
        case ACT_StopAvi:
          break;
        default:
          return;
        }
    }
  // not playing,
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
          default:
            break;
        }
        return;
    }

  // Dispatch actions, we have a file loaded
  if(action>ACT_NAVIGATE_BEGIN && action < ACT_NAVIGATE_END)
  {
    return HandleAction_Navigate(action);
  }
  if(action>ACT_SAVE_BEGIN && action < ACT_SAVE_END)
  {
    return HandleAction_Save(action);
  }

  switch (action)
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
        case ACT_SelectTrack1:
                A_audioTrack();
                break;

        case ACT_Bitrate:
    			{
				uint32_t a,b;
//				DIA_Calculator(&a,&b );
			}
    			break;

        case ACT_ADD_JOB:
            A_addJob();
            break;

    case ACT_OpenAvi:
        GUI_FileSelRead (QT_TR_NOOP("Select AVI File..."),(SELFILE_CB *) A_openAvi);
        break;
    case ACT_AppendAvi:
        GUI_FileSelRead (QT_TR_NOOP("Select AVI File to Append..."),(SELFILE_CB *) A_appendAvi);
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
    case ACT_MarkA:
    case ACT_MarkB:
    {
      uint32_t swapit=0;
      uint64_t markA,markB;
      uint64_t pts=admPreview::getCurrentPts();
      if( prefs->get(FEATURE_SWAP_IF_A_GREATER_THAN_B, &swapit) != RC_OK )     swapit = 1;

      markA=video_body->getMarkerAPts();
      markB=video_body->getMarkerBPts();
      if (action == ACT_MarkA)
            markA=pts;
      else
            markB=pts;
      if (markA>markB && swapit )	// auto swap
        {
          uint64_t y;
          y = markA;
          markA=markB;
          markB=y;
        }
        video_body->setMarkerAPts(markA);
        video_body->setMarkerBPts(markB);
        UI_setMarkers (markA, markB);
      break;
    }
    case ACT_Copy:
      		if( frameEnd < frameStart ){
                  GUI_Error_HIG(QT_TR_NOOP("Marker A > B"), QT_TR_NOOP("Cannot copy."));
      		}else{
//    		   video_body->copyToClipBoard (frameStart,frameEnd);
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
		
      		GUI_setAllFrameAndTime ();
            A_ResetMarkers();
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
        {
            uint64_t a=video_body->getMarkerAPts();
            uint64_t b=video_body->getMarkerBPts();
            if(false==video_body->remove(a,b))
            {
                GUI_Error_HIG("Cutting","Error while cutting out.");
            }
            else    
            {
              A_ResetMarkers();              
              A_Resync(); // total duration & stuff
            // Rewind to first frame...
               video_body->rewind();
               admPreview::samePicture();
               GUI_setCurrentFrameAndTime();
            }
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
      video_body->setDecodeParam ( admPreview::getCurrentPts());

      break;
    case ACT_VideoParameter:
#if 0
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
#endif
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
   case ACT_SIZE_DUMP:
      GUI_showSize();
      break;
    default:
      printf ("\n unhandled action %d\n", action);
      ADM_assert (0);
      return;

    }
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

//  DIA_StartBusy ();
  /*
  ** we may get a relative path by cmdline
  */
  longname = ADM_PathCanonize(name);
  res = video_body->addFile (longname);
//  DIA_StopBusy ();

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
            if(video_body->getAudioStreamsInfo(admPreview::getCurrentPts()+1,&nbAudio,&infos))
            {
                if(nbAudio>1)
                {   // Multiple track warn user
                  GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Multiple Audio Tracks"),QT_TR_NOOP("The file you just loaded contains several audio tracks.\n"
                      "Go to Audio->MainTrack to select the active one."));
                }
            }
            if(infos) delete [] infos;
            // Revert mixer to copy
            //setCurrentMixerFromString("NONE");
            audioFilterSetMixer(CHANNEL_INVALID);
        }
	for(i=strlen(longname);i>=0;i--)
    {
#ifdef __WIN32
		if( longname[i] == '\\' || longname[i] == '/' )
#else
		if( longname[i] == '/' )
#endif
        {

			i++;
			break;
		}
    }
	UI_setTitle(longname+i);
    }

	delete[] longname;
	return 1;
}
/**
    \fn updateLoaded
    \brief update the UI after loading a file

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


//  getFirstVideoFilter(); // reinit first filter

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
//      A_changeAudioStream (aviaudiostream, AudioAvi,NULL);
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
  A_ResetMarkers();
//  getFirstVideoFilter(); // Rebuild filter if needed

  /* Zoom out if needed */
  uint32_t phyW,phyH;
  UI_getPhysicalScreenSize(NULL, &phyW,&phyH);
  if(3*phyW<4*avifileinfo->width || 3*phyH<4*avifileinfo->height)
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



      video_body->rewind();
      admPreview::samePicture();
      GUI_setCurrentFrameAndTime();

   ADM_info(" conf updated \n");
}

//___________________________________________
//  Append an AVI to the existing one
//___________________________________________
int
A_appendAvi (const char *name)
{


  if (playing)
    return 0;
//  DIA_StartBusy ();
  if (!video_body->addFile (name))
    {
//      DIA_StopBusy ();
      GUI_Error_HIG (QT_TR_NOOP("Something failed when appending"), NULL);
      return 0;
    }
//  DIA_StopBusy ();


//  video_body->dumpSeg ();
  if (!video_body->updateVideoInfo (avifileinfo))
    {
      GUI_Error_HIG (QT_TR_NOOP("Something bad happened (II)"), NULL);
      return 0;
    }

  ReSync ();
  A_ResetMarkers();
  
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
//      A_changeAudioStream (aviaudiostream, AudioAvi,NULL);
    }
  	//updateVideoFilters ();
//	getFirstVideoFilter();

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
#if 0
	if (aviaudiostream)
	{
		delete aviaudiostream;
		aviaudiostream=NULL;
	}
#endif
	if (secondaudiostream)
	{
		delete secondaudiostream;
		secondaudiostream=NULL;
	}
#if 0
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
#endif
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
//	filterCleanUp();
	admPreview::cleanUp();
}

#warning fixme



void A_parseECMAScript(const char *name){
  bool ret;
  char *longname = ADM_PathCanonize(name);
   if (playing){
      GUI_PlayAvi();
   }
   ret = parseECMAScript(longname);
   if( ret == 0 ){
      if( actual_workbench_file )
         ADM_dealloc(actual_workbench_file);
      actual_workbench_file = ADM_strdup(longname);
   }
   ADM_dealloc(longname);
}

/*
	Unpack all frames without displaying them to check for error

*/
void A_videoCheck( void)
{
#if 0
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
	GUI_GoToFrame(0);
#endif
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

//      video_body->dumpSeg ();
//      if (!video_body->removeFrames (start, end))
    if(0)
	{
          GUI_Error_HIG (QT_TR_NOOP("Something bad happened"), NULL);
	  return 0;
	}
//      video_body->dumpSeg ();
      //resync GUI and video
      if (!video_body->updateVideoInfo (avifileinfo))
	{
          GUI_Error_HIG (QT_TR_NOOP("Something bad happened (II)"), NULL);
	}


      A_ResetMarkers();
      GUI_setAllFrameAndTime ();
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
extern const char *getStrFromAudioCodec( uint32_t codec);
/**
      \fn A_audioTrack
      \brief Allow to select audio track
*/
void A_audioTrack( void )
{        
        audioInfo *infos=NULL;
        uint32_t nbAudioTracks,currentAudioTrack;
        uint32_t newTrack;

        if(!video_body->getAudioStreamsInfo(0,&nbAudioTracks,&infos)) return;
        currentAudioTrack=video_body->getCurrentAudioStreamNumber(0);
        newTrack=currentAudioTrack;
        // Now build the list of embedded track
#define MAX_AUDIO_TRACK 10
#define MAX_AUDIO_TRACK_NAME 100
        diaMenuEntryDynamic *sourceavitracks[MAX_AUDIO_TRACK];
        char string[MAX_AUDIO_TRACK_NAME];
        for(int i=0;i<nbAudioTracks;i++)
        {
          sprintf(string,"Audio track %d (%s, %d channels, %d kbit/s)",i,
                        getStrFromAudioCodec(infos[i].encoding),
                        infos[i].channels,infos[i].bitrate);
           sourceavitracks[i]=new diaMenuEntryDynamic(i,string,NULL);
        }
         if(infos) delete [] infos;

         diaElemMenuDynamic   sourceFromVideo(&newTrack,QT_TR_NOOP("_Track from video:"),nbAudioTracks,sourceavitracks);
         diaElem *allWidgets[]={&sourceFromVideo};

         if( diaFactoryRun(QT_TR_NOOP("Main Audio Track"),1,allWidgets))
         {
            if(newTrack!=currentAudioTrack)
            {
                    video_body->changeAudioStream(0,newTrack);
                    video_body->getAudioStream(&aviaudiostream);
            }
        }
      
roger_and_out:
         /* Clean up */
         for(int i=0;i<nbAudioTracks;i++)
            delete sourceavitracks[i];
        return;

}
/**
        \fn A_externalAudioTrack
        \brief Select external audio track (for 2nd track)
*/
void A_externalAudioTrack( void )
{
#if 0
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
#endif
}
#if 0
uint8_t A_setSecondAudioTrack(const AudioSource nw,char *name)
{

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

}
#endif
/**
    \fn A_Resync
    \brief 
*/
void A_Resync(void)
{
        if(!avifileinfo) return;
        GUI_setAllFrameAndTime();
        UI_setMarkers (video_body->getMarkerAPts(),video_body->getMarkerBPts());
        GUI_GoToFrame(0);
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

        delete [] fullname;
        delete [] base;
        ADM_dealloc(name);
        ADM_dealloc(final);
}
/**
    \fn GUI_GetScale
    \brief Return the % of the scale, between 0 and ADM_SCALE_SIZE

*/
uint32_t GUI_GetScale(void)
{

    double  percent;
    float tg;

    percent = UI_readScale();
    tg= ADM_SCALE_SIZE * percent / 100.;

    return (uint32_t)floor(tg);;
}
/**
    \fn GUI_SetScale
    \brief Set the scale, input is between 0 and ADM_SCALE_SIZE (max)
*/
void     GUI_SetScale( uint32_t scale )
{
    double percent;
    percent=scale;
    percent/=ADM_SCALE_SIZE;
    percent*=100;
    UI_setScale(percent);
}


/**
      \fn GUI_getFrameContent
      \brief fill image with content of frame frame
*/
uint8_t GUI_getFrameContent(ADMImage *image, uint32_t frame)
{
//  uint32_t flags;
//  if(!video_body->getUncompressedFrame(frame,image,&flags)) return 0;
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
      // Audio streams are cleared by editor

	  aviaudiostream=NULL;
	  secondaudiostream=NULL;

//      filterCleanUp ();
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
  res = video_body->addFile ("avsproxy.avs");
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
#if 0
 if (!avifileinfo) return;

 buffer=new uint8_t [avifileinfo->width*avifileinfo->height*3];
 image.data=buffer;


 video_body->getFrame (video_body->getCurrentFrame(),&image,&seq);
 fullLen=image.dataLength;
 video_body->getFlags (video_body->getCurrentFrame(), &flags);

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
#endif
}
/**
    \fn GUI_showSize
    \brief Show frame size

*/
#define DUMP_SIZE 30
void GUI_showSize(void)
{
uint8_t *buffer;
 uint32_t fullLen,flags;
 ADMCompressedImage image;
 uint8_t seq;
 char                text[DUMP_SIZE][100];
 
 if (!avifileinfo) return;
#if 0
 buffer=new uint8_t [avifileinfo->width*avifileinfo->height*3];
 image.data=buffer;
    for(int i=0;i<DUMP_SIZE;i++)
    {
        int target=video_body->getCurrentFrame()+i;
        video_body->getFlags ( target,&flags);
        video_body->getFrame ( target,&image,&seq);
        fullLen=image.dataLength;
        sprintf(text[i],"Frame %d:%d",target,fullLen);
        printf("%s\n",text[i]);
    }
 


 
 delete [] buffer;
 #endif
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

/**
    \fn A_ResetMarkers
*/
void A_ResetMarkers(void)
{
uint64_t duration=video_body->getVideoDuration();
        video_body->setMarkerAPts(0);
        video_body->setMarkerBPts(duration);
}
//
// EOF
