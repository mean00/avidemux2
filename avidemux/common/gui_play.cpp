
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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include <math.h>
#include "prefs.h"
#include "avi_vars.h"
#include "ADM_Video.h"
#include "ADM_editor/ADM_edit.hxx"
#include "audio_out.h"
#include "DIA_coreToolkit.h"
#include "gtkgui.h"
#include "ADM_render/GUI_render.h"
#include "avidemutils.h"
#include "ADM_preview.h"
#include "audiofilter.h"
#include "ADM_filterChain.h"
#include "GUI_ui.h"

//___________________________________
// In 10 ms chunks
#define AUDIO_PRELOAD 100
#define EVEN(x) (x&0xffffffe)
//___________________________________

extern void UI_purge(void);

//___________________________________
uint8_t stop_req;

extern ADM_Composer *video_body;
static uint32_t originalPriority;
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
        uint64_t        vuMeterPts;
        uint64_t        audioLatency;
        ADM_coreVideoFilter  *videoFilter;
        ADM_videoFilterChain *videoChain;

private :
        bool initialized;
        bool initializeAudio();
        bool cleanup(void);
        bool audioPump(bool wait);
        bool cleanupAudio(void);
        bool updateVu(void);
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
    videoFilter=NULL;
    videoChain=NULL;
    initialized=false;
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
    aviInfo info;
    renderZoom oldZoom=admPreview::getCurrentZoom();
    video_body->getVideoInfo(&info);

    // check we got everything...
    if (!video_body->getNbSegment())	return;
    if (playing)
      {
        stop_req = 1;
        return;
      }
    oldTimeFrame=admPreview::getCurrentPts();
	uint32_t priorityLevel;

	#ifndef __HAIKU__
	originalPriority = getpriority(PRIO_PROCESS, 0);
	prefs->get(PRIORITY_PLAYBACK,&priorityLevel);
	setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));
	#endif

    stop_req = 0;
    playing = 1;

    admPreview::deferDisplay(true);
    //admPreview::samePicture();

    GUIPlayback *playLoop=new GUIPlayback;
    playLoop->initialize();
    playLoop->run();

    delete playLoop;
    playing = 0;

   admPreview::deferDisplay(false);
   // Resize the output window to original size...
   ADM_info("Restoring display.\n");
   
   admPreview::setMainDimension(info.width,info.height,oldZoom);
   admPreview::seekToTime(oldTimeFrame);
   UI_purge();
   
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
        ADM_info("Cleaning up..\n");
        cleanupAudio();
        if(videoChain)
        {
            ADM_info("Destroying video playback chain\n");
            destroyVideoFilterChain(videoChain);
            videoChain=NULL;
        }
        videoFilter=NULL; // it has been destroyed by the chain
       // done.
	   #ifndef __HAIKU__
	   setpriority(PRIO_PROCESS, 0, originalPriority);
	   #endif
       return true;
}

/**
        \fn initialize
*/
bool GUIPlayback::initialize(void)
{
    firstPts=admPreview::getCurrentPts();
    if(getPreviewMode()==ADM_PREVIEW_NONE) // copy
    {
        videoChain=createEmptyVideoFilterChain(firstPts,1000*1000*1000*1000LL); 
    }else
    {
        videoChain=createVideoFilterChain(firstPts,1000*1000*1000*1000LL); 
    }
    if(!videoChain)
    {
        ADM_warning("Cannot create video chain\n");
        return false;
    }
    int nb=videoChain->size();
    videoFilter=(*videoChain)[nb-1];
    FilterInfo *info=videoFilter->getInfo();
    admPreview::setMainDimension(info->width,info->height,ZOOM_AUTO);

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
    uint32_t fn;
    int refreshCounter=0;
    ticktock.reset();
    vuMeterPts=0;
    ADMImage *previewBuffer=admPreview::getBuffer();
    ADM_HW_IMAGE hwImageFormat=admPreview::getPreferedHwImageFormat();

    do
    {

        admPreview::displayNow();;
        GUI_setCurrentFrameAndTime(firstPts);
        if(false==videoFilter->getNextFrameAs(hwImageFormat,&fn,previewBuffer))
        {
            printf("[Play] Cancelling playback, nextPicture failed\n");
            break;
        }
        audioPump(false);
        lastPts=admPreview::getCurrentPts();
        systemTime = ticktock.getElapsedMS();
        movieTime=(uint32_t)((lastPts-firstPts*0)/1000);
        movieTime+=audioLatency;
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
                        audioPump(true);
                    }else
                        audioPump(false);

                    UI_purge();
                    refreshCounter=0;
                    systemTime = ticktock.getElapsedMS();
                    delta=movieTime-systemTime;
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

bool  GUIPlayback::audioPump(bool wait)
{
    uint32_t oaf = 0;
    uint32_t load = 0;
	uint8_t channels;
	uint32_t fq;
    static bool errorMet=false;

    if (!playbackAudio)	    return false;


    channels= playbackAudio->getInfo()->channels;
    fq=playbackAudio->getInfo()->frequency;
    uint32_t slice=(fq * channels)/50; // 20 ms
    if(AVDM_getMsFullness() >= AUDIO_PRELOAD)
    {
        if(wait==true) GUI_Sleep(10);
        updateVu();
        return true;
    }

     while(AVDM_getMsFullness() < AUDIO_PRELOAD)
      {

             AUD_Status status;
             if (! (oaf = playbackAudio->fill(slice,  wavbuf,&status)))
             {
                  if(errorMet==false)
                    ADM_warning("[Playback] Error reading audio stream...\n");
                  errorMet=true;
                  break;
             }
            errorMet=false;
            AVDM_AudioPlay(wavbuf, oaf);
            nbSamplesSent += oaf/channels;
            load+=oaf;
    }
    updateVu();
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
    uint32_t channels,frequency;

    wavbuf = 0;
    uint64_t startPts=firstPts;

    playbackAudio = createPlaybackFilter(startPts,0);
    if(!playbackAudio) 
    {
        ADM_info("No audio\n");
        return false;
    }

    channels= playbackAudio->getInfo()->channels;
    frequency=playbackAudio->getInfo()->frequency;
    preload=  (frequency * channels)/5;	// 200 ms preload
    // 4 sec buffer..
    wavbuf =  (float *)  ADM_alloc((20*sizeof(float)*preload)); // 4 secs buffers
    ADM_assert(wavbuf);
    // Read a at least one block to have the proper channel mapping
    uint32_t fill=0;
    AUD_Status status;
    small_ = playbackAudio->fill(channels, wavbuf,&status);
    fill+=small_;
    // Call it twice to be sure it is properly setup
     state = AVDM_AudioSetup(frequency,  channels ,playbackAudio->getChannelMapping());
     AVDM_AudioClose();
     state = AVDM_AudioSetup(frequency,  channels ,playbackAudio->getChannelMapping());
     latency=AVDM_GetLayencyMs();
     printf("[Playback] Latency : %d ms\n",latency);
     audioLatency=latency; // ms -> us
      if (!state)
      {
          GUI_Error_HIG(QT_TR_NOOP("Trouble initializing audio device"), NULL);
          cleanupAudio();
          return false;
      }
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
    uint32_t slice=(frequency * channels)/100; // 10 ms
    // pump data until latency is over
    updateVu();
    #if 0
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
       updateVu();
    }
    #endif
    printf("[Playback] Latency is now %u\n",ticktock.getElapsedMS());
    return true;
}
/**
    \fn updateVu
*/
bool GUIPlayback::updateVu(void)
{
 uint64_t time=  ticktock.getElapsedMS();
    // Refresh vumeter every 50 ms
    if(!playbackAudio)  return true;
    if(time>(vuMeterPts+50))
    {
        uint32_t stat[6];
        AVDM_getStats(stat);
        UI_setVUMeter(stat);
        vuMeterPts=time;
    }
    return true;
}
// EOF
