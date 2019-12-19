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

#include "ADM_coreMuxer6_export.h"
#include "fourcc.h"
#include "ADM_muxer.h"
#include "ADM_muxerUtils.h"

extern "C"
{
	#include "libavutil/avutil.h"
    #include "libavformat/avformat.h"
};
#define ADM_MAX_AUDIO_STREAM 10
/**
    \class muxerFFmpeg
*/
class ADM_COREMUXER6_EXPORT muxerFFmpeg : public ADM_muxer
{
protected:
        bool    writePacket(AVPacket *pkt);
        virtual bool muxerRescaleVideoTime(uint64_t *time)
        {
             AVRational *scale=&(video_st->time_base);
            *time=rescaleLavPts(*time,scale);
            // round up / down to the nearest to my timebase
            if(roundup)
            {
                *time=(*time+roundup/2)/(roundup);
                *time=*time*roundup;
            }
            return true;
        }
        virtual bool muxerRescaleAudioTime(int trk,uint64_t *time,uint32_t fq)
        {       
             AVRational *scale=&(audio_st[trk]->time_base);
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
        int             roundup;
        AVStream *audio_st[ADM_MAX_AUDIO_STREAM];
        AVStream *video_st;
        double audio_pts, video_pts;

        bool closeMuxer(void);
        bool setupMuxer(const char *format,const char *filename);
        bool initVideo(ADM_videoStream *stream);
        bool initAudio(uint32_t nbAudioTrack,ADM_audioStream **audio);
        bool initialized;
  virtual   const char *getContainerName(void)=0;
public:
                muxerFFmpeg();
        virtual ~muxerFFmpeg();

};

#endif
