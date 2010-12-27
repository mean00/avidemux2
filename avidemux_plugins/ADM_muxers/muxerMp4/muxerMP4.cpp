/***************************************************************************
            \file            muxerMP4
            \brief           i/f to lavformat mpeg4 muxer
                             -------------------
    
    copyright            : (C) 2008 by mean
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
#include "fourcc.h"
#include "muxerMP4.h"
#include "DIA_coreToolkit.h"
#include "ADM_muxerUtils.h"

#if 0
#define aprintf(...) {}
#else
#define aprintf printf
#endif

mp4_muxer muxerConfig=
{
    MP4_MUXER_MP4,
    true
};


/**
    \fn     muxerMP4
    \brief  Constructor
*/
muxerMP4::muxerMP4() 
{
};
/**
    \fn     muxerMP4
    \brief  Destructor
*/

muxerMP4::~muxerMP4() 
{
   
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerMP4::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
    if(!isMpeg4Compatible(s->getFCC()) && !isH264Compatible(s->getFCC()))
    {
            GUI_Error_HIG("Unsupported","Only MP4Video & H264 supported for video");
            return false;
    }
    if(nbAudioTrack)
        for(int i=0;i<nbAudioTrack;i++)
        {
            uint32_t acc=a[i]->getInfo()->encoding;
            if(acc!=WAV_MP2 && acc!=WAV_MP3 && acc!=WAV_AAC)
            {
                GUI_Error_HIG("Unsupported","Only AAC & mpegaudio supported for audio");
                return false;
            }
        }
    /* All seems fine, open stuff */
    const char *f="mp4";
    if(muxerConfig.muxerType==MP4_MUXER_PSP) f="psp";
    if(false==setupMuxer(f,file))
    {
        printf("[MP4] Failed to open muxer\n");
        return false;
    }
 
   if(initVideo(s)==false) 
    {
        printf("[MP4] Failed to init video\n");
        return false;
    }
  
    
        AVCodecContext *c;
        c = video_st->codec;
        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        c->gop_size=15;
        
        if(initAudio(nbAudioTrack,a)==false)
        {
            printf("[MP4] Failed to init audio\n");
            return false;
        }
        
        // /audio
        oc->mux_rate=10080*1000;
        oc->preload=AV_TIME_BASE/10; // 100 ms preloading
        oc->max_delay=200*1000; // 500 ms
        if (av_set_parameters(oc, NULL) < 0)
        {
            printf("Lav: set param failed \n");
            return false;
        }
        if (url_fopen(&(oc->pb), file, URL_WRONLY) < 0)
        {
            printf("[MP4]: Failed to open file :%s\n",file);
            return false;
        }

        ADM_assert(av_write_header(oc)>=0);
        vStream=s;
        aStreams=a;
        nbAStreams=nbAudioTrack;
        initialized=true;
        return true;
}

/**
    \fn save
*/
bool muxerMP4::save(void) 
{
    const char *title=QT_TR_NOOP("Saving mp4");
    if(muxerConfig.muxerType==MP4_MUXER_PSP) title=QT_TR_NOOP("Saving PSP");
    return saveLoop(title);
}

/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerMP4::close(void) 
{
   
    printf("[MP4] Closing\n");
    return closeMuxer();
}

//EOF



