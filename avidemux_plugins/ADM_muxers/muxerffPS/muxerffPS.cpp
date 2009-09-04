/***************************************************************************
            \file            muxerffPS
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
#include "muxerffPS.h"
#include "DIA_coreToolkit.h"
#include "ADM_muxerUtils.h"
#include "ADM_codecType.h"
#if 0
#define aprintf(...) {}
#else
#define aprintf printf
#endif

psMuxerConfig_s psMuxerConfig=
{
    MUXER_DVD,false
};

/**
    \fn     muxerffPS
    \brief  Constructor
*/
muxerffPS::muxerffPS() 
{
};
/**
    \fn     muxerffPS
    \brief  Destructor
*/

muxerffPS::~muxerffPS() 
{
   
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerffPS::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
    uint32_t fcc=s->getFCC();
    uint32_t w,h;
     w=s->getWidth();
     h=s->getHeight();
        
     if(!isMpeg12Compatible(fcc))
     {
            printf("[ffPs] video not compatible\n");
            return false;
     }
    if(!psMuxerConfig.acceptNonCompliant)
    {
        switch(psMuxerConfig.muxingType)
        {
            case MUXER_VCD:
                    if(w!=352 || (h!=240 && h!=288))
                    {
                            printf("[ffPs] Bad width/height for VCD\n");
                            return false;
                    }
                    break;
            case MUXER_SVCD:
                    if((w!=352 && w!=480)|| (h!=576 && h!=480))
                    {
                            printf("[ffPs] Bad width/height for SVCD\n");
                            return false;
                    }
                    break;
            case MUXER_DVD:
                    if((w!=720 && w!=704)|| (h!=576 && h!=480))
                    {
                            printf("[ffPs] Bad width/height for DVD\n");
                            return false;
                    }
                    break;
            default:
                    ADM_assert(0);
        }
    }
    if(!nbAudioTrack) 
        {
            printf("[ffPS] One audio track needed\n");
            return false;
        }
    for(int i=0;i<nbAudioTrack;i++)
    {
        WAVHeader *head=a[i]->getInfo();
        switch(psMuxerConfig.muxingType)
        {
            case MUXER_VCD:
            case MUXER_SVCD:
                    if(head->encoding!=WAV_MP2) 
                    {
                        printf("[ffPS] VCD : only MP2 audio accepted\n");
                        return false;
                    }
                    if(head->frequency!=44100) 
                    {
                        printf("[ffPS] VCD : only 44.1 khz audio accepted\n");
                        return false;
                    }
                    break;
            case MUXER_DVD:
                    if(head->encoding!=WAV_MP2 && head->encoding!=WAV_AC3 && head->encoding!=WAV_DTS) 
                    {
                        printf("[ffPS] DVD : only MP2/AC3/DTS audio accepted\n");
                        return false;
                    }
                    if(head->frequency!=48000) 
                    {
                        printf("[ffPS] DVD : only 48 khz audio accepted\n");
                        return false;
                    }
                    break;
            default:
                    ADM_assert(0);
        }
    }

    /* All seems fine, open stuff */
    const char *fmt;
     switch(psMuxerConfig.muxingType)
        {
            case MUXER_VCD: fmt="vcd";break;
            case MUXER_SVCD:fmt="svcd";break;
            case MUXER_DVD: fmt="dvd";break;
        }
    if(false==setupMuxer(fmt,file))
    {
        printf("[ffPS] Failed to open muxer\n");
        return false;
    }
 
   if(initVideo(s)==false) 
    {
        printf("[ffPS] Failed to init video\n");
        return false;
    }
  
    
        AVCodecContext *c;
        c = video_st->codec;
        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        // Override codec settings
#define MKX(a,bsize,maxb) case a: c->bit_rate=maxb*1000;c->rc_buffer_size=bsize*8*1000;break;
        switch(psMuxerConfig.muxingType)
        {
            MKX(MUXER_VCD,  40,1152)
            MKX(MUXER_SVCD,112,2400)
            MKX(MUXER_DVD, 224,9800)
        }

        c->gop_size=15;
        
        if(initAudio(nbAudioTrack,a)==false)
        {
            printf("[ffPS] Failed to init audio\n");
            return false;
        }
        
        // /audio
        switch(psMuxerConfig.muxingType)
        {
            case MUXER_VCD:  oc->mux_rate=10080*1000;;break;
            case MUXER_SVCD: oc->mux_rate=2500*1000;;break;
            case MUXER_DVD:  oc->mux_rate=1152*1000;;break;
        }
       
        oc->preload=0; // 100 ms preloading
        oc->max_delay=2000; // 500 ms
        if (av_set_parameters(oc, NULL) < 0)
        {
            printf("[ffPs]Lav: set param failed \n");
            return false;
        }
        if (url_fopen(&(oc->pb), file, URL_WRONLY) < 0)
        {
            printf("[ffPS]: Failed to open file :%s\n",file);
            return false;
        }

        ADM_assert(av_write_header(oc)>=0);
        vStream=s;
        aStreams=a;
        nbAStreams=nbAudioTrack;
        return true;
}

/**
    \fn save
*/
bool muxerffPS::save(void) 
{
    const char *title=QT_TR_NOOP("Saving mpeg PS (ff)");
    return saveLoop(title);
}
// Clock is 90 Khz for all mpeg streams
// Since the unit is in us=10e6,
// time=time/10E6*90E3
// time=(time*9)/100
bool muxerffPS::muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts)
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
bool muxerffPS::close(void) 
{
   
    printf("[ffPS] Closing\n");
    return closeMuxer();
}

//EOF



