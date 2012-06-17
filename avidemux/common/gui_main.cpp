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
#include "ADM_cpp.h"
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
#include "ADM_videoEncoderApi.h"
#include "ADM_audioFilter/include/ADM_audioFilterInterface.h"

#include "avi_vars.h"
#include "prototype.h" // FIXME
#include "ADM_script2/include/ADM_script.h"

renderZoom currentZoom=ZOOM_1_1;
#include "DIA_audioTracks.h"
//***********************************
//******** A Function ***************
//***********************************
#include "A_functions.h"
//***********************************
//******** GUI Function**************
//***********************************
extern const char * GUI_getCustomJsScript(uint32_t nb);
extern const char * GUI_getCustomPyScript(uint32_t nb);
extern const char * GUI_getAutoPyScript(uint32_t nb);

extern int     GUI_handleVFilter (void);
// Debug functions
       void    GUI_showCurrentFrameHex(void);
       void    GUI_showSize(void);

       void    GUI_avsProxy(void);
       uint8_t GUI_close(void);
extern bool    GUI_GoToTime(uint64_t time);
//***********************************
//******** DIA Function**************
//***********************************
extern uint8_t DIA_about( void );
extern void    DIA_properties( void);
extern uint8_t DIA_Preferences(void);
extern uint8_t DIA_builtin(void);
extern uint8_t DIA_pluginsInfo(void);
extern void GUI_ScriptHelp(void);

static void ReSync (void);
void cleanUp (void);
void        updateLoaded (void);

extern void GUI_OpenApplicationLog();
extern void GUI_OpenApplicationDataFolder();

extern bool ADM_mux_configure(int index);
void brokenAct(void);
//
//  Sub gui files...
//
void HandleAction (Action action);
void HandleAction_Navigate(Action action);
void HandleAction_Save(Action action);

// Hacky functions because we currently don't have versatile
// file dialogs
static IScriptEngine *tempEngine;

static void RunScript(const char *name)
{
	tempEngine->runScriptFile(name, IScriptEngine::DebugOnError);
	A_Resync(); // total duration & stuff
}

static void DebugScript(const char *name)
{
	tempEngine->runScriptFile(name, IScriptEngine::Debug);
    A_Resync(); // total duration & stuff
}

//
//
/**
    \fn HandleAction
    \brief  serialization of user event through gui

*/
typedef  const char * (*getName)(uint32_t nb);
bool getScriptName(int action, int base,getName name,const char *ext,string &out )
{
    if(action<base) return false;
    action=action-base;
    const char *p=name(action);
    if(!p) return false;
    out=string(p)+string(".")+string(ext);
    return true;
}
void HandleAction (Action action)
{
  uint32_t nf = 0;
  uint32_t old;

  ADM_warning("************ %s **************\n",getActionName(action));

  // handle out of band actions
  // independant load not loaded
//------------------------------------------------
int nw;
#ifdef USE_SPIDERMONKEY
  if(action>=ACT_CUSTOM_BASE_JS && action <ACT_CUSTOM_END_JS)
  {
      string script;
      if(true==getScriptName( action, ACT_CUSTOM_BASE_JS,GUI_getCustomJsScript,"js",script))
      {
            A_parseECMAScript(script.c_str());
      }
      return ;
  }
#endif

#ifdef USE_TINYPY
  if(action>=ACT_CUSTOM_BASE_PY && action <ACT_CUSTOM_END_PY)
  {
      string script;
      if(true==getScriptName( action, ACT_CUSTOM_BASE_PY,GUI_getCustomPyScript,"py",script))
      {
            A_parseTinyPyScript(script.c_str());
      }
      return ;
  }
  if(action>=ACT_AUTO_BASE_PY && action <ACT_AUTO_END_PY)
  {
      string script;
      if(true==getScriptName( action, ACT_AUTO_BASE_PY,GUI_getAutoPyScript,"py",script))
      {
            A_parseTinyPyScript(script.c_str());
      }
      return ;
  }
#endif

	if (action >= ACT_SCRIPT_ENGINE_FIRST && action < ACT_SCRIPT_ENGINE_LAST)
	{
		int engineIndex = (action - ACT_SCRIPT_ENGINE_FIRST) / 3;
		int actionId = (action - ACT_SCRIPT_ENGINE_FIRST) % 3;

		tempEngine = getScriptEngines()[engineIndex];

		switch (actionId)
		{
			case 0:
				GUI_FileSelRead("Select script to run", RunScript);
				break;

			case 1:
				GUI_FileSelRead("Select script to debug", DebugScript);
				break;

			case 2:
				// Hack until save routines are moved to IScriptEngine
				if (tempEngine->name().compare("SpiderMonkey") == 0)
				{
					GUI_FileSelWrite(QT_TR_NOOP("Select jsProject to Save"), A_saveJsProject);
				}
				else if (tempEngine->name().compare("Python") == 0)
				{
					GUI_FileSelWrite(QT_TR_NOOP("Select pyProject to Save"), A_savePyProject);
				}

				break;
		}

		return;
	}

  switch (action)
    {
        case ACT_TimeShift:
        case ACT_Goto:
                                brokenAct();
                                return;
#ifdef USE_TINYPY
        case ACT_PY_SHELL:
                                interactiveScript(getPythonEngine());
                                return;
#endif
#ifdef USE_SPIDERMONKEY
        case ACT_JS_SHELL:
                                interactiveScript(getSpiderMonkeyEngine());
                                return;
#endif
        case ACT_AVS_PROXY:
                                GUI_avsProxy();
                                return;
        case ACT_BUILT_IN:
                                DIA_builtin();
                                return;
        case ACT_RECENT0:
        case ACT_RECENT1:
        case ACT_RECENT2:
        case ACT_RECENT3:
                const char **name;
                int rank;

                name=prefs->get_lastfiles();
                rank=(int)action-ACT_RECENT0;
                ADM_assert(name[rank]);
                A_openAvi (name[rank]);
                return;
		return;
	case ACT_VIDEO_CODEC_CONFIGURE:
    		videoEncoder6Configure();
            return;
    case ACT_ContainerConfigure:
            {
            int index=UI_GetCurrentFormat();
            ADM_mux_configure(index);
            return;
            }
    case ACT_VIDEO_CODEC_CHANGED:
    		nw=UI_getCurrentVCodec();
    		videoEncoder6_SetCurrentEncoder(nw);
            return;
   case ACT_AUDIO_CODEC_CHANGED:
            nw=UI_getCurrentACodec();
            audioCodecSetByIndex(0,nw);
            return;
    case ACT_PLUGIN_INFO:
            DIA_pluginsInfo();
            return;
	case ACT_OPEN_APP_LOG:
		GUI_OpenApplicationLog();
		break;
	case ACT_OPEN_APP_FOLDER:
		GUI_OpenApplicationDataFolder();
		break;

    case ACT_ABOUT :
    		 DIA_about( );
		 return;
	case ACT_SCRIPT_HELP:
		GUI_ScriptHelp();
		return;
    case ACT_AUDIO_CODEC_CONFIGURE:
      audioCodecConfigure(0);
      return;
    case ACT_AUDIO_FILTERS:
        {
            EditableAudioTrack *ed=video_body->getDefaultEditableAudioTrack();
            if(ed) ed->audioEncodingConfig.audioFilterConfigure();
        }
      return;
    case ACT_PREFERENCES:
        if(playing) return;
    	if(DIA_Preferences())
        {
            prefs->save ();
        }
        return;
    case ACT_SavePref:
        prefs->save ();
        return;
    case ACT_EXIT:
	  UI_closeGui();
      return;
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
          case ACT_OPEN_VIDEO:
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
        case ACT_AUDIO_SELECT_TRACK:
                A_audioTrack();
                break;

    case ACT_OPEN_VIDEO:
        GUI_FileSelRead (QT_TR_NOOP("Select AVI File..."),(SELFILE_CB *) A_openAvi);
        break;
    case ACT_APPEND_VIDEO:
        GUI_FileSelRead (QT_TR_NOOP("Select AVI File to Append..."),(SELFILE_CB *) A_appendAvi);
        break;
    case ACT_VIDEO_PROPERTIES:
        DIA_properties ();
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
    case ACT_MarkA:
    case ACT_MarkB:
    {
      bool swapit=0;
      uint64_t markA,markB;
      uint64_t pts=admPreview::getCurrentPts();
      if( prefs->get(FEATURES_SWAP_IF_A_GREATER_THAN_B, &swapit) != RC_OK )     swapit = 1;

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
            brokenAct();
//    		   video_body->copyToClipBoard (frameStart,frameEnd);
		break;
    case ACT_Paste:
            brokenAct();
            break;
      break;

    case ACT_ResetSegments:
       if(avifileinfo)
         if(GUI_Question(QT_TR_NOOP("Are you sure?")))
        {
            video_body->resetSeg();
            video_body->getVideoInfo (avifileinfo);

            A_ResetMarkers();
      		ReSync ();

            // forget last project file
            video_body->setProjectName("");
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
              //A_Rewind();
              GUI_GoToTime(a);

            }
        }

      break;
      // set decoder option (post processing ...)
    case ACT_DecoderOption:
      video_body->setDecodeParam ( admPreview::getCurrentPts());

      break;
    case ACT_VIDEO_FILTERS:
        GUI_handleVFilter();
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

/**
        \fn A_openAvi
        \brief Open (replace mode) a video
*/
int A_openAvi (const char *name)
{
  uint8_t res;
  char *longname;
  uint32_t magic[4];
  uint32_t id = 0;

  if (playing)
    return 0;
  /// check if name exists
  FILE *fd;
  fd = ADM_fopen (name, "rb");
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
    video_body->setProjectName("");

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
#warning FIXME
#if 0
	if( (fd = ADM_fopen(longname,"rb"))  ){
		if( fread(magic,4,1,fd) == 4 ){
			/* remember a workbench file */
			if( !strncmp(magic,"ADMW",4) ){
				actual_workbench_file = ADM_strdup(longname);
			}
		}
		fclose(fd);
	}
#endif
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
            EditableAudioTrack *ed=video_body->getDefaultEditableAudioTrack();
            if(ed) ed->audioEncodingConfig.audioFilterSetMixer(CHANNEL_INVALID);

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
  WAVHeader *wavinfo=NULL;
  ADM_audioStream *stream=NULL;
  video_body->getDefaultAudioTrack(&stream);
  if(stream)
        wavinfo=stream->getInfo();

  if (!wavinfo)
    {
      printf ("\n *** NO AUDIO ***\n");
      wavinfo = (WAVHeader *) NULL;
    }

  // Init renderer
    admPreview::setMainDimension(avifileinfo->width, avifileinfo->height,ZOOM_AUTO);
  // Draw first frame
    GUI_setAllFrameAndTime();
    A_ResetMarkers();
    A_Rewind();
    ADM_info(" conf updated \n");
    UI_setDecoderName(video_body->getVideoDecoderName());

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

  // update audio stream
  // If we were on avi , mark it...
  GUI_setAllFrameAndTime ();

}




//      Clean up
//      free all pending stuff, make leakchecker happy
//
void cleanUp (void)
{
	bool saveprefsonexit;

	prefs->get(FEATURES_SAVEPREFSONEXIT, &saveprefsonexit);

	if (saveprefsonexit)
	{
		prefs->save();
	}

	if (avifileinfo)
	{
		delete avifileinfo;
		avifileinfo=NULL;
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

static bool parseScript(IScriptEngine *engine, const char *name)
{
	bool ret;
	char *longname = ADM_PathCanonize(name);

	if (playing)
	{
		return false;
	}

	ret = engine->runScriptFile(std::string(longname), IScriptEngine::Normal);
	A_Resync(); // total duration & stuff

	if (ret)
	{
		video_body->setProjectName(longname);
	}

	ADM_dealloc(longname);
	return ret;
}

#ifdef USE_TINYPY
/**
    \fn A_parseTinyPyScript
*/
bool A_parseTinyPyScript(const char *name)
{
	return parseScript(getPythonEngine(), name);
}
#endif

#ifdef USE_QTSCRIPT
bool A_parseQtScript(const char *name)
{
	return parseScript(getQtScriptEngine(), name);
}
#endif

#ifdef USE_SPIDERMONKEY
/**
    \fn A_parseECMAScript
*/
bool A_parseECMAScript(const char *name)
{
	return parseScript(getSpiderMonkeyEngine(), name);
}
#endif

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
extern int DIA_getMPParams( uint32_t *pplevel, uint32_t *ppstrength,bool *swap);
//

//
void	A_setPostproc( void )
{
uint32_t type,strength;
bool swap;
	if(!avifileinfo) return;

	video_body->getPostProc(&type,&strength,&swap);

 	if(DIA_getMPParams( &type, &strength,&swap))
 	{
		video_body->setPostProc(type,strength,swap);
 	}

}
extern const char *getStrFromAudioCodec( uint32_t codec);
/**

*/
int A_setAudioTrack(int track)
{
        video_body->changeAudioStream(0,track);
        return true;
}
/**
      \fn A_audioTrack
      \brief Allow to select audio track
*/

void A_audioTrack( void )
{
        PoolOfAudioTracks *pool=video_body->getPoolOfAudioTrack();
        ActiveAudioTracks *active=video_body->getPoolOfActiveAudioTrack();
        DIA_audioTrackBase *base=createAudioTrack(pool,active);
        base->run();
        delete base;
        EditableAudioTrack *ed=video_body->getDefaultEditableAudioTrack();
        if(ed)
        {
            UI_setAudioCodec(ed->encoderIndex);
        }

}
#if 0
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
                    A_setAudioTrack(newTrack);
            }
        }

roger_and_out:
         /* Clean up */
         for(int i=0;i<nbAudioTracks;i++)
            delete sourceavitracks[i];
        return;

}
#endif
/**
        \fn A_externalAudioTrack
        \brief Select external audio track (for 2nd track)
*/
void A_externalAudioTrack( void )
{
}
/**
    \fn A_Resync
    \brief
*/
void A_Resync(void)
{
        if(!avifileinfo) return;
        GUI_setAllFrameAndTime();
        UI_setMarkers (video_body->getMarkerAPts(),video_body->getMarkerBPts());
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
        strcat(fullname,".py");

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
      admPreview::setMainDimension(0, 0,ZOOM_1_1);
      if(getPreviewMode()!=ADM_PREVIEW_NONE)
      {
        admPreview::stop();
        setPreviewMode(ADM_PREVIEW_NONE);
      }
      delete avifileinfo;
      //delete wavinfo;

      avifileinfo = NULL;
      video_body->cleanup ();

//      filterCleanUp ();
	  UI_setTitle(NULL);

	A_ResetMarkers();
	ReSync();

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
  res = video_body->addFile (AVS_PROXY_DUMMY_FILE);
  // forget last project file
  video_body->setProjectName("avsproxy");
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

 if(flags&AVI_KEY_FRAME) sprintf(sType,"I");
  else if(flags&AVI_B_FRAME) sprintf(sType,"B");
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

        if(prefs->get(VIDEODEVICE,&renderI)!=RC_OK)
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
        ADM_info("Video Total duration : %s ms\n",ADM_us2plain(duration));
        video_body->setMarkerAPts(0);
        video_body->setMarkerBPts(duration);
        UI_setMarkers(0,duration);

}
/**
    \fn A_Rewind
    \brief Go back to the first frame
*/
void A_Rewind(void)
{
               admPreview::stop();
               video_body->rewind();
               admPreview::start();
               admPreview::samePicture();
               admPreview::samePicture();
               GUI_setCurrentFrameAndTime();
}
void brokenAct(void)
{
    GUI_Error_HIG("Oops","This function is disabled or no longer valid");
}

//
// EOF
