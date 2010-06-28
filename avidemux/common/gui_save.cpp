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
// Local prototypes
#include "A_functions.h"
int      A_Save(const char *name);
uint8_t  GUI_getFrameContent(ADMImage *image, uint32_t frame);

/**
    \fn HandleAction_Navigate

*/
void HandleAction_Save(Action action)
{
    switch(action)
    {
    case ACT_SavePyWork:
            GUI_FileSelWrite (QT_TR_NOOP("Select pyProject to Save"), A_savePyProject);
            UI_refreshCustomMenu();
            break;
    case ACT_SaveWork:
      GUI_FileSelWrite (QT_TR_NOOP("Select Project to Save"), A_saveJsProject);
	  UI_refreshCustomMenu();
      break;
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
      
    case ACT_SaveRaw:
        GUI_Error_HIG (QT_TR_NOOP("File error"), QT_TR_NOOP("Deprecated function."));        
      break;
    case ACT_SaveWave:
      	{
          GUI_FileSelWrite (QT_TR_NOOP("Select File to Save Audio"),(SELFILE_CB *)A_audioSave);
        }
      break;

    case ACT_SaveBunchJPG:
      GUI_FileSelWrite (QT_TR_NOOP("Select JPEG Sequence to Save"), (SELFILE_CB *)A_saveBunchJpg);
    	break;
    case ACT_SaveImg:
      GUI_FileSelWrite (QT_TR_NOOP("Select BMP to Save"), (SELFILE_CB *)A_saveImg);
      //GUI_FileSelWrite ("Select Jpg to save ", A_saveJpg);
      break;
    case ACT_SaveJPG :
      GUI_FileSelWrite (QT_TR_NOOP("Select JPEG to Save"), (SELFILE_CB *)A_saveJpg);
      	//GUI_FileSelWrite ("Select Jpg to save ", A_saveJpg);
      	break;
//----------------------test-----------------------
    case ACT_SaveAvi:
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
    if(false==video_body->getAudioStream( &stream)) 
    {
        printf("[A_audioSave] No stream\n");
        return 0;
    }
	if (audioProcessMode())
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
    \fn A_saveAudioProcessed
    \brief Save current stream (generally avi...)
     in decoded mode (assuming MP3)
*/
int A_saveAudioProcessed (const char *name)
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


  if (!(out = ADM_fopen (name, "wb")))
    {
      GUI_Error_HIG (QT_TR_NOOP("File error"), QT_TR_NOOP("Cannot open \"%s\" for writing."), name);
      return false;
    }

  outbuffer = (uint8_t *) ADM_alloc (2 * OUTCHUNK);	// 1Meg cache;
  if (!outbuffer)
    {
      GUI_Error_HIG (QT_TR_NOOP("Memory Error"), NULL);
      return false;
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
			return false;
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
#else
  return 0;
#endif
}
/**
        \fn A_saveAudioCopy
        \brief Save current stream (generally avi...)     in raw mode
*/
int A_saveAudioCopy (const char *name)
{ 
  uint32_t written, max;
  uint64_t dts;
  DIA_workingBase *work;
  FILE *out;

#define ONE_STRIKE (64*1024)
  uint8_t *buffer=NULL;
  ADM_audioStream *stream;
  if(false==video_body->getAudioStream( &stream)) 
    {
        printf("[A_audioSave] No stream\n");
        return false;
    }


  out = ADM_fopen (name, "wb");
  if (!out) return false;

  work=createWorking(QT_TR_NOOP("Saving audio"));

  uint64_t timeEnd,timeStart;
  uint32_t hold,len,sample;
  uint64_t tgt_sample,cur_sample;
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
   duration*=stream->getInfo()->frequency;
   duration/=1000000; // in seconds to have samples
   tgt_sample=(uint64_t)floor(duration);
   printf("[saveAudio] Start time :%"LLU" ms\n",timeStart/1000);
   printf("[saveAudio] End time :%"LLU" ms\n",timeEnd/1000);
   printf("[saveAudio]Duration:%f ms\n",duration/1000);
   printf("[saveAudio]Samples:%"LLU" ms\n",tgt_sample);

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
            fwrite(buffer,hold,1,out);
            hold=0;
        }
        if(cur_sample>tgt_sample)
            break;
        if(!work->isAlive()) break;
        work->update(cur_sample>>10, tgt_sample>>10);
        
    };
  if(hold)
  {
  	fwrite(buffer,hold,1,out);
	hold=0;
  }
  fclose (out);
  delete work;
  delete[] buffer;
  ADM_info ("\n wanted %"LLU" samples, goto %"LLU" samples, written %"LU" bytes\n", tgt_sample,cur_sample, written);
  return true;
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

        if(frameStart>frameEnd)
                {
                  GUI_Error_HIG(QT_TR_NOOP("Mark A > B"), QT_TR_NOOP("Set your markers correctly."));
                        return 0;
                }
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
    \fn A_saveWorkbench
    \brief Save current workbench as ecmascript
*/
void A_saveJsProject (const char *name)
{
  video_body->saveAsScript(name,NULL);
  video_body->setProjectName(name);
}

/**
    \fn A_savePyProject
    \brief Save workbench as pyscript
*/
void A_savePyProject (const char *name)
{
  video_body->saveAsPyScript(name);
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

//EOF

