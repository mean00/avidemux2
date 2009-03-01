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

#include "ADM_default.h"
#include "avi_vars.h"

#include <math.h>
#include "prototype.h"
#include "gui_action.hxx"
#include "gtkgui.h"

#include "DIA_coreToolkit.h"
#include "ADM_userInterfaces/ADM_commonUI/GUI_ui.h"
#include "DIA_enter.h"

#include "ADM_video/ADM_vidMisc.h"
#include "DIA_fileSel.h"
#include "DIA_working.h"
// Local prototypes
void A_saveAudio (char *name);
int  A_saveJpg (char *name);
void A_saveBunchJpg(const char *name);
void A_saveImg (const char *name);
uint8_t ADM_saveRaw (const char *name);
void A_saveWorkbench (const char *name);
int  A_audioSave(char *name);
int  A_SaveWrapper(char *name);
void A_saveAudioDecodedTest (char *name);
// Xternal prototypes
int      A_SaveUnpackedVop(const char *name);
uint8_t  A_SaveAudioDualAudio(const char *inname);
int      A_Save(const char *name);
uint8_t  GUI_getFrameContent(ADMImage *image, uint32_t frame);

extern char * actual_workbench_file; // UGLY FIXME
/**
    \fn HandleAction_Navigate

*/
void HandleAction_Save(Action action)
{
    switch(action)
    {

    case ACT_SaveWork:
      GUI_FileSelWrite (QT_TR_NOOP("Select Workbench to Save"), A_saveWorkbench);
	  UI_refreshCustomMenu();
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
    case ACT_SaveWave:
      	{
          GUI_FileSelWrite (QT_TR_NOOP("Select File to Save Audio"),(SELFILE_CB *)A_audioSave);
        }
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

/**
    \fn A_saveAudioDecodedTest
    \brief Save current stream (generally avi...)
     in decoded mode (assuming MP3)
*/
void A_saveAudioDecodedTest (char *name)
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
/**
        \fn A_saveAudio
        \brief Save current stream (generally avi...)     in raw mode
*/
void A_saveAudio (char *name)
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

  uint64_t timeEnd,timeStart;
  uint32_t hold,len,sample;
  uint64_t tgt_sample,cur_sample;
  double   duration;

  // compute start position and duration in samples

   timeStart=video_body->estimatePts (frameStart);
   timeEnd=video_body->estimatePts (frameEnd+1);

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

#ifndef TEST_MP2
/**
        \fn A_saveJpg
        \brief Save a Jpg image from current display buffer
*/
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
/**
    \fn A_saveWorkbench
    \brief Save current workbench as ecmascript
*/
void A_saveWorkbench (const char *name)
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

//EOF

