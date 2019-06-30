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
    MP4_MUXER_OPT_FASTSTART,
    false,
    WIDE,
    MP4_MUXER_ROTATE_0
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
    if(!isMpeg4Compatible(s->getFCC()) && !isH264Compatible(s->getFCC()) && !isH265Compatible(s->getFCC()))
    {
            GUI_Error_HIG(QT_TRANSLATE_NOOP("mp4muxer","Unsupported"),QT_TRANSLATE_NOOP("mp4muxer","Only MP4Video, H264, and H265 supported for video"));
            return false;
    }
    if(nbAudioTrack)
        for(int i=0;i<nbAudioTrack;i++)
        {
            uint32_t acc=a[i]->getInfo()->encoding;
            if(acc!=WAV_MP2 && acc!=WAV_MP3 && acc!=WAV_AAC && acc!=WAV_AC3 && acc!=WAV_EAC3)
            {
                GUI_Error_HIG(QT_TRANSLATE_NOOP("mp4muxer","Unsupported"),QT_TRANSLATE_NOOP("mp4muxer","Only AAC, AC3, E-AC3 and mpegaudio supported for audio"));
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
        AVCodecParameters *par;
        par = video_st->codecpar;
        if(isH265Compatible(s->getFCC()))
            par->codec_tag = MKTAG('h', 'v', 'c', '1');
        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        myTimeBase=video_st->time_base=c->time_base;
        ADM_info("Video stream time base :%d,%d\n",video_st->time_base.num,video_st->time_base.den);
        c->gop_size=15;

        if(true==muxerConfig.forceAspectRatio)
        {
            float h=(float)(s->getHeight());
            float w=h;
            switch (muxerConfig.aspectRatio)
            {
                case 0:
                    w*=4.;
                    w/=3.;
                    break;
                case 1:
                    w*=16.;
                    w/=9.;
                    break;
                case 2:
                    w*=2.;
                    break;
                case 3:
                    w*=64.;
                    w/=27.;
                    break;
            }
            int num=1,den=1;
            av_reduce(&num, &den, (uint32_t)w, s->getWidth(),65535);
            par->sample_aspect_ratio.num=num;
            par->sample_aspect_ratio.den=den;
            video_st->sample_aspect_ratio.num=num;
            video_st->sample_aspect_ratio.den=den;
            ADM_info("Forcing pixel aspect ratio of %d:%d\n",den,num);
        }

        if(initAudio(nbAudioTrack,a)==false)
        {
            printf("[MP4] Failed to init audio\n");
            return false;
        }
        // Mark all audio tracks as enabled, VLC is picky about that.
        for(int i=0;i<nbAudioTrack;i++)
        {
            audio_st[i]->disposition |= AV_DISPOSITION_DEFAULT;
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

        switch(muxerConfig.optimize)
        {
            case(MP4_MUXER_OPT_FASTSTART):
                av_dict_set(&dict, "movflags", "faststart", 0);
                break;
            case(MP4_MUXER_OPT_FRAGMENT):
                av_dict_set(&dict, "movflags", "frag_keyframe+empty_moov", 0);
                av_dict_set(&dict, "min_frag_duration", "2000000", 0); // 2 seconds, an arbitrary value
                break;
            default: break;
        }

        const char *angle=NULL;
        switch(muxerConfig.rotation)
        {
            case(MP4_MUXER_ROTATE_90):
                angle="90";
                break;
            case(MP4_MUXER_ROTATE_180):
                angle="180";
                break;
            case(MP4_MUXER_ROTATE_270):
                angle="270";
                break;
            default: break;
        }
        if(angle)
        {
            ADM_info("Setting rotation to %s degrees clockwise\n",angle);
            av_dict_set(&(video_st->metadata), "rotate", angle, 0);
        }
        ADM_assert(avformat_write_header(oc, &dict) >= 0);

        ADM_info("Timebase codec = %d/%d\n",c->time_base.num,c->time_base.den);
        ADM_info("Timebase stream = %d/%d\n",video_st->time_base.num,video_st->time_base.den);
        // The reason to enforce strict CFR is unclear. Disable rounding, which may result in collisions, for now.
        if(myTimeBase.den==video_st->time_base.den && video_st->time_base.num==1)
        {
            //roundup=myTimeBase.num;
            ADM_info("Timebase roundup would have been %d, ignoring\n",myTimeBase.num);
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
    const char *title=QT_TRANSLATE_NOOP("mp4muxer","Saving mp4");
    if(muxerConfig.muxerType==MP4_MUXER_PSP) title=QT_TRANSLATE_NOOP("mp4muxer","Saving PSP");
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



