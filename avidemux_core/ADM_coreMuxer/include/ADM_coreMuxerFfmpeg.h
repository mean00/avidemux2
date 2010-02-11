/***************************************************************************
            \file            ADM_coreMuxerFfmpeg.h
            \brief           Base class for ffmpeg based muxers
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
#ifndef ADM_COREMUXERFFMPEG_H
#define ADM_COREMUXERFFMPEG_H
#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_muxer.h"
#include "ADM_muxerUtils.h"
#include "ADM_lavcodec.h"
extern "C"
{
    #include "libavformat/avformat.h"
};
#ifndef INT64_C
#define INT64_C(x) (uint64_t)(x##LL)
#endif
/**
    \class muxerFFmpeg
*/
class muxerFFmpeg : public ADM_muxer
{
protected:
        bool    writePacket(AVPacket *pkt);
        virtual bool muxerRescaleVideoTime(uint64_t *time)
        {
             AVRational *scale=&(video_st->time_base);
            *time=rescaleLavPts(*time,scale);
            return true;
        }
        virtual bool muxerRescaleAudioTime(uint64_t *time,uint32_t fq)
        {       
             AVRational *scale=&(audio_st->time_base);
            *time=rescaleLavPts(*time,scale);
            return true;
        }
        // On case the muxer does not accept ADM_NO_PTS we can use computedDts
        // The default is use ADM_NO_PTS
        virtual bool muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts)
        {
            return muxerRescaleVideoTime(time);
        }
        uint64_t audioDelay; // since video decoder can cause a delay, we have to delay also all audio tracks
                             // by this value. It will be 0 for copy or 0 bframe codec.
protected:
        bool saveLoop(const char *title);
protected:
        AVOutputFormat *fmt;
        AVFormatContext *oc;
        AVStream *audio_st;
        AVStream *video_st;
        double audio_pts, video_pts;

        bool closeMuxer(void);
        bool setupMuxer(const char *format,const char *filename);
        bool initVideo(ADM_videoStream *stream);
        bool initAudio(uint32_t nbAudioTrack,ADM_audioStream **audio);
        bool initialized;
public:
                muxerFFmpeg();
        virtual ~muxerFFmpeg();

};

#endif
