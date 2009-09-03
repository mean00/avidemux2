/***************************************************************************
            \file            muxerFFmpeg
            \brief           Base class for ffmpeg based muxer
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
#include "ADM_coreMuxerFfmpeg.h"
#include "ADM_muxerUtils.h"
/**
    \fn muxerFFmpeg
*/
muxerFFmpeg::muxerFFmpeg()
{
    fmt=NULL;
    oc=NULL;
    audio_st=NULL;
    video_st=NULL;
}
/**
    \fn closeMuxer
*/
bool muxerFFmpeg::closeMuxer()
{
    if(oc)
    {
        av_write_trailer(oc);
        url_fclose((oc->pb));
    }
    if(audio_st)
    {
         av_free(audio_st);
    }
    if(video_st)
    {
         av_free(video_st);
    }
    video_st=NULL;
    audio_st=NULL;
    if(oc)
        av_free(oc);
    oc=NULL;
    return true;
}
/**
    \fn muxerFFmpeg
*/
muxerFFmpeg::~muxerFFmpeg()
{
        closeMuxer();
}
/**
    \fn setupMuxer
    \brief open and do the base muxer setup
*/
bool muxerFFmpeg::setupMuxer(const char *format,const char *filename)
{
    fmt=guess_format(format, NULL, NULL);
    if(!fmt)
    {
            printf("[FF] guess format failed\n");
            return false;
    }
    oc = avformat_alloc_context();
	if (!oc)
	{
       		printf("[FF] alloc format context failed\n");
            return false;
	}
	oc->oformat = fmt;
	snprintf(oc->filename,1000,"file://%s",filename);
    // probably a memeleak here
    char *foo=ADM_strdup(filename);
    
    strcpy(oc->title,ADM_GetFileName(foo));
    strcpy(oc->author,"Avidemux");
    printf("[FF] Muxer opened\n");
    return true;
}
/**
    \fn initVideo
    \brief setup video part of muxer
*/
bool muxerFFmpeg::initVideo(ADM_videoStream *stream)
{
    video_st = av_new_stream(oc, 0);
	if (!video_st)
	{
		printf("[FF] new stream failed\n");
		return false;
	}
    AVCodecContext *c;
        c = video_st->codec;
        c->sample_aspect_ratio.num=1;
        c->sample_aspect_ratio.den=1;
        video_st->sample_aspect_ratio=c->sample_aspect_ratio;

        uint32_t videoExtraDataSize=0;
        uint8_t  *videoExtraData;
        stream->getExtraData(&videoExtraDataSize,&videoExtraData);
        printf("[FF] Using %d bytes for video extradata\n",(int)videoExtraDataSize);
        if(videoExtraDataSize)
        {
                c->extradata=videoExtraData;
                c->extradata_size= videoExtraDataSize;
        }
        
        c->rc_buffer_size=8*1024*224;
        c->rc_max_rate=9500*1000;
        c->rc_min_rate=0;
        c->bit_rate=9000*1000;
        c->codec_type = CODEC_TYPE_VIDEO;
        c->flags=CODEC_FLAG_QSCALE;
        c->width = stream->getWidth();
        c->height =stream->getHeight();
        printf("[FF] Video initialized\n");

    return true;
}
/**
    \fn initAudio
    \brief setup the audio parts if present
*/
bool muxerFFmpeg::initAudio(uint32_t nbAudioTrack,ADM_audioStream **audio)
{
    if(!nbAudioTrack)
    {
        printf("[FF] No audio\n");
        return true;
    }
#warning : only handle one audio track!
    for(int i=0;i<1;i++)
    {
          uint32_t audioextraSize;
          uint8_t  *audioextraData;
          
          audio[i]->getExtraData(&audioextraSize,&audioextraData);

          audio_st = av_new_stream(oc, 1);
          if (!audio_st)
          {
                  printf("[FF]: new stream failed (audio)\n");
                  return false;
          }
          WAVHeader *audioheader=audio[i]->getInfo();;
          AVCodecContext *c;
          c = audio_st->codec;
          c->frame_size=1024; //For AAC mainly, sample per frame
          printf("[FF] Bitrate %u\n",(audioheader->byterate*8)/1000);
          c->sample_rate = audioheader->frequency;
          switch(audioheader->encoding)
          {
                  case WAV_AC3: c->codec_id = CODEC_ID_AC3;c->frame_size=6*256;break;
                  case WAV_MP2: c->codec_id = CODEC_ID_MP2;break;
                  case WAV_MP3:
  #warning FIXME : Probe deeper
                              c->frame_size=1152;
                              c->codec_id = CODEC_ID_MP3;
                              break;
                  case WAV_PCM:
                                  // One chunk is 10 ms (1/100 of fq)
                                  c->frame_size=4;
                                  c->codec_id = CODEC_ID_PCM_S16LE;break;
                  case WAV_AAC:
                                  c->extradata=audioextraData;
                                  c->extradata_size= audioextraSize;
                                  c->codec_id = CODEC_ID_AAC;
                                  c->frame_size=1024;
                                  break;
                  default:
                                 printf("[FF]: Unsupported audio\n");
                                 return false; 
                          break;
          }
          c->codec_type = CODEC_TYPE_AUDIO;
          c->bit_rate = audioheader->byterate*8;
          c->rc_buffer_size=(c->bit_rate/(2*8)); // 500 ms worth
          c->channels = audioheader->channels;

        }
        printf("[FF] Audio initialized\n");
        return true;
}
// EOF
