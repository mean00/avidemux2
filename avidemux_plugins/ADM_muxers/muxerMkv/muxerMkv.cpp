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
    1280,   // Display width
    0       // Display aspect ratio (any)
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
        AVCodecParameters *par;
        par = video_st->codecpar;
        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        video_st->time_base=c->time_base;
        video_st->avg_frame_rate.den =c->time_base.num;
        video_st->avg_frame_rate.num =c->time_base.den;
        c->gop_size=15;
        
        if((mkvMuxerConfig.forceDisplayWidth && mkvMuxerConfig.displayWidth) || mkvMuxerConfig.displayAspectRatio)
        {
            float h=(float)(s->getHeight());
            float w=h;
            switch (mkvMuxerConfig.displayAspectRatio)
            {
                case 1:
                    w*=4.;
                    w/=3.;
                    break;
                case 2:
                    w*=16.;
                    w/=9.;
                    break;
                case 3:
                    w*=2.;
                    break;
                case 4:
                    w*=64.;
                    w/=27.;
                    break;
                default:break;
            }

            //sar=display/code  
            int num=1,den=1;
            if(mkvMuxerConfig.forceDisplayWidth)
                av_reduce(&num, &den, mkvMuxerConfig.displayWidth, s->getWidth(),65535);
            else
                av_reduce(&num, &den, (uint32_t)w, s->getWidth(),65535);
            par->sample_aspect_ratio.num=num;
            par->sample_aspect_ratio.den=den;
            video_st->sample_aspect_ratio.num=num;
            video_st->sample_aspect_ratio.den=den;
            int dw=(int)w;
            if(mkvMuxerConfig.forceDisplayWidth)
                dw=(int)mkvMuxerConfig.displayWidth;
            ADM_info("Forcing display width of %d (pixel aspect ratio %d:%d)\n",dw,num,den);
        }

        if(initAudio(nbAudioTrack,a)==false)
        {
            ADM_warning("[Mkv] Failed to init audio\n");
            return false;
        }
        
        // /audio
        int er = avio_open(&(oc->pb), file, AVIO_FLAG_WRITE);

        if (er)
        {
            ADM_error("[Mkv]: Failed to open file :%s, er=%d\n",file,er);
            return false;
        }

        AVDictionary *dict = NULL;
		char buf[64];
        
        snprintf(buf, sizeof(buf), "%d", AV_TIME_BASE / 10);
        av_dict_set(&dict, "preload", buf, 0);
        av_dict_set(&dict, "max_delay", "200000", 0);

        ADM_assert(avformat_write_header(oc, &dict) >= 0);
        ADM_info("Timebase codec = %d/%d\n",video_st->time_base.num,video_st->time_base.den);
        ADM_info("Timebase codec2 = %d/%d\n",c->time_base.num,c->time_base.den);
//        ADM_info("Original timebase = %d/%d\n",myTimeBase.num,myTimeBase.den);
        
        
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
    const char *title=QT_TRANSLATE_NOOP("mkvmuxer","Saving Mkv");
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



