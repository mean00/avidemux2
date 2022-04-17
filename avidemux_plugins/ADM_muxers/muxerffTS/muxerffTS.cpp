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
    10,         // muxRate MBits/s
    false       // m2ts mode
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
    std::string msg;
    bool wrongVideo = false;
    bool wrongAudio = false;

    if(!isMpeg12Compatible(fcc) && !isH264Compatible(fcc) && !isH265Compatible(fcc) && !isVC1Compatible(fcc))
    {
        wrongVideo = true;
        msg += QT_TRANSLATE_NOOP("fftsmuxer","Video track is incompatible");
        msg += "\n";
    }

    for(int i=0;i<nbAudioTrack;i++)
    {
        char *msgformat = NULL;
        uint32_t acc = a[i]->getInfo()->encoding;
        if( acc != WAV_MP2 &&
            acc != WAV_MP3 &&
            acc != WAV_AC3 &&
            acc != WAV_EAC3 &&
            acc != WAV_DTS &&
            acc != WAV_AAC &&
            acc != WAV_AAC_HE &&
            acc != WAV_TRUEHD)
        {
            wrongAudio = true;
            msgformat = ADM_strdup(QT_TRANSLATE_NOOP("fftsmuxer","Audio track %d out of %u is incompatible"));
        }else if(tsMuxerConfig.m2TsMode)
        {
            if( acc != WAV_AC3 &&
                acc != WAV_EAC3 &&
                acc != WAV_DTS &&
                /* we don't support Blu-Ray variety of LPCM yet */
                acc != WAV_TRUEHD)
            {
                wrongAudio = true;
                msgformat = ADM_strdup(QT_TRANSLATE_NOOP("fftsmuxer","Audio track %d out of %u is incompatible with M2TS mode"));
            }
        }
        if(!msgformat) continue;

        char str[512];
        snprintf(str,512,msgformat,i+1,nbAudioTrack);
        str[511] = 0;
        msg += str;
        msg += "\n";
        ADM_dealloc(msgformat);
    }
    if(wrongVideo)
    {
        msg += "\n";
        msg += QT_TRANSLATE_NOOP("fftsmuxer","Only MPEG-1/2, VC-1, H264 and HEVC supported for video");
    }
    if(wrongAudio)
    {
        msg += "\n";
        msg += tsMuxerConfig.m2TsMode ?
            QT_TRANSLATE_NOOP("fftsmuxer","Only AC3, E-AC3, DTS and TrueHD supported for audio") :
            QT_TRANSLATE_NOOP("fftsmuxer","Only MP2, MP3, AC3, E-AC3, DTS, AAC and TrueHD supported for audio");
    }
    if(wrongVideo || wrongAudio)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("fftsmuxer","Unsupported"),msg.c_str());
        return false;
    }
    /* All seems fine, open stuff */
    const char *fmt="mpegts";
    if(!setupMuxer(fmt,file))
    {
        printf("[ffTS] Failed to open muxer\n");
        return false;
    }
    if(!initVideo(s))
    {
        printf("[ffTS] Failed to init video\n");
        return false;
    }

    AVRational tbase;
    uint32_t clockFreq = s->getTimeBaseDen();
    uint32_t nbTicks = s->getTimeBaseNum();
    if(clockFreq && nbTicks)
    {
        tbase.den = clockFreq;
        tbase.num = nbTicks;
    }else
    {
        rescaleFps(s->getAvgFps1000(),&tbase);
    }
    video_st->time_base = tbase;

    if(!initAudio(nbAudioTrack,a))
    {
        printf("[ffTS] Failed to init audio\n");
        return false;
    }

    int erx = avio_open(&(oc->pb), file, AVIO_FLAG_WRITE);
    if (erx < 0)
    {
        char str[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, erx);
        ADM_error("[ffTS] Error %d (\"%s\") opening file \"%s\"\n", erx, str, file);
        return false;
    }

    AVDictionary *dict = NULL;

    if (tsMuxerConfig.m2TsMode)
    {
        printf("[ffTS] Requesting M2TS muxing mode\n");
        av_dict_set(&dict, "mpegts_m2ts_mode", "1", 0);
    }

    if (tsMuxerConfig.vbr)
        av_dict_set(&dict, "muxrate", "1", 0);
    else
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d", tsMuxerConfig.muxRateInMBits * 1000000);
        av_dict_set(&dict, "muxrate", buf, 0);
    }

    av_dict_set(&dict, "preload", "3000", 0);
    av_dict_set(&dict, "max_delay", "2000", 0);

    erx = avformat_write_header(oc, &dict);
    if (erx < 0)
    {
        char str[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, erx);
        ADM_error("[ffTS] Writing header failed with error %d (\"%s\")\n", erx, str);
        av_dict_free(&dict);
        avio_close(oc->pb);
        return false;
    }
    vStream=s;
    aStreams=a;
    nbAStreams=nbAudioTrack;
    setOutputFileName(file);
    initialized=true;
    return true;
}

/**
    \fn save
*/
bool muxerffTS::save(void) 
{
    const char *title=QT_TRANSLATE_NOOP("fftsmuxer","Saving mpeg TS (ff)");
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



