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
        AVRational myTimeBase;
        c = video_st->codec;
        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        myTimeBase=video_st->time_base=c->time_base;
        ADM_info("Video stream time base :%d,%d\n",video_st->time_base.num,video_st->time_base.den);
        c->gop_size=15;

        if(initAudio(nbAudioTrack,a)==false)
        {
            printf("[MP4] Failed to init audio\n");
            return false;
        }

        // /audio
        int er = avio_open(&(oc->pb), file, AVIO_FLAG_WRITE);

        
        ADM_info("Timebase In  = %d/%d\n",myTimeBase.num,myTimeBase.den);
        
        if (er)
        {
            ADM_error("[Mp4]: Failed to open file :%s, er=%d\n",file,er);
            return false;
        }

        AVDictionary *dict = NULL;
		char buf[64];

        snprintf(buf, sizeof(buf), "%d", AV_TIME_BASE / 10);
        av_dict_set(&dict, "preload", buf, 0);
        av_dict_set(&dict, "max_delay", "200000", 0);
        av_dict_set(&dict, "muxrate", "10080000", 0);
        av_dict_set(&dict, "movflags","faststart",0);

        ADM_assert(avformat_write_header(oc, &dict) >= 0);

        ADM_info("Timebase codec = %d/%d\n",c->time_base.num,c->time_base.den);
        ADM_info("Timebase stream = %d/%d\n",video_st->time_base.num,video_st->time_base.den);
        if(myTimeBase.den==video_st->time_base.den && video_st->time_base.num==1)
        {
            roundup=myTimeBase.num;
            ADM_warning("Timebase roundup = %d\n",roundup);

        }
        av_dict_free(&dict);
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



