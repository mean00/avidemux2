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
#include "ADM_lavcodec.h"
extern "C"
{
    #include "libavformat/avformat.h"
};
/**
    \class muxerFFmpeg
*/
class muxerFFmpeg : public ADM_muxer
{
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
public:
                muxerFFmpeg();
        virtual ~muxerFFmpeg();

};

#endif