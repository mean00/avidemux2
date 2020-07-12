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
    \fn audioSavePrepare
    \brief Check whether we have an audio track to save and get the default file name extension when possible
*/
static bool audioSavePrepare(std::string *audioFileExtension)
{
    ADM_audioStream *stream;
    if(false==video_body->getDefaultAudioTrack(&stream))
    {
        ADM_error("Cannot get the default audio track.\n");
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Audio"),QT_TRANSLATE_NOOP("adm","No audio track"));
        return false;
    }
    uint32_t type;
    if(audioProcessMode(0))
    {
        EditableAudioTrack *ed=video_body->getDefaultEditableAudioTrack();
        if(ed->encoderIndex >= ListOfAudioEncoder.size())
        {
            ADM_error("Illegal encoder index: %d\n",ed->encoderIndex);
            GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Audio"),QT_TRANSLATE_NOOP("adm","Audio encoder index out of bounds"));
            return false;
        }
        type=ListOfAudioEncoder[ed->encoderIndex]->wavTag;
    }else
    {
        type=stream->getInfo()->encoding;
    }
    switch(type)
    {
        case WAV_PCM:
        case WAV_LPCM:
            *audioFileExtension="wav";
            break;
        case WAV_MP3:
            *audioFileExtension="mp3";
            break;
        case WAV_MP2:
            *audioFileExtension="mp2";
            break;
        case WAV_AC3:
        case WAV_EAC3:
            *audioFileExtension="ac3";
            break;
        case WAV_AAC:
        case WAV_AAC_HE:
            *audioFileExtension="aac";
            break;
        case WAV_DTS:
            *audioFileExtension="dts";
            break;
        default: break;
    };
    return true;
}

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
                if(!ADM_mx_getNbMuxers()) break;
                if(false==ADMJob::jobInit())
                {
                    GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Job"),QT_TRANSLATE_NOOP("adm","Cannot reach database. Do you have Job control running ?"));
                }else
                {
                    std::string oFile;
                    std::string prefilled=std::string("Job ")+ADM_getTimeDateAsString();
                    const char *defaultExtension=ADM_MuxerGetDefaultExtension(UI_GetCurrentFormat());
                    char *oText=ADM_strdup(prefilled.c_str());
                    diaElemFile wFile(1,oFile,QT_TRANSLATE_NOOP("adm","Output file"),defaultExtension,NULL);
                    diaElemText wText(&oText,QT_TRANSLATE_NOOP("adm","Job name"));
                    diaElem *elems[2]={&wText,&wFile};

                    if(  diaFactoryRun(QT_TRANSLATE_NOOP("adm","Queue job to jobList"),2,elems))
                    {
                        A_queueJob(oText,oFile.c_str());
                    }
                    ADM_dealloc(oText);
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
        std::string ext;
        if(false==audioSavePrepare(&ext))
            break;
        if(ext.size())
            GUI_FileSelWriteExtension (QT_TRANSLATE_NOOP("adm","Select File to Save Audio"),ext.c_str(),(SELFILE_CB *)A_audioSave);
        else
            GUI_FileSelWrite (QT_TRANSLATE_NOOP("adm","Select File to Save Audio"),(SELFILE_CB *)A_audioSave);
    }
    break;

    case ACT_SAVE_BUNCH_OF_JPG:
    {
      const char *defaultExtension="jpg";
      GUI_FileSelWriteExtension (QT_TRANSLATE_NOOP("adm","Select JPEG Sequence to Save"),defaultExtension,(SELFILE_CB *)A_saveBunchJpg);
    }
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
    case ACT_SAVE_PNG:
    {
        const char *defaultExtension="png";
        GUI_FileSelWriteExtension (QT_TRANSLATE_NOOP("adm","Select PNG to Save"),defaultExtension,(SELFILE_CB *)A_savePng);
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
  uint32_t written;
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

*/
static ADMImage *getCurrentFilteredImage(void)
{
    uint64_t current=admPreview::getCurrentPts();
    // resync last sent frame & stuff
    admPreview::deferDisplay(true);
    video_body->rewind();
    admPreview::seekToTime(current);
    admPreview::deferDisplay(false);

    uint64_t end=video_body->getVideoDuration();
    ADM_HW_IMAGE hw=admPreview::getPreferedHwImageFormat();
    ADM_videoFilterChain *chain;
    chain=NULL;
    ADM_coreVideoFilter *filter;
    filter=NULL;

    chain=createVideoFilterChain(current,end);
    if(!chain)
    {
        ADM_error("Cannot create video filter chain\n");
        return NULL;
    }

    filter=chain->back();
    FilterInfo *info=filter->getInfo();
    uint32_t width=info->width;
    uint32_t height=info->height;
    ADMImage *image=new ADMImageDefault(width,height);
    if(!image)
    {
        ADM_error("No buffer\n");
        destroyVideoFilterChain(chain);
        chain=NULL;
        filter=NULL;
        return NULL;
    }

    uint32_t fn;
    if(!filter->getNextFrameAs(hw,&fn,image))
    {
        ADM_error("No image\n");
        delete image;
        image=NULL;
    }

    destroyVideoFilterChain(chain);
    chain=NULL;
    filter=NULL;

    // resync last sent frame & stuff again
    admPreview::deferDisplay(true);
    video_body->rewind();
    admPreview::seekToTime(current);
    admPreview::deferDisplay(false);

    return image;
}

/**
        \fn A_saveJpg
        \brief Save a Jpg image from current display buffer
*/
bool A_saveJpg (const char *name)
{
    bool result=true;
    bool fromDisplayBuffer=false;
    if(getPreviewMode()==ADM_PREVIEW_NONE)
        fromDisplayBuffer=true;
    ADMImage *image;
    image=NULL;
    if(fromDisplayBuffer)
        image=admPreview::getBuffer();
    else
        image=getCurrentFilteredImage();
    if(!image || !image->saveAsJpg(name))
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Jpeg"),QT_TRANSLATE_NOOP("adm","Failed to save as JPEG"));
        result=false;
    }
    if(!fromDisplayBuffer)
    {
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
#if defined(__APPLE__)
 #define MAX_LEN 1024
#else
 #define MAX_LEN 4096
#endif

    char fullName[MAX_LEN];
    std::string baseName,ext;

    // Split name into base + extension
    ADM_PathSplit(std::string(name),baseName,ext);

    int check=strlen(baseName.c_str());
    check+=11; // '-'+'\d{5}'+'\.'+'jpg'+'\0'
    if(check>MAX_LEN)
    {
        ADM_error("Full path = %d is too long, aborting.\n",check);
        return 0;
    }

    int success=0;
    uint64_t original=admPreview::getCurrentPts();
    uint64_t start=video_body->getMarkerAPts();
    uint64_t end=video_body->getMarkerBPts();
    if(start>end)
    {
        uint64_t swap=end;
        end=start;
        start=swap;
    }
    uint64_t inc=video_body->getFrameIncrement();
    uint64_t pts=0;
    ADM_HW_IMAGE hw=admPreview::getPreferedHwImageFormat();

    ADM_videoFilterChain *chain;
    if(getPreviewMode()==ADM_PREVIEW_NONE)
        chain=createEmptyVideoFilterChain(start,end);
    else
        chain=createVideoFilterChain(start,end);
    if(!chain)
    {
        ADM_error("Cannot create video filter chain\n");
        return 0;
    }

    ADM_coreVideoFilter *filter;
    filter=chain->back();
    FilterInfo *info=filter->getInfo();
    uint32_t width=info->width;
    uint32_t height=info->height;
    ADMImage *src=new ADMImageDefault(width,height);
    if(!src)
    {
        ADM_error("No buffer\n");
        destroyVideoFilterChain(chain);
        chain=NULL;
        filter=NULL;
        return 0;
    }

    admPreview::deferDisplay(true);
    admPreview::seekToTime(start);


    uint32_t range=(uint32_t)((end-start)/1000);
    uint32_t fn;
    DIA_workingBase *working;

    working=createWorking(QT_TRANSLATE_NOOP("adm","Saving selection as set of JPEG images"));
    while(true)
    {
        if(!filter->getNextFrameAs(hw,&fn,src))
        {
            //GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Cannot decode frame"), QT_TRANSLATE_NOOP("adm","Aborting."));
            break;
        }
        if(src->Pts==ADM_NO_PTS)
            pts+=inc;
        else
            pts=src->Pts;
        pts/=1000;
        working->update((uint32_t)pts,range);
        success++;
        if(!working->isAlive()) break;
        sprintf(fullName,"%s-%05d.jpg",baseName.c_str(),success);
        if(!src->saveAsJpg(fullName)) break;
        if(success==99999) break;
    }

    delete working;
    working=NULL;
    delete src;
    src=NULL;
    destroyVideoFilterChain(chain);
    chain=NULL;
    filter=NULL;

    if(success==99999)
        GUI_Info_HIG(ADM_LOG_INFO,QT_TRANSLATE_NOOP("adm","Warning"),QT_TRANSLATE_NOOP("adm","Maximum number of 99999 images reached."));
    else if(success)
        GUI_Info_HIG(ADM_LOG_INFO,QT_TRANSLATE_NOOP("adm","Done"),QT_TRANSLATE_NOOP("adm","Saved %d images."),success);
    else
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","Saving images failed."));

    admPreview::seekToTime(original);
    admPreview::deferDisplay(false);
    return success;
}

/**
    \fn A_savePng
    \brief Save a PNG image from current display buffer or from filter chain
*/
bool A_savePng (const char *name)
{
    bool result=true;
    bool fromDisplayBuffer=false;
    if(getPreviewMode()==ADM_PREVIEW_NONE)
        fromDisplayBuffer=true;
    ADMImage *image;
    image=NULL;
    if(fromDisplayBuffer)
        image=admPreview::getBuffer();
    else
        image=getCurrentFilteredImage();
    if(!image || !image->saveAsPng(name))
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","PNG"),QT_TRANSLATE_NOOP("adm","Failed to save as PNG"));
        result=false;
    }
    if(!fromDisplayBuffer)
    {
        delete image;
        image=NULL;
    }
    return result;
}

/**
      \fn A_saveImg
      \brief Save current displayed image as a BMP file
*/
bool A_saveImg (const char *name)
{
    bool result=true;
    bool fromDisplayBuffer=false;
    if(getPreviewMode()==ADM_PREVIEW_NONE)
        fromDisplayBuffer=true;
    ADMImage *image;
    image=NULL;
    if(fromDisplayBuffer)
        image=admPreview::getBuffer();
    else
        image=getCurrentFilteredImage();
    if(!image || !image->saveAsBmp(name))
    {
        GUI_Error_HIG (QT_TRANSLATE_NOOP("adm","BMP op failed"),QT_TRANSLATE_NOOP("adm", "Saving %s as a BMP file failed."), ADM_getFileName(name).c_str());
        result=false;
    }
    if(!fromDisplayBuffer)
    {
        delete image;
        image=NULL;
    }
    return result;
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
                GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","Cannot get tinyPy script engine"));
                return;
            }

            job.outputFileName=string(outputFile);
            if(job.outputFileName.empty())
            {
                GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","Output file not specified"));
                return;
            }
            job.jobName=string(jobName);
//#warning make sure it is unique
            job.scriptName=string(jobName)+string(".")+engine->defaultFileExtension();
            string completePath=ADM_getJobDir();
            completePath+=job.scriptName;
            bool collision=false;
            if(ADM_fileExist(completePath.c_str()))
            {
                char str[4096+512+1];
                str[0]='\0';
                snprintf(str,4096+512+1,QT_TRANSLATE_NOOP("adm","Job script %s already exists. Overwrite?"),completePath.c_str());
                str[4096+512]='\0';
                if(false==GUI_Question(str))
                    collision=true;
            }
            if(collision || false==ADMJob::jobAdd(job))
            {
                GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","Cannot add job %s"),jobName);
                return;
            }
            // Save the script...

            A_saveScript(engine, completePath.c_str());
}
//EOF

