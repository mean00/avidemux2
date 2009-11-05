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

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

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
        uint32_t fcc=stream->getFCC();
        if(isMpeg4Compatible(fcc))
        {
                c->codec_id = CODEC_ID_MPEG4;
                if(stream->providePts()==true)
                {
                    c->has_b_frames=1; // in doubt...
                    c->max_b_frames=2;
                }else   
                {
                    c->has_b_frames=0; // No PTS=cannot handle CTS...
                    c->max_b_frames=0;
                }
        }else
        {
                if(isH264Compatible(fcc))
                {
                        if(stream->providePts()==true)
                        {
                            c->has_b_frames=1; // in doubt...
                            c->max_b_frames=2;
                        }else
                        {
                            printf("[MP4] Source video has no PTS information, assuming no b frames\n");
                            c->has_b_frames=0; // No PTS=cannot handle CTS...
                            c->max_b_frames=0;
                        }
                        c->codec_id = CODEC_ID_H264;
                        c->codec=new AVCodec;
                        memset(c->codec,0,sizeof(AVCodec));
                        c->codec->name=ADM_strdup("H264");
                }
                else
                {
                        if(isDVCompatible(fcc))
                        {
                          c->codec_id = CODEC_ID_DVVIDEO;
                        }else
                        {
                          if(fourCC::check(fcc,(uint8_t *)"H263"))
                          {
                                    c->codec_id=CODEC_ID_H263;
                            }else

                           if(isVP6Compatible(stream->getFCC()))
                                {
                                         c->codec=new AVCodec;
                                         c->codec_id=CODEC_ID_VP6F;
                                         c->codec->name=ADM_strdup("VP6F");
                                         c->has_b_frames=0; // No PTS=cannot handle CTS...
                                         c->max_b_frames=0;
                                }else
                                        if(fourCC::check(stream->getFCC(),(uint8_t *)"FLV1"))
                                        {
                                                c->has_b_frames=0; // No PTS=cannot handle CTS...
                                                c->max_b_frames=0;
                                                c->codec_id=CODEC_ID_FLV1;

                                                c->codec=new AVCodec;
                                                memset(c->codec,0,sizeof(AVCodec));
                                                c->codec->name=ADM_strdup("FLV1");
                                        }else
                                        {
                                            if(fourCC::check(stream->getFCC(),(uint8_t *)"MPEG1"))
                                            {
                                                c->has_b_frames=1; // No PTS=cannot handle CTS...
                                                c->max_b_frames=2;
                                                c->codec_id=CODEC_ID_MPEG1VIDEO;
                                            }
                                            else if(fourCC::check(stream->getFCC(),(uint8_t *)"MPEG2"))
                                            {
                                                c->has_b_frames=1; // No PTS=cannot handle CTS...
                                                c->max_b_frames=2;
                                                c->codec_id=CODEC_ID_MPEG2VIDEO;
                                            }else
                                            {
                                                printf("[FF] Unknown video codec\n");
                                                return false;
                                            }
                                        }
                        }
                }
        }


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
                  case WAV_OGG_VORBIS: 
                                c->codec_id = CODEC_ID_VORBIS;c->frame_size=6*256;
                                c->extradata=audioextraData;
                                c->extradata_size= audioextraSize;
                                break;
                  case WAV_AC3: c->codec_id = CODEC_ID_AC3;c->frame_size=6*256;break;
                  case WAV_MP2: c->codec_id = CODEC_ID_MP2;c->frame_size=1152;break;
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
/**

*/
bool muxerFFmpeg::saveLoop(const char *title)
{
#define AUDIO_BUFFER_SIZE 48000*6*sizeof(float)

    printf("[FF] Saving\n");
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;
    uint8_t *buffer=new uint8_t[bufSize];
    uint32_t len,flags;
    uint64_t pts,dts,rawDts;
    uint64_t lastVideoDts=0;
    uint64_t videoIncrement;
    int ret;
    int written=0;
    bool result=true;

    float f=(float)vStream->getAvgFps1000();
    f=1000./f;
    f*=1000000;
    videoIncrement=(uint64_t)f;

    uint8_t *audioBuffer=new uint8_t[AUDIO_BUFFER_SIZE];


    printf("[MP4]avg fps=%u\n",vStream->getAvgFps1000());
    AVRational *scale=&(video_st->codec->time_base);
    uint64_t videoDuration=vStream->getVideoDuration();
    
    encoding=createWorking(title);

    while(true==vStream->getPacket(&len, buffer, bufSize,&pts,&dts,&flags))
    {
	AVPacket pkt;

            float p=1;
            if(videoDuration)
            {
                    p=lastVideoDts;
                    p/=videoDuration;
                    p=p*100;
            }
            
            encoding->update((uint32_t)p);
            if(!encoding->isAlive()) 
            {
                result=false;
                break;
            }
            int64_t xpts=(int64_t)pts;
            int64_t xdts=(int64_t)dts;
            if(pts==ADM_NO_PTS) xpts=-1;
            if(dts==ADM_NO_PTS) xdts=-1;
            aprintf("[FF:V] Pts: %"LLD" DTS:%"LLD" ms\n",xpts/1000,xdts/1000);

            aprintf("[FF:V] LastDts:%08"LLU" Dts:%08"LLU" (%04"LLU") Delta : %"LLU"\n",
                        lastVideoDts,dts,dts/1000000,dts-lastVideoDts);
            rawDts=dts;
            if(rawDts==ADM_NO_PTS)
            {
                lastVideoDts+=videoIncrement;
            }else
            {
                lastVideoDts=dts;
            }
            muxerRescaleVideoTimeDts(&dts,lastVideoDts);
            muxerRescaleVideoTime(&pts);
            aprintf("[FF:V] RawDts:%lu Scaled Dts:%lu\n",rawDts,dts);
            aprintf("[FF:V] Rescaled: Len : %d flags:%x Pts:%"LLU" Dts:%"LLU"\n",len,flags,pts,dts);

            av_init_packet(&pkt);
            pkt.dts=dts;
            if(vStream->providePts()==true)
            {
                pkt.pts=pts;
            }else
            {
                pkt.pts=pkt.dts;
            }
            pkt.stream_index=0;
            pkt.data= buffer;
            pkt.size= len;
            if(flags & 0x10) // FIXME AVI_KEY_FRAME
                        pkt.flags |= PKT_FLAG_KEY;
            ret =av_interleaved_write_frame(oc, &pkt);
            aprintf("[FF]Frame:%u, DTS=%08lu PTS=%08lu\n",written,dts,pts);
            if(ret)
            {
                printf("[FF]Error writing video packet\n");
                break;
            }
            written++;
            // Now send audio until they all have DTS > lastVideoDts+increment
            for(int audio=0;audio<nbAStreams;audio++)
            {
                uint32_t audioSize,nbSample;
                uint64_t audioDts;
                ADM_audioStream*a=aStreams[audio];
                uint32_t fq=a->getInfo()->frequency;
                int nb=0;
                while(a->getPacket(audioBuffer,&audioSize, AUDIO_BUFFER_SIZE,&nbSample,&audioDts))
                {
                    // Write...
                    nb++;
                    AVPacket pkt;
                    uint64_t rescaledDts;
                    rescaledDts=audioDts;
                    muxerRescaleAudioTime(&rescaledDts,a->getInfo()->frequency);
                    aprintf("[FF] A: Video frame  %d, audio Dts :%"LLU" size :%"LU" nbSample : %"LU" rescaled:%"LLU"\n",
                                    written,audioDts,audioSize,nbSample,rescaledDts);
                    av_init_packet(&pkt);
                    
                    pkt.dts=rescaledDts;
                    pkt.pts=rescaledDts;
                    pkt.stream_index=1+audio;
                    pkt.data= audioBuffer;
                    pkt.size= audioSize;
                    ret =av_interleaved_write_frame(oc, &pkt);
                    if(ret)
                    {
                        printf("[FF]Error writing audio packet\n");
                        break;
                    }
                    aprintf("[FF] A:%"LU" ms vs V: %"LU" ms\n",(uint32_t)audioDts/1000,(uint32_t)(lastVideoDts+videoIncrement)/1000);
                    if(audioDts!=ADM_NO_PTS)
                    {
                        if(audioDts>lastVideoDts+videoIncrement) break;
                    }
                }
                //if(!nb) printf("[FF] A: No audio for video frame %d\n",written);
            }

    }
    delete [] buffer;
    delete [] audioBuffer;
    printf("[FF] Wrote %d frames, nb audio streams %d\n",written,nbAStreams);
    return result;
}
// EOF
