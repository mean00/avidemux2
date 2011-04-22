/***************************************************************************
            \file            muxerMkv
            \brief           i/f to lavformat Matroska muxer
                             -------------------
    
    copyright            : (C) 2009 by mean
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
#include "muxerMkv.h"
#include "DIA_coreToolkit.h"
#include "ADM_muxerUtils.h"

#if 0
#define aprintf(...) {}
#else
#define aprintf printf
#endif

mkv_muxer mkvMuxerConfig=
{
    false,  // Force
    1280    // Display width
};


/**
    \fn     muxerMkv
    \brief  Constructor
*/
muxerMkv::muxerMkv() 
{
};
/**
    \fn     muxerMkv
    \brief  Destructor
*/

muxerMkv::~muxerMkv() 
{
   
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerMkv::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
    /* All seems fine, open stuff */
    if(false==setupMuxer("matroska",file))
    {
        ADM_warning("[Mkv] Failed to open muxer (setup)\n");
        return false;
    }
 
   if(initVideo(s)==false) 
    {
        ADM_warning("[Mkv] Failed to init video\n");
        return false;
    }
  
    
        AVCodecContext *c;
        c = video_st->codec;
        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        c->gop_size=15;
        
        if(true==mkvMuxerConfig.forceDisplayWidth && mkvMuxerConfig.displayWidth)
        {
            //sar=display/code  
            int num=1,den=1;
            av_reduce(&num, &den, mkvMuxerConfig.displayWidth, s->getWidth(),65535);
            c->sample_aspect_ratio.num=num;
            c->sample_aspect_ratio.den=den;
            video_st->sample_aspect_ratio.num=num;
            video_st->sample_aspect_ratio.den=den;
            ADM_info("Forcing display width of %d\n",(int)mkvMuxerConfig.displayWidth);
        }

        if(initAudio(nbAudioTrack,a)==false)
        {
            ADM_warning("[Mkv] Failed to init audio\n");
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
        int er=avio_open(&(oc->pb), file, AVIO_FLAG_WRITE);
        if (er)
        {
            ADM_error("[Mkv]: Failed to open file :%s, er=%d\n",file,er);
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
bool muxerMkv::save(void) 
{
    const char *title=QT_TR_NOOP("Saving Mkv");
    return saveLoop(title);
}

bool muxerMkv::muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts)
{
    if(*time==ADM_NO_PTS)
    {
        *time=computedDts;
        return muxerRescaleVideoTime(time);
    }
    return muxerRescaleVideoTime(time);
}  
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerMkv::close(void) 
{
   
    ADM_info("[Mkv] Closing\n");
    return closeMuxer();
}

//EOF



