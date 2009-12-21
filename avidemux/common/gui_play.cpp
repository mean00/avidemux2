
/***************************************************************************
    \file  gui_play.cpp
	\brief Playback loop
    
    copyright            : (C) 2001/2009 by mean
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
#include <math.h>
#include "prefs.h"
#include "avi_vars.h"
#include "audio_out.h"
#include "DIA_coreToolkit.h"
#include "gtkgui.h"
#include "ADM_render/GUI_render.h"
#include "avidemutils.h"
#include "ADM_preview.h"
#include "audiofilter.h"
//___________________________________
// In ms
#define AUDIO_PRELOAD 200
#define EVEN(x) (x&0xffffffe)
//___________________________________

extern void UI_purge(void);

//___________________________________
uint8_t stop_req;

extern renderZoom currentZoom;


/**
    \class GUIPlayback
    \brief Wrapper for the playback stuff
*/  
class GUIPlayback
{
private:
        AUDMAudioFilter *playbackAudio;
        Clock           ticktock;
        uint32_t        nbSamplesSent ;        
        float           *wavbuf ;
        uint64_t        firstPts,lastPts;

private : 
        bool initialized;
        bool initializeAudio();
        bool cleanup(void);
        bool audioPump(void);
        bool cleanupAudio(void);
public:
        bool run(void);
        bool initialize(void);
        GUIPlayback();
        ~GUIPlayback();

};
/**
    \fn GUIPlayback
*/
GUIPlayback::GUIPlayback(void)
{
    playbackAudio=NULL;
    wavbuf=NULL;
}
/**
    \fn ~GUIPlayback
*/

GUIPlayback::~GUIPlayback()
{
    cleanup();
}
/**
    \fn         GUI_PlayAvi
    \brief      MainLoop for internal movie playback

*/
void GUI_PlayAvi(void)
{
    
    uint32_t framelen,flags;
    uint32_t max,err;
    uint64_t oldTimeFrame;
   
    // check we got everything...
    if (!avifileinfo)	return;
    if (!avifileinfo->fps1000)        return;
    
    if (playing)
      {
        stop_req = 1;
        return;
      }
    oldTimeFrame=admPreview::getCurrentPts();
	uint32_t priorityLevel;

	originalPriority = getpriority(PRIO_PROCESS, 0);
	prefs->get(PRIORITY_PLAYBACK,&priorityLevel);
	setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));
    
    if(getPreviewMode()==ADM_PREVIEW_OUTPUT)
    {
//            filter=getLastVideoFilter();
    }
    else
    {
  //          filter=getFirstVideoFilter( );
    }
    
    stop_req = 0;
    playing = 1;

    admPreview::deferDisplay(1,curframe);
    admPreview::samePicture();

    GUIPlayback *playLoop=new GUIPlayback;
    playLoop->initialize();
    playLoop->run();
    
    delete playLoop;
   playing = 0;
            
//   getFirstVideoFilter( );

   admPreview::deferDisplay(0,0);
   
   
   UI_purge();
#warning FIXME
//   admPreview::seekToFrame(oldTimeFrame);
   admPreview::samePicture();
   GUI_setCurrentFrameAndTime();
   UI_purge();
      

}
/**
    \fn cleanupAudio
*/
bool GUIPlayback::cleanupAudio(void)
{
      if (wavbuf)
              ADM_dealloc(wavbuf);
          wavbuf=NULL;
    
      AVDM_AudioClose();
      
      if (playbackAudio)
       {
            destroyPlaybackFilter();
            playbackAudio=NULL;
       }
       return true;

}
/**
    \fn cleanupAudio
*/
bool GUIPlayback::cleanup(void)
{
        cleanupAudio();
       // done.
	   setpriority(PRIO_PROCESS, 0, originalPriority);
       return true;

}

/**
        \fn initialize
*/
bool GUIPlayback::initialize(void)
{
    firstPts=admPreview::getCurrentPts();
    initializeAudio();
    return true;
}
/**
        \fn run
*/

bool GUIPlayback::run(void)
{
   
    uint32_t movieTime;
    uint32_t systemTime;
    int refreshCounter=0;
    ticktock.reset();
    do
    {
        
        admPreview::displayNow();;
        GUI_setCurrentFrameAndTime();
        if(false==admPreview::nextPicture()) 
        {
            printf("[Play] Cancelling playback, nextPicture failed\n");
            break;
        }
        audioPump();
        lastPts=admPreview::getCurrentPts();
        systemTime = ticktock.getElapsedMS();
        movieTime=(uint32_t)((lastPts-firstPts)/1000);
       // printf("[Playback] systemTime: %lu movieTime : %lu  \r",systemTime,movieTime);
        if(systemTime>movieTime) // We are late, the current PTS is after current closk
        {
            if(systemTime >movieTime+20)
            {
                refreshCounter++;
            }
            if(refreshCounter>15)
            {
                UI_purge();
                UI_purge();
                refreshCounter=0;
            }
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
                        GUI_Sleep(10);
                        audioPump();
                    }
                    
                    UI_purge();
                    refreshCounter=0;
                    systemTime = ticktock.getElapsedMS();
                    delta=movieTime-systemTime;                
                }
                
                if(getPreviewMode()==ADM_PREVIEW_SEPARATE )
                {
                  UI_purge();
                  UI_purge(); 
                  refreshCounter=0;
                }
        }
      }
    while (!stop_req);

abort_play:
        return true;
};

/**
    \fn audioPump
    \brief send ~ worth of one video frame of audio
*/

bool  GUIPlayback::audioPump(void)
{
    uint32_t oaf = 0;
    uint32_t load = 0;
	uint8_t channels;
	uint32_t fq;

    if (!playbackAudio)	    return false;

  
    channels= playbackAudio->getInfo()->channels;
    fq=playbackAudio->getInfo()->frequency;  


     while(AVDM_getMsFullness() < AUDIO_PRELOAD)
      {

             AUD_Status status;
             if (! (oaf = playbackAudio->fill(256*16,  wavbuf,&status)))
             {
                  printf("[Playback] Error reading audio stream...\n");
                  
                  return false;
             }
            AVDM_AudioPlay(wavbuf, oaf);
            nbSamplesSent += oaf/channels;
            load+=oaf;
    }
    //printf("[Playback] Wrote %u bytes\n",load);
    // finally play the filled up buffer
    return true;
}

/**
    \fn initializeAudio
    \brief Initialize audio
*/
bool  GUIPlayback::initializeAudio(void)

{
    uint32_t state,latency, preload;
    uint32_t small_;
    uint32_t channels;

    wavbuf = 0;

//    if (!currentaudiostream)	  return;
    
    double db;
    uint64_t startPts=firstPts;

    playbackAudio = createPlaybackFilter(startPts,0);
    if(!playbackAudio) return false;

    channels= playbackAudio->getInfo()->channels;
    preload=  (wavinfo->frequency * channels)/5;	// 200 ms preload
    // 4 sec buffer..               
    wavbuf =  (float *)  ADM_alloc((20*sizeof(float)*preload)); // 4 secs buffers
    ADM_assert(wavbuf);
    // Call it twice to be sure it is properly setup
     state = AVDM_AudioSetup(playbackAudio->getInfo()->frequency,  channels );
     AVDM_AudioClose();
     state = AVDM_AudioSetup(playbackAudio->getInfo()->frequency,  channels );
     latency=AVDM_GetLayencyMs();
     printf("[Playback] Latency : %d ms\n",latency);
      if (!state)
      {
          GUI_Error_HIG(QT_TR_NOOP("Trouble initializing audio device"), NULL);
          cleanupAudio();
          return false;
      }
     
     AUD_Status status;
    uint32_t fill=0;
    while(fill<preload)
    {
      if (!(small_ = playbackAudio->fill(preload-fill, wavbuf+fill,&status)))
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
          if (!(small_ = playbackAudio->fill(slice, wavbuf,&status)))
          {
            printf("[Playback] Compensating for latency failed\n");
            break;
          }
          AVDM_AudioPlay(wavbuf, slice);
        }
       ADM_usleep(10*1000);
    }
    printf("[Playback] Latency is now %u\n",ticktock.getElapsedMS());
    return true;
}

// EOF
