/***************************************************************************
            \file            muxerFlv
            \brief           i/f to lavformat Flv muxer
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
#include "muxerFlv.h"
#include "DIA_coreToolkit.h"
#include "ADM_muxerUtils.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif



/**
    \fn     muxerFlv
    \brief  Constructor
*/
muxerFlv::muxerFlv()
{
};
/**
    \fn     muxerFlv
    \brief  Destructor
*/

muxerFlv::~muxerFlv()
{
    printf("[FLV] Destructing\n");
    closeMuxer();

}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerFlv::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
    uint32_t fcc=s->getFCC();
    bool r=true;
    char *fileTitle=NULL;
        
     if(fourCC::check(fcc,(uint8_t *)"FLV1") || isVP6Compatible(fcc))
     {

     }else
        {
            GUI_Error_HIG("Unsupported","Only FLV1 & VP6 supported for video");
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
            uint32_t fq=a[i]->getInfo()->frequency;
            if(fq!=44100 &&fq!=22050 && fq!=11025)
            {
                GUI_Error_HIG("Unsupported","Only 44.1, 22.050 and 11.025 kHz supported");
                return false;
            }


        }

    if(false==setupMuxer("flv",file))
    {
        printf("[FLV] Failed to open muxer\n");
        return false;
    }
 
   if(initVideo(s)==false) 
    {
        printf("[FLV] Failed to init video\n");
        return false;
    }
  
    AVCodecContext *c;
    c = video_st->codec;   
    rescaleFps(s->getAvgFps1000(),&(c->time_base));
    c->gop_size=15;

    if(initAudio(nbAudioTrack,a)==false)
    {
        printf("[FLV] Failed to init audio\n");
        return false;
    }

        
        oc->mux_rate=10080*1000;
        oc->preload=AV_TIME_BASE/10; // 100 ms preloading
        oc->max_delay=200*1000; // 500 ms
        if (av_set_parameters(oc, NULL) < 0)
        {
            printf("[FLV]: set param failed \n");
            return false;
        }
        int er=avio_open(&(oc->pb), file, AVIO_FLAG_WRITE);
        if (er)
        {
            ADM_error("[Flv]: Failed to open file :%s, er=%d\n",file,er);
            r=false;
            goto finish;
        }

        if(av_write_header(oc)<0)
        {
            printf("[Flv Muxer] Muxer rejected the parameters\n");
            r=false;
            goto finish;
        }
        initialized=true;
finish:
        vStream=s;
        aStreams=a;
        nbAStreams=nbAudioTrack;
        return r;
}

/**
    \fn save
*/
bool muxerFlv::save(void)
{
    return saveLoop("FLV");
}



/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerFlv::close(void)
{
    printf("[FLV] Closing\n");
    closeMuxer();
    return true;
}

//EOF



