/** *************************************************************************
            \file gui_save.cpp
            \brief handle all kind of save

    copyright            : (C) 2002/2009 by mean
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

#include "ADM_cpp.h"
#include "ADM_default.h"
#include "avi_vars.h"

#include <math.h>
#include "prototype.h"
#include "gui_action.hxx"
#include "gtkgui.h"

#include "DIA_coreToolkit.h"
#include "ADM_commonUI/GUI_ui.h"
#include "DIA_enter.h"

#include "ADM_vidMisc.h"
#include "DIA_fileSel.h"
#include "DIA_working.h"
#include "ADM_preview.h"
#include "DIA_factory.h"
#include "ADM_coreJobs.h"
#include "ADM_audioWrite.h"
// Local prototypes
#include "A_functions.h"
#include "ADM_script2/include/ADM_script.h"

int      A_Save(const char *name);
extern   ADM_audioStream  *audioCreateEncodingStream(EditableAudioTrack *ed,bool globalHeader,uint64_t startTime,int32_t shift);

/**
    \fn HandleAction_Navigate

*/
void HandleAction_Save(Action action)
{
    if(!video_body->getNbSegment())
    {
        GUI_Error_HIG("No","No file loaded");
        return;
    }
    switch(action)
    {
    case ACT_SAVE_QUEUE:
            {
                char *oFile=NULL;
                char *oText=NULL;
                diaElemFile wFile(1,&oFile,QT_TR_NOOP("Output file"),"");
                diaElemText wText(&oText,QT_TR_NOOP("Job name"));
                diaElem *elems[2]={&wText,&wFile};

                if(  diaFactoryRun(QT_TR_NOOP("Queue job to jobList"),2,elems))
                {
                    A_queueJob(oText,oFile);
                }
            }
            break;
#if 0
   case ACT_SaveCurrentWork:
        {
          const std::string pj=video_body->getProjectName();
          if( pj.length() ){
             A_saveJsProject( pj.c_str() ); // will write "actual_workbench_file" itself
          }else{
            GUI_FileSelWrite (QT_TR_NOOP("Select Workbench to Save"), A_saveJsProject);
            UI_refreshCustomMenu();
          }
        }
      break;
#endif
    case ACT_SAVE_AUDIO:
      	{
          GUI_FileSelWrite (QT_TR_NOOP("Select File to Save Audio"),(SELFILE_CB *)A_audioSave);
        }
      break;

    case ACT_SAVE_BUNCH_OF_JPG:
      GUI_FileSelWrite (QT_TR_NOOP("Select JPEG Sequence to Save"), (SELFILE_CB *)A_saveBunchJpg);
    	break;
    case ACT_SAVE_BMP:
      GUI_FileSelWrite (QT_TR_NOOP("Select BMP to Save"), (SELFILE_CB *)A_saveImg);
      //GUI_FileSelWrite ("Select Jpg to save ", A_saveJpg);
      break;
    case ACT_SAVE_JPG :
      GUI_FileSelWrite (QT_TR_NOOP("Select JPEG to Save"), (SELFILE_CB *)A_saveJpg);
      	//GUI_FileSelWrite ("Select Jpg to save ", A_saveJpg);
      	break;
//----------------------test-----------------------
    case ACT_SAVE_VIDEO:
      GUI_FileSelWrite (QT_TR_NOOP("Select File to Save"),(SELFILE_CB *)A_SaveWrapper); // A_SaveAudioNVideo);
      break;
//---------------------------------------------------

    default:
        ADM_assert(0);
        break;
    }
}
/**
    \fn A_audioSave
    \brief Save audio track
*/

int A_audioSave(const char *name)
{
    ADM_audioStream *stream;
    if(false==video_body->getDefaultAudioTrack( &stream))
    {
        ADM_error("[A_audioSave] No stream\n");
        return 0;
    }
	if (audioProcessMode(0))
	{
		// if we get here, either not compressed
		// or decompressable
		A_saveAudioProcessed(name);
    }
	else			// copy mode...
    {
        A_saveAudioCopy(name);
    }
	return 1;
}

/**
        \fn A_saveAudioCommon
        \brief Save giveb stream
*/
static bool A_saveAudioCommon (const char *name,ADM_audioStream *stream,double duration)
{
  uint32_t written, max;
  uint64_t dts;
  DIA_workingBase *work;
#define ONE_STRIKE (64*1024)
  uint8_t *buffer=NULL;

  ADM_audioWrite *saver=admCreateAudioWriter(stream);
  if(!saver)
  {
    ADM_error("Dont know how to save this\n");
    return false;
  }
  if(false==saver->init(stream,name))
  {
    delete saver;
    ADM_error("Cannot open file for writing\n");
    return false;
  }

  work=createWorking(QT_TR_NOOP("Saving audio"));

  uint64_t timeEnd,timeStart;
  uint32_t hold,len,sample;
  uint64_t tgt_sample,cur_sample;

   duration*=stream->getInfo()->frequency;
   duration/=1000000; // in seconds to have samples
   tgt_sample=(uint64_t)floor(duration);
   printf("[saveAudio]Duration:%f ms\n",duration/1000);
   printf("[saveAudio]Samples:%"PRIu64" ms\n",tgt_sample);

   cur_sample=0;
   written = 0;
   hold=0;
   buffer=new uint8_t[ONE_STRIKE*2];

   while (1)
    {
    	if(!stream->getPacket(buffer+hold,&len,ONE_STRIKE,&sample,&dts)) break;
        hold+=len;
        written+=len;
        cur_sample+=sample;
        if(hold>ONE_STRIKE) // flush
        {
            saver->write(hold,buffer);
            hold=0;
        }
        if(cur_sample>tgt_sample)
            break;
        if(!work->isAlive()) break;
        work->update(cur_sample>>10, tgt_sample>>10);

    };
  if(hold)
  {
  	saver->write(hold,buffer);
	hold=0;
  }
  saver->close();
  delete saver;
  delete work;
  delete[] buffer;
  ADM_info ("\n wanted %"PRIu64" samples, goto %"PRIu64" samples, written %"PRIu32" bytes\n", tgt_sample,cur_sample, written);
  return true;
}

/**
        \fn A_saveAudioCopy
        \brief Save current stream (generally avi...)     in raw mode
*/
int A_saveAudioCopy (const char *name)
{
  uint32_t written, max;
  uint64_t dts;

#define ONE_STRIKE (64*1024)
  ADM_audioStream *stream;
  if(false==video_body->getDefaultAudioTrack( &stream))
    {
        ADM_error("[A_audioSave] No stream\n");
        return false;
    }

  uint64_t timeEnd,timeStart;
  double   duration;

  // compute start position and duration in samples

   timeStart=video_body->getMarkerAPts ();
   timeEnd=video_body->getMarkerBPts ();

   duration=timeEnd-timeStart;
   if(duration<0)
    {
            stream->goToTime (timeEnd);
            duration=-duration;
    }else
    {
            stream->goToTime (timeStart);
    }
   ADM_info("Saving from %s \n",ADM_us2plain(timeStart));
   ADM_info("Saving to %s \n",ADM_us2plain(timeEnd));
   ADM_info("duration %s \n",ADM_us2plain((uint64_t)duration));
   return A_saveAudioCommon (name,stream,duration);

}


/**
    \fn A_saveAudioProcessed
    \brief Save current stream (generally avi...)
     in decoded mode (assuming MP3)
*/
int A_saveAudioProcessed (const char *name)
{
#if 0
    GUI_Error_HIG("Audio","Function not implemented\n");
    return false;
#else

  uint64_t timeEnd,timeStart;
  uint32_t hold,len,sample;
  uint64_t tgt_sample,cur_sample;
  double   duration;
   timeStart=video_body->getMarkerAPts ();
   timeEnd=video_body->getMarkerBPts ();
   uint64_t start=timeStart;
   duration=timeEnd-timeStart;
   if(duration<0)
    {
            start=timeEnd;
            duration=-duration;
    }
  EditableAudioTrack *ed=video_body->getDefaultEditableAudioTrack();
  ADM_audioStream *access=audioCreateEncodingStream(ed,false,start,0);
  if(!access)
    {
        GUI_Error_HIG("Audio","Cannot create stream");
        return false;
    }
  #warning Fixme,duration can change! e.g. pal2film /film2pal
  bool r=A_saveAudioCommon (name,access,duration);
  delete access;
  if(false==r)
        GUI_Error_HIG("Audio","Saving failed");
  return r;
#endif
}

/**
        \fn A_saveJpg
        \brief Save a Jpg image from current display buffer
*/
int A_saveJpg (const char *name)
{
  uint8_t fl;
    ADMImage *image=admPreview::getBuffer();
    if(!image)
    {
        printf("[SaveJpeg] No image\n");
        return false;

    }
    if(!image->saveAsJpg (name))
    {
        GUI_Error_HIG(QT_TR_NOOP("Jpeg"),QT_TR_NOOP("Fail to save as jpeg"));
        return false;
    }
    return true ;
}


/**
      \fn A_saveBunchJpg
      \brief Save the selection  as a bunch of jpeg 95% qual

*/
int A_saveBunchJpg(const char *name)
{
  ADMImage *src=NULL;
  uint32_t curImg;
  char	 fullName[2048],*ext;
  char *baseName;
  DIA_workingBase *working;
  uint8_t success=0;

    ADM_error("Broken\n");
    return 0;
#if 0

        // Split name into base + extension
        ADM_PathSplit(name,&baseName,&ext);

        src=new ADMImageDefault(avifileinfo->width,avifileinfo->height);
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
        return success;
#endif

}
/**
      \fn A_saveImg
      \brief Save current displayed image as a BMP file
*/
int A_saveImg (const char *name)
{
  ADMImage *image=admPreview::getBuffer();
    if(!image)
    {
        ADM_warning("[SaveJpeg] No image\n");
        return 0;

    }
    int r=image->saveAsBmp(name);
    if(!r)
        GUI_Error_HIG (QT_TR_NOOP("BMP op failed"),QT_TR_NOOP( "Saving %s as a BMP file failed."), ADM_GetFileName(name));
    return r;

}

/**
    \fn A_SaveWrapper

*/
int A_SaveWrapper(const char *name)
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
/**
    \fn A_queueJob
    \brief Save current stuff as py script and create the associated job
*/
void A_queueJob(const char *jobName,const char *outputFile)
{
    ADMJob job;
    IScriptEngine *engine=getPythonScriptEngine();
            if(!engine)
            {
                GUI_Error_HIG("Queue","Cannot get tinyPÿ script engine");
                return;
            }

            job.outputFileName=string(outputFile);
            job.jobName=string(jobName);
#warning make sure it is unique
            job.scriptName=string(jobName)+string(".")+engine->defaultFileExtension();
            if(false==ADM_jobAdd(job))
            {
                GUI_Error_HIG("Queue","Cannot add job %s",jobName);
                return;
            }
            string completePath=string(ADM_getJobDir());
            completePath=completePath+string("/")+job.scriptName;
            // Save the script...

            A_saveScript(engine, completePath.c_str());
}
//EOF

