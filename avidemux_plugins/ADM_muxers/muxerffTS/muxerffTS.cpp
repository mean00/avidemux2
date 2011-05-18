/***************************************************************************
            \file            muxerffTS
            \brief           i/f to lavformat PS muxer
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
#include "muxerffTS.h"
#include "DIA_coreToolkit.h"
#include "ADM_muxerUtils.h"
#include "ADM_codecType.h"
#if 0
#define aprintf(...) {}
#else
#define aprintf printf
#endif

ts_muxer tsMuxerConfig=
{
    false,      // non compliant stream
    true,       // vbr
    10          // muxRate MBits/s
};

/**
    \fn     muxerffTS
    \brief  Constructor
*/
muxerffTS::muxerffTS() 
{
};
/**
    \fn     muxerffTS
    \brief  Destructor
*/

muxerffTS::~muxerffTS() 
{
   
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerffTS::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
    uint32_t fcc=s->getFCC();
    uint32_t w,h;
     w=s->getWidth();
     h=s->getHeight();
        
     if(!isMpeg12Compatible(fcc) && !isH264Compatible(fcc))
     {
            printf("[ffTS] video not compatible\n");
            return false;
     }
   
    if(!nbAudioTrack) 
        {
            printf("[ffTS] One audio track needed\n");
            return false;
        }
   
    /* All seems fine, open stuff */
    const char *fmt="mpegts";
    
    if(false==setupMuxer(fmt,file))
    {
        printf("[ffTS] Failed to open muxer\n");
        return false;
    }
 
   if(initVideo(s)==false) 
    {
        printf("[ffTS] Failed to init video\n");
        return false;
    }
  
    
        AVCodecContext *c;
        c = video_st->codec;
        rescaleFps(s->getAvgFps1000(),&(c->time_base));


        c->gop_size=15;
        
        if(initAudio(nbAudioTrack,a)==false)
        {
            printf("[ffTS] Failed to init audio\n");
            return false;
        }
        
        // /audio
        if(tsMuxerConfig.vbr)
            oc->mux_rate=1;
        else
            oc->mux_rate=tsMuxerConfig.muxRateInMBits*1000000LL;
        audio_st->codec->bit_rate=a[0]->getInfo()->byterate*8;        
       
        oc->preload=3000; // 30 ms preloading
        oc->max_delay=2000; // 500 ms
        if (av_set_parameters(oc, NULL) < 0)
        {
            printf("[ffTS]Lav: set param failed \n");
            return false;
        }
        int erx=avio_open(&(oc->pb), file, AVIO_FLAG_WRITE);
        if (erx)
        {
            ADM_error("[Mp4]: Failed to open file :%s, er=%d\n",file,erx);
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
bool muxerffTS::save(void) 
{
    const char *title=QT_TR_NOOP("Saving mpeg TS (ff)");
    return saveLoop(title);
}
// Clock is 90 Khz for all mpeg streams
// Since the unit is in us=10e6,
// time=time/10E6*90E3
// time=(time*9)/100
bool muxerffTS::muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts)
{
    if(*time==ADM_NO_PTS)
    {
        *time=computedDts;
    }
    return muxerRescaleVideoTime(time);
}
  
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerffTS::close(void) 
{
   
    printf("[ffTS] Closing\n");
    return closeMuxer();
}

//EOF



