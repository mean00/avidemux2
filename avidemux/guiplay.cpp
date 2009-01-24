
/***************************************************************************
    \file  guiplay.cpp
	\brief Playback loop
    
    copyright            : (C) 2001/2008 by mean
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
 
#include <math.h>
#include "prefs.h"
#include "fourcc.h"
#include "avi_vars.h"
#include "ADM_assert.h" 

#include "DIA_fileSel.h"
#include "prototype.h"
#include "audio_out.h"

#include "DIA_coreToolkit.h"
#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "gtkgui.h"
#include "ADM_userInterfaces/ADM_render/GUI_render.h"
#include "ADM_audiofilter/audioeng_buildfilters.h"
#include "avidemutils.h"
#include "ADM_preview.h"
//___________________________________
#define AUDIO_PRELOAD 200
//___________________________________

static void resetTime(void);
static void ADM_playPreloadAudio(void);
static void ADM_playFillAudio(void);
extern void UI_purge(void);
#define EVEN(x) (x&0xffffffe)

//___________________________________
uint8_t stop_req;
static uint32_t auds = 0, nbSamplesSent = 0;


static uint16_t audio_available = 0;
static uint32_t one_audio_frame = 0;
static uint32_t one_frame;
static float *wavbuf = NULL;
AUDMAudioFilter *playback = NULL;
extern renderZoom currentZoom;
static Clock    ticktock;
uint64_t firstPts,lastPts;
/**
    \fn         GUI_PlayAvi
    \brief      MainLoop for internal movie playback

*/
void GUI_PlayAvi(void)
{
    
    uint32_t framelen,flags;
    AVDMGenericVideoStream *filter;
    uint32_t max,err,oldFrame;
   
    // check we got everything...
    if (!avifileinfo)	return;
    if (!avifileinfo->fps1000)        return;
    
    if (playing)
      {
        stop_req = 1;
        return;
      }
    oldFrame=video_body->getCurrentFrame();
	uint32_t priorityLevel;

	originalPriority = getpriority(PRIO_PROCESS, 0);
	prefs->get(PRIORITY_PLAYBACK,&priorityLevel);
	setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));
    
    if(getPreviewMode()==ADM_PREVIEW_OUTPUT)
    {
            filter=getLastVideoFilter();
    }
    else
    {
            filter=getFirstVideoFilter( );
    }
    
    stop_req = 0;
    playing = 1;


    


    admPreview::deferDisplay(1,curframe);
    admPreview::samePicture();

    firstPts=admPreview::getCurrentPts();
    ADM_playPreloadAudio();

    uint32_t movieTime;
    uint32_t systemTime;
    
    ticktock.reset();
    do
    {
        
        admPreview::displayNow();;
        GUI_setCurrentFrameAndTime();
        if(false==admPreview::nextPicture()) break;
        ADM_playFillAudio();
        lastPts=admPreview::getCurrentPts();
        systemTime = ticktock.getElapsedMS();
        movieTime=(uint32_t)((lastPts-firstPts)/1000);
       // printf("[Playback] systemTime: %lu movieTime : %lu  \r",systemTime,movieTime);
        if(systemTime>movieTime) // We are late, the current PTS is after current closk
        {
            if(movieTime>systemTime+20)
                printf("[Playback] We are late!\n");
        }
	    else
	    {
            int32_t delta;
                delta=movieTime-systemTime;
                // a call to whatever sleep function will last at leat 10 ms
                // give some time to GTK                		
                while(delta > 10)
                {
                    if(delta>10)
                    {
                        GUI_Sleep(delta - 10);
                        ADM_playFillAudio();
                    }
                    
                    UI_purge();
                    systemTime = ticktock.getElapsedMS();
                    delta=movieTime-systemTime;                
                }
                
                if(getPreviewMode()==ADM_PREVIEW_SEPARATE )
                {
                  UI_purge();
                  UI_purge(); 
                }
        }
      }
    while (!stop_req);

abort_play:
	// ___________________________________
    // go back to normal display mode
    //____________________________________
       playing = 0;
          
	   getFirstVideoFilter( );

       admPreview::deferDisplay(0,0);
       
       
       UI_purge();
       video_body->setCurrentFrame(oldFrame);
       admPreview::samePicture();
       GUI_setCurrentFrameAndTime();
       UI_purge();
       if (currentaudiostream)
       {
          if (wavbuf)
              ADM_dealloc(wavbuf);
          wavbuf=NULL;
          AVDM_AudioClose();
       }
       // done.
	   setpriority(PRIO_PROCESS, 0, originalPriority);
};

/**
    \fn ADM_playFillAudio
    \brief send ~ worth of one video frame of audio
*/

void ADM_playFillAudio(void)
{
    uint32_t oaf = 0;
    uint32_t load = 0;
	uint8_t channels;
	uint32_t fq;

    if (!audio_available)	    return;
    if (!currentaudiostream)	return;			// audio ?

  
    channels= playback->getInfo()->channels;
    fq=playback->getInfo()->frequency;  
	double db_clock, db_wav,db_sys;
    int32_t delta=0;

    db_clock = admPreview::getCurrentPts()-firstPts;
    db_clock /= 1000000.;  // in seconds

    db_sys=ticktock.getElapsedMS();
    db_sys/=1000;

     while(AVDM_getMsFullness() < AUDIO_PRELOAD)
      {
          //printf("Fullness:%u\n",AVDM_getMsFullness());
          db_wav = nbSamplesSent;	// in seconds also
          db_wav /= fq;

          delta = (long int) floor(1000. * (db_wav - db_clock));
          int deltaSys=( int) floor(1000. * (db_sys - db_clock));

           AUD_Status status;
             if (! (oaf = playback->fill(256*16,  wavbuf,&status)))
             {
                  printf("[Playback] Error reading audio stream...\n");
                  audio_available=0;
                  return;
             }
            AVDM_AudioPlay(wavbuf, oaf);
            nbSamplesSent += oaf/channels;
            load+=oaf;
    }
    //printf("[Playback] Wrote %u bytes\n",load);
    // finally play the filled up buffer
    
}

/**
    \fn ADM_playPreloadAudio
    \brief Preload audio
*/
void ADM_playPreloadAudio(void)

{
    uint32_t state,latency, preload;
    uint32_t small_;
    uint32_t channels;

    wavbuf = 0;

    if (!currentaudiostream)	  return;
    
    double db;
    uint64_t startPts=video_body->getTime(video_body->getCurrentFrame());

    playback = buildPlaybackFilter(currentaudiostream,startPts/1000, 0xffffffff);
    
    channels= playback->getInfo()->channels;
    preload=  (wavinfo->frequency * channels)/5;	// 200 ms preload
    // 4 sec buffer..               
    wavbuf =  (float *)  ADM_alloc((20*sizeof(float)*preload)); // 4 secs buffers
    ADM_assert(wavbuf);
    // Call it twice to be sure it is properly setup
     state = AVDM_AudioSetup(playback->getInfo()->frequency,  channels );
     AVDM_AudioClose();
     state = AVDM_AudioSetup(playback->getInfo()->frequency,  channels );
     latency=AVDM_GetLayencyMs();
     printf("[Playback] Latency : %d ms\n",latency);
      if (!state)
      {
          GUI_Error_HIG(QT_TR_NOOP("Trouble initializing audio device"), NULL);
          return;
      }
     
     AUD_Status status;
    uint32_t fill=0;
    while(fill<preload)
    {
      if (!(small_ = playback->fill(preload-fill, wavbuf+fill,&status)))
      {
        break;
      }
      fill+=small_;
    }
    nbSamplesSent = fill/channels;  // In sample
    AVDM_AudioPlay(wavbuf, fill);
    // Let audio latency sets in...
    ticktock.reset();
    uint32_t slice=(wavinfo->frequency * channels)/100; // 10 ms
    // pump data until latency is over
    while(ticktock.getElapsedMS()<latency)
    {
        if(AVDM_getMsFullness()<AUDIO_PRELOAD)
        {
          if (!(small_ = playback->fill(slice, wavbuf,&status)))
          {
            printf("[Playback] Compensating for latency failed\n");
            break;
          }
          AVDM_AudioPlay(wavbuf, slice);
        }
       ADM_usleep(10*1000);
    }
    printf("[Playback] Latency is now %u\n",ticktock.getElapsedMS());
    audio_available = 1;
}

// EOF
