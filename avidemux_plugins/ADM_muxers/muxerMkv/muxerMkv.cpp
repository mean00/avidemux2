/***************************************************************************
            \file            muxerMkv
            \brief           i/f to lavformat Matroska / WebM muxer
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

mkv_muxer muxerConfig=
{
    false,  // Force DAR
    1280,   // Display width
    2,      // Display aspect ratio (16:9)
    false,  // Add color info
    2,      // Matrix coeff. unspecified
    0,      // Color range unspecified
    2,      // Transfer characteristic unspecified
    2       // Color primaries unspecified
};

#ifdef MUXER_IS_WEBM
#   define MUXFMT WebM
#else
#   define MUXFMT Matroska
#endif
#define STR(x) #x
#define MKSTRING(x) STR(x)
#define MUXTYPE MKSTRING(MUXFMT)

/**
    \fn     muxerMkv
    \brief  Constructor
*/
MKVCLASS::MKVCLASS()
{
    ADM_info("Creating " MUXTYPE " muxer.\n");
}

/**
    \fn     muxerMkv
    \brief  Destructor
*/
MKVCLASS::~MKVCLASS()
{
    ADM_info("Destroying " MUXTYPE " muxer.\n");
}

/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/
bool MKVCLASS::open(const char *file, ADM_videoStream *s, uint32_t nbAudioTrack, ADM_audioStream **a)
{
#ifdef MUXER_IS_WEBM
    // We only support VP8 / VP9 / AV1 + Vorbis / Opus
    uint32_t fcc=s->getFCC();
    if( !fourCC::check(fcc,(const uint8_t *)"VP8 ")
        && !fourCC::check(fcc,(const uint8_t *)"VP9 ")
        && !fourCC::check(fcc,(const uint8_t *)"av01"))
    {
        GUI_Error_HIG("WebM",QT_TRANSLATE_NOOP("mkvmuxer","Unsupported Video.\nOnly VP8/VP9/AV1 video and Vorbis/Opus audio supported"));
        return false;
    }
#endif

    /* All seems fine, open stuff */
    if(false==setupMuxer(
#ifdef MUXER_IS_WEBM
        "webm",
#else
        "matroska",
#endif
        file))
    {
        ADM_warning("Failed to setup " MUXTYPE " muxer.\n");
        return false;
    }
 
    if(initVideo(s)==false)
    {
        ADM_warning("[" MUXTYPE "] Failed to init video.\n");
        return false;
    }

    AVCodecContext *c = video_st->codec;
    AVCodecParameters *par = video_st->codecpar;
#ifndef MUXER_IS_WEBM
    if(par->codec_tag == MKTAG('V','P','9',' '))
        par->codec_tag = MKTAG('V','P','9','0');
#endif
    rescaleFps(s->getAvgFps1000(),&(c->time_base));
    video_st->time_base=c->time_base;
    video_st->avg_frame_rate.den =c->time_base.num;
    video_st->avg_frame_rate.num =c->time_base.den;
    //c->gop_size=15; // ??

    /* DAR / SAR code */
    if(muxerConfig.forceAspectRatio && (muxerConfig.displayWidth || muxerConfig.displayAspectRatio))
    {
        float h=(float)(s->getHeight());
        float w=h;
        switch (muxerConfig.displayAspectRatio)
        {
            case STANDARD:
                w*=4.;
                w/=3.;
                break;
            case WIDE:
                w*=16.;
                w/=9.;
                break;
            case UNI:
                w*=2.;
                break;
            case CINEMA:
                w*=64.;
                w/=27.;
                break;
            default:break;
        }

        int num=1,den=1;
        if(muxerConfig.displayAspectRatio == OTHER)
            av_reduce(&num, &den, muxerConfig.displayWidth, s->getWidth(),65535);
        else
            av_reduce(&num, &den, w, s->getWidth(),65535);
        par->sample_aspect_ratio.num=num;
        par->sample_aspect_ratio.den=den;
        video_st->sample_aspect_ratio.num=num;
        video_st->sample_aspect_ratio.den=den;
        int dw=(int)w;
        if(muxerConfig.displayAspectRatio == OTHER)
            dw=(int)muxerConfig.displayWidth;
        ADM_info("Forcing display width of %d (pixel aspect ratio %d:%d)\n",dw,num,den);
    }

    /* HDR stuff */
    if(muxerConfig.addColourInfo)
    {
        par->color_range = (AVColorRange)muxerConfig.colRange;
        par->color_primaries = (AVColorPrimaries)muxerConfig.colPrimaries;
        par->color_trc = (AVColorTransferCharacteristic)muxerConfig.colTransfer;
        par->color_space = (AVColorSpace)muxerConfig.colMatrixCoeff;
    }

    /* Audio */
    if(initAudio(nbAudioTrack,a)==false)
    {
        ADM_warning("[" MUXTYPE "] Failed to init audio.\n");
        return false;
    }

    int er = avio_open(&(oc->pb), file, AVIO_FLAG_WRITE);

    if (er)
    {
        char str[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, er);
        ADM_error("[" MUXTYPE "] Failed to open file \"%s\", error %d (%s)\n", file, er, str);
        return false;
    }

    AVDictionary *dict = NULL;
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", AV_TIME_BASE / 10);
    av_dict_set(&dict, "preload", buf, 0);
    av_dict_set(&dict, "max_delay", "200000", 0);

    er = avformat_write_header(oc, &dict);

    if(er < 0)
    {
        char str[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, er);
        ADM_error("Writing header failed with error %d (%s)\n", er, str);
        av_dict_free(&dict);
        avio_close(oc->pb);
        return false;
    }
    ADM_info("Timebase codec = %d/%d\n",video_st->time_base.num,video_st->time_base.den);
    ADM_info("Timebase codec2 = %d/%d\n",c->time_base.num,c->time_base.den);
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
bool MKVCLASS::save(void)
{
    const char *title=
#ifdef MUXER_IS_WEBM
    QT_TRANSLATE_NOOP("mkvmuxer","Saving WebM");
#else
    QT_TRANSLATE_NOOP("mkvmuxer","Saving Mkv");
#endif
    return saveLoop(title);
}

bool MKVCLASS::muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts)
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
bool MKVCLASS::close(void)
{
    ADM_info("[" MUXTYPE "] Closing\n");
    return closeMuxer();
}

//EOF



