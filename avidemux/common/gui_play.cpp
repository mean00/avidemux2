
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
#include "ADM_edit.hxx"
#include "audio_out.h"
#include "DIA_coreToolkit.h"
#include "gtkgui.h"
#include "ADM_render/GUI_render.h"
#include "ADM_preview.h"
#include "audiofilter.h"
#include "ADM_filterChain.h"
#include "GUI_ui.h"
#include "ADM_coreUtils.h"
#include "ADM_vidMisc.h"

//___________________________________
// In 10 ms chunks
#define AUDIO_PRELOAD 100
#define EVEN(x) (x&0xffffffe)
//___________________________________

static uint8_t stop_req;
static bool exiting;

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
        Clock           ticktock,tocktick;
        bool            refreshCapEnabled;
        uint32_t        refreshCapValue;
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
        uint64_t getLastPts(void) { return lastPts; }
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
    vuMeterPts=0;
    audioLatency=0;
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
void GUI_PlayAvi(bool quit)
{
    exiting = quit;
    // check we got everything...
    if (!video_body->getNbSegment())
        return;
    if (playing)
    {
        stop_req = 1;
        return;
    }

    uint64_t oldTimeFrame,newTimeFrame;
    aviInfo info;
    float oldZoom=admPreview::getCurrentZoom();
    video_body->getVideoInfo(&info);

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
    newTimeFrame=oldTimeFrame+playLoop->getLastPts();
    delete playLoop;
    playing = 0;

    // Don't touch display on application exit
    if(exiting) return;

   admPreview::deferDisplay(false);
   // Resize the output window to original size...
   ADM_info("Restoring display.\n");
   
   admPreview::setMainDimension(info.width,info.height,oldZoom);
    // If we are processing the video, the current time
    // might not be matching a source video time => PROBLEM
    // Go back to the beginning to be on safe ground
    // In copy mode, we can keep the current position
    if(getPreviewMode()!=ADM_PREVIEW_NONE)
        admPreview::seekToTime(oldTimeFrame);
    else
    {
        if(false==admPreview::seekToTime(newTimeFrame))
            admPreview::nextPicture();
    }
    UI_purge();
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
    refreshCapEnabled=false;
    refreshCapValue=0;
    prefs->get(FEATURES_CAP_REFRESH_ENABLED,&refreshCapEnabled);
    prefs->get(FEATURES_CAP_REFRESH_VALUE,&refreshCapValue);

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
    float currentZoom=admPreview::getCurrentZoom();
    float zoom=ZOOM_AUTO;
    // if the video output has same width/height as input, we keep the same zoom
    aviInfo originalVideo;
    video_body->getVideoInfo(&originalVideo);
    if(info->width==originalVideo.width && info->height==originalVideo.height)
        zoom=currentZoom;
    //
    admPreview::setMainDimension(info->width,info->height,zoom);
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
    bool gotAudio=true;
    bool first=true;
    int refreshCounter=0;
    ticktock.reset();
    tocktick.reset();
    vuMeterPts=0;
    ADMImage *previewBuffer=admPreview::getBuffer();
    ADM_HW_IMAGE hwImageFormat=admPreview::getPreferedHwImageFormat();

    do
    {
        if(!first)
            admPreview::displayNow();
        else
            first=false;
        if(refreshCapEnabled)
        {
            if(stop_req || tocktick.getElapsedMS()>refreshCapValue)
            {
                GUI_setCurrentFrameAndTime(firstPts);
                tocktick.reset();
            }
        }else
        {
            GUI_setCurrentFrameAndTime(firstPts);
        }
        lastPts=admPreview::getCurrentPts();
        if(false==videoFilter->getNextFrameAs(hwImageFormat,&fn,previewBuffer))
        {
            printf("[Play] Cancelling playback, nextPicture failed\n");
            break;
        }
        if(gotAudio)
            gotAudio=audioPump(false);
        systemTime = ticktock.getElapsedMS();
        movieTime=(uint32_t)(lastPts/1000);
        movieTime+=audioLatency;
        //printf("[Playback] systemTime: %lu movieTime : %lu audioLatency=%lu \r",systemTime,movieTime,(uint32_t)audioLatency);
        //printf("[LastPTS]=%s \n",ADM_us2plain(lastPts));
        //printf("[firstPts]=%s \n",ADM_us2plain(firstPts));
        
        
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
            delta=(int32_t)movieTime-(int32_t)systemTime;
            // a call to whatever sleep function will last at leat 10 ms
            // give some time to GTK
            while(delta > 10)
            {
                if(stop_req)
                    break;
                
                if(delta>10)
                {
                    if(gotAudio)
                        gotAudio=audioPump(true);
                    else
                        ADM_usleep(10*1000);
                }else
                {
                    if(gotAudio)
                        gotAudio=audioPump(false);
                }

                UI_purge();
                refreshCounter=0;
                systemTime = ticktock.getElapsedMS();
                delta=(int32_t)movieTime-(int32_t)systemTime;
            }
        }
    }
    while (!stop_req);
    return true;
};

/**
    \fn audioPump
    \brief send ~ worth of one video frame of audio
*/

bool  GUIPlayback::audioPump(bool wait)
{
    if (!playbackAudio)
        return false;

    uint32_t oaf = 0;
    uint32_t load = 0;
    uint8_t channels;
    uint32_t fq;
    static bool errorMet=false;

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
                  if(status==AUD_END_OF_STREAM)
                  {
                      uint32_t stat[8]={0};
                      UI_setVUMeter(stat);
                      return false;
                  }
                  if(errorMet==false)
                    ADM_warning("[Playback] Error reading audio stream...\n");
                  errorMet=true;
                  break;
             }
            errorMet=false;
            AVDM_AudioPlay(wavbuf, oaf);
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
    int32_t shift=0; // unit is ms, + => delay audio, -=> advance audio
    
    // if audio shift is activated, take it into account
    //
    EditableAudioTrack *ed=video_body->getDefaultEditableAudioTrack();
    if(!ed)
        return false;
    if(ed->audioEncodingConfig.shiftEnabled)
        shift=ed->audioEncodingConfig.shiftInMs;
    playbackAudio = createPlaybackFilter(startPts,shift);
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

     state = AVDM_AudioSetup(frequency,  channels ,playbackAudio->getChannelMapping());
     latency=AVDM_GetLayencyMs();
     printf("[Playback] Latency : %d ms\n",latency);
     audioLatency=latency; // ms -> us
      if (!state)
      {
          GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Trouble initializing audio device"), NULL);
          cleanupAudio();
          return false;
      }
    UI_setVolume();
    while(fill<preload)
    {
      if (!(small_ = playbackAudio->fill(preload-fill, wavbuf+fill,&status)))
      {
        break;
      }
      fill+=small_;
    }
    // Let audio latency sets in...
    ticktock.reset();
    AVDM_AudioPlay(wavbuf, fill);
    updateVu();
#if 0
    uint32_t slice=(frequency * channels)/100; // 10 ms
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
       updateVu();
    }
    printf("[Playback] Latency is now %u\n",ticktock.getElapsedMS());
#endif
    return true;
}
/**
    \fn updateVu
*/
bool GUIPlayback::updateVu(void)
{
    uint64_t time=ticktock.getElapsedMS();
    // Refresh vumeter at most every 50 ms
    uint64_t wait=50;
    if(refreshCapEnabled && refreshCapValue > wait)
        wait=refreshCapValue;
    if(!playbackAudio)  return true;
    if(time>(vuMeterPts+wait))
    {
        uint32_t stat[8];
        AVDM_getStats(stat);
        UI_setVUMeter(stat);
        vuMeterPts=time;
    }
    return true;
}
// EOF
