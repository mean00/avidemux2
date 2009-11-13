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

typedef struct
{
    const char *fmt;
    uint32_t   bufferSizekBytes;
    uint32_t   muxRatekBits;
    uint32_t   videoRatekBits;
}mpegPsStruct;

const mpegPsStruct psDescriptor[3]=
{
    { "vcd",  40,1400,1152},  // Verify, not sure!
    { "svcd",112,2800,2400},
    { "dvd", 224,11000,9800},
};

ps_muxer psMuxerConfig=
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
const char *er;

    if(verifyCompatibility(psMuxerConfig.acceptNonCompliant,psMuxerConfig.muxingType,s,nbAudioTrack,a,&er)==false)
    {
        GUI_Error_HIG("[Mismatch]","%s",er);
        return false;
    }

    mpegPsStruct myself=psDescriptor[psMuxerConfig.muxingType];
    if(false==setupMuxer(myself.fmt,file))
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

        // Override codec settings
        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        c->bit_rate=myself.videoRatekBits*1000;
        c->rc_buffer_size=myself.bufferSizekBytes*8*1024;
        c->rc_buffer_size_header=myself.bufferSizekBytes*8*1024;
        c->gop_size=15;

        // Audio
        if(initAudio(nbAudioTrack,a)==false)
        {
            printf("[ffPS] Failed to init audio\n");
            return false;
        }
        audio_st->codec->bit_rate=a[0]->getInfo()->byterate*8;        
        // /audio
        oc->mux_rate=myself.muxRatekBits*1000;
        // Also copy audio & video bitrate


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

/**
    \fn verifyCompatibility
    \return true if the streams are ok to be muxed by selected muxer

*/
#define FAIL(x) {*er=x;return false;}
bool muxerffPS::verifyCompatibility(bool nonCompliantOk, uint32_t muxingType,
                                    ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a, 
                                    const char **er)
{
    uint32_t fcc=s->getFCC();
    uint32_t w,h;
     w=s->getWidth();
     h=s->getHeight();
     *er="??";

     if(!isMpeg12Compatible(fcc))
     {
            FAIL(" video not compatible\n");
     }
    if(!nonCompliantOk)
    {
        switch(muxingType)
        {
            case MUXER_VCD:
                    if(w!=352 || (h!=240 && h!=288))
                    {
                            FAIL(" Bad width/height for VCD\n");
                    }
                    break;
            case MUXER_SVCD:
                    if((w!=352 && w!=480)|| (h!=576 && h!=480))
                    {
                            FAIL(" Bad width/height for SVCD\n");
                    }
                    break;
            case MUXER_DVD:
                    if((w!=720 && w!=704)|| (h!=576 && h!=480))
                    {
                            FAIL(" Bad width/height for DVD\n");
                    }
                    break;
            default:
                    ADM_assert(0);
        }
    }
    if(!nbAudioTrack) 
        {
            FAIL(" One audio track needed\n");
            
        }
    for(int i=0;i<nbAudioTrack;i++)
    {
        WAVHeader *head=a[i]->getInfo();
        switch(muxingType)
        {
            case MUXER_VCD:
            case MUXER_SVCD:
                    if(head->encoding!=WAV_MP2) 
                    {
                        FAIL(" VCD : only MP2 audio accepted\n");
                    }
                    if(head->frequency!=44100) 
                    {
                        FAIL(" VCD : only 44.1 khz audio accepted\n");
                    }
                    break;
            case MUXER_DVD:
                    if(head->encoding!=WAV_MP2 && head->encoding!=WAV_AC3 && head->encoding!=WAV_DTS) 
                    {
                        FAIL("[ffPS] DVD : only MP2/AC3/DTS audio accepted\n");
                    }
                    if(head->frequency!=48000) 
                    {
                        FAIL(" DVD : only 48 khz audio accepted\n");
                    }
                    break;
            default:
                    ADM_assert(0);
        }
    }
    return true;
}
//EOF



