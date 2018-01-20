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
#include "ADM_filterChain.h"
// Local prototypes
#include "A_functions.h"
#include "ADM_script2/include/ADM_script.h"
#include "ADM_muxerProto.h"

int      A_Save(const char *name);
extern   ADM_audioStream  *audioCreateEncodingStream(EditableAudioTrack *ed,bool globalHeader,uint64_t startTime);

/**
    \fn HandleAction_Navigate

*/
void HandleAction_Save(Action action)
{
    if(!video_body->getNbSegment())
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","No"),QT_TRANSLATE_NOOP("adm","No file loaded"));
        return;
    }
    switch(action)
    {
    case ACT_SAVE_QUEUE:
            {
                if(false==ADMJob::jobInit())
                {
                    GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Job"),QT_TRANSLATE_NOOP("adm","Cannot reach database. Do you have Job control running ?"));
                }else
                {
                    std::string oFile;
                    char *oText=NULL;
                    
                    diaElemFile wFile(1,oFile,QT_TRANSLATE_NOOP("adm","Output file"),"");
                    diaElemText wText(&oText,QT_TRANSLATE_NOOP("adm","Job name"));
                    diaElem *elems[2]={&wText,&wFile};

                    if(  diaFactoryRun(QT_TRANSLATE_NOOP("adm","Queue job to jobList"),2,elems))
                    {
                        A_queueJob(oText,oFile.c_str());
                    }
                    ADMJob::jobShutDown();
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
            GUI_FileSelWrite (QT_TRANSLATE_NOOP("adm","Select Workbench to Save"), A_saveJsProject);
            UI_refreshCustomMenu();
          }
        }
      break;
#endif
    case ACT_SAVE_AUDIO:
      	{
          GUI_FileSelWrite (QT_TRANSLATE_NOOP("adm","Select File to Save Audio"),(SELFILE_CB *)A_audioSave);
        }
      break;

    case ACT_SAVE_BUNCH_OF_JPG:
      GUI_FileSelWrite (QT_TRANSLATE_NOOP("adm","Select JPEG Sequence to Save"), (SELFILE_CB *)A_saveBunchJpg);
    	break;
    case ACT_SAVE_BMP:
    {
      const char *defaultExtension="bmp";
      GUI_FileSelWriteExtension (QT_TRANSLATE_NOOP("adm","Select BMP to Save"),defaultExtension,(SELFILE_CB *)A_saveImg); // A_saveImg);
    }
      break;
    case ACT_SAVE_JPG :
    {
      const char *defaultExtension="jpg";
      GUI_FileSelWriteExtension (QT_TRANSLATE_NOOP("adm","Select JPEG to Save"),defaultExtension,(SELFILE_CB *)A_saveJpg); // A_saveJpg);
    }
      	break;
//----------------------test-----------------------
    case ACT_SAVE_VIDEO:
    {
      if(!ADM_mx_getNbMuxers()) break;
      int  muxerIndex=UI_GetCurrentFormat();
      const char *defaultExtension=ADM_MuxerGetDefaultExtension(muxerIndex);
      GUI_FileSelWriteExtension (QT_TRANSLATE_NOOP("adm","Select File to Save"),defaultExtension,(SELFILE_CB *)A_SaveWrapper); // A_SaveAudioNVideo);
    }
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
  bool buffered=true;
  
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

  work=createWorking(QT_TRANSLATE_NOOP("adm","Saving audio"));

  uint64_t timeEnd,timeStart;
  uint32_t hold,len,sample;
  uint64_t tgt_sample,cur_sample;

   duration*=stream->getInfo()->frequency;
   duration/=1000000; // in seconds to have samples
   tgt_sample=(uint64_t)floor(duration);
   printf("[saveAudio]Duration:%f ms\n",duration/1000);
   printf("[saveAudio]Samples:%" PRIu64" ms\n",tgt_sample);

   cur_sample=0;
   written = 0;
   hold=0;
   buffer=new uint8_t[ONE_STRIKE*2];

   buffered=saver->canBeBuffered();
   
   while (1)
    {
    	if(!stream->getPacket(buffer+hold,&len,ONE_STRIKE,&sample,&dts)) break;
        hold+=len;
        written+=len;
        cur_sample+=sample;
        if(hold>ONE_STRIKE || !buffered) // flush
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
  ADM_info ("\n wanted %" PRIu64" samples, goto %" PRIu64" samples, written %" PRIu32" bytes\n", tgt_sample,cur_sample, written);
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
    GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Audio"),QT_TRANSLATE_NOOP("adm","Function not implemented\n"));
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
  ADM_audioStream *access=audioCreateEncodingStream(ed,false,start);
  if(!access)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Audio"),QT_TRANSLATE_NOOP("adm","Cannot create stream"));
        return false;
    }
//  #warning Fixme,duration can change! e.g. pal2film /film2pal
  bool r=A_saveAudioCommon (name,access,duration);
  delete access;
  if(false==r)
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Audio"),QT_TRANSLATE_NOOP("adm","Saving failed"));
  return r;
#endif
}

/**
        \fn A_saveJpg
        \brief Save a Jpg image from current display buffer
*/
bool A_saveJpg (const char *name)
{
    bool result=true;
    uint64_t current=video_body->getCurrentFramePts();
    uint64_t end=video_body->getVideoDuration();
    ADM_HW_IMAGE hw=admPreview::getPreferedHwImageFormat();
    ADM_videoFilterChain *chain;
    if(getPreviewMode()==ADM_PREVIEW_NONE)
        chain=createEmptyVideoFilterChain(current,end);
    else
        chain=createVideoFilterChain(current,end);
    if(chain && chain->size())
    {
        ADM_coreVideoFilter *filter;
        filter=chain->back();
        FilterInfo *info=filter->getInfo();
        uint32_t width=info->width;
        uint32_t height=info->height;
        ADMImage *image=new ADMImageDefault(width,height);
        uint32_t fn;
        if(!filter->getNextFrameAs(hw,&fn,image))
        {
            ADM_error("No image\n");
            result=false;
        }
        if(result && !image->saveAsJpg (name))
        {
            GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Jpeg"),QT_TRANSLATE_NOOP("adm","Failed to save as JPEG"));
            result=false;
        }
        delete image;
        image=NULL;
    }
    return result;
}


/**
      \fn A_saveBunchJpg
      \brief Save the selection  as a bunch of jpeg 95% qual

*/
int A_saveBunchJpg(const char *name)
{
    int success=0;
    uint64_t pts=admPreview::getCurrentPts();
    uint64_t start=video_body->getMarkerAPts();
    uint64_t end=video_body->getMarkerBPts();
    uint64_t inc=video_body->getFrameIncrement();
    uint64_t original=pts;
    pts=start;

    admPreview::deferDisplay(true);
    admPreview::seekToTime(start);

    ADM_HW_IMAGE hw=admPreview::getPreferedHwImageFormat();
    ADM_videoFilterChain *chain;
    if(getPreviewMode()==ADM_PREVIEW_NONE)
        chain=createEmptyVideoFilterChain(start,end);
    else
        chain=createVideoFilterChain(start,end);
    if(!chain || chain->empty())
    {
        admPreview::seekToTime(original);
        admPreview::deferDisplay(false);
        return 0;
    }

    ADM_coreVideoFilter *filter;
    filter=chain->back();
    FilterInfo *info=filter->getInfo();
    uint32_t width=info->width;
    uint32_t height=info->height;
    ADMImage *src=new ADMImageDefault(width,height);
    uint32_t fn;

    char fullName[2048];
    std::string baseName,ext;
    uint32_t range=(uint32_t)((end-start)/1000);
    DIA_workingBase *working;

    // Split name into base + extension
    ADM_PathSplit(std::string(name),baseName,ext);

    working=createWorking(QT_TRANSLATE_NOOP("adm","Saving selection as set of JPEG images"));
    while(pts<=end)
    {
        uint32_t current=(uint32_t)((pts-start)/1000);
        working->update(current,range);
        if(!filter->getNextFrameAs(hw,&fn,src))
        {
            //GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Cannot decode frame"), QT_TRANSLATE_NOOP("adm","Aborting."));
            break;
        }
        if(src->Pts==ADM_NO_PTS)
            pts+=inc;
        else
            pts=src->Pts;
        success++;
        if(!working->isAlive()) break;
        sprintf(fullName,"%s-%04d.jpg",baseName.c_str(),success);
        if(!src->saveAsJpg(fullName)) break;
        if(success==9999)
        {
            GUI_Info_HIG(ADM_LOG_INFO,QT_TRANSLATE_NOOP("adm","Warning"),QT_TRANSLATE_NOOP("adm","Maximum number of 9999 images reached, aborting."));
            break;
        }
    }

    if(success)
        GUI_Info_HIG(ADM_LOG_INFO,QT_TRANSLATE_NOOP("adm","Done"),QT_TRANSLATE_NOOP("adm","Saved %d images."),success);
    else
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","Saving images failed."));
    delete working;
    working=NULL;
    delete src;
    src=NULL;
    admPreview::seekToTime(original);
    admPreview::deferDisplay(false);
    return success;
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
        ADM_warning("[SaveBmp] No image\n");
        return 0;

    }
    int r=image->saveAsBmp(name);
    if(!r)
        GUI_Error_HIG (QT_TRANSLATE_NOOP("adm","BMP op failed"),QT_TRANSLATE_NOOP("adm", "Saving %s as a BMP file failed."), ADM_getFileName(name).c_str());
    return r;

}

/**
    \fn A_SaveWrapper

*/
int A_SaveWrapper(const char *name)
{

        if(A_Save(name))
        {
          GUI_Info_HIG (ADM_LOG_INFO,QT_TRANSLATE_NOOP("adm","Done"),QT_TRANSLATE_NOOP("adm", "File %s has been successfully saved."),ADM_getFileName(name).c_str());
        }
        else
        {
          GUI_Error_HIG (QT_TRANSLATE_NOOP("adm","Failed"), QT_TRANSLATE_NOOP("adm","File %s was NOT saved correctly."),ADM_getFileName(name).c_str());
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
                GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Queue"),QT_TRANSLATE_NOOP("adm","Cannot get tinyPÿ script engine"));
                return;
            }

            job.outputFileName=string(outputFile);
            job.jobName=string(jobName);
//#warning make sure it is unique
            job.scriptName=string(jobName)+string(".")+engine->defaultFileExtension();
            if(false==ADMJob::jobAdd(job))
            {
                GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Queue"),QT_TRANSLATE_NOOP("adm","Cannot add job %s"),jobName);
                return;
            }
            string completePath=string(ADM_getJobDir());
            completePath=completePath+string("/")+job.scriptName;
            // Save the script...

            A_saveScript(engine, completePath.c_str());
}
//EOF

