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
#include "ADM_coreCodecMapping.h"
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif
/**
    \fn writePacket
*/
bool muxerFFmpeg::writePacket(AVPacket *pkt)
{
#if 0
        printf("Track :%d size :%d PTS:%"LLD" DTS:%"LLD"\n",
                    pkt->stream_index,pkt->size,pkt->pts,pkt->dts);
#endif
    int ret =av_write_frame(oc, pkt);
    if(ret)
        return false;
    return true;
}

/**
    \fn muxerFFmpeg
*/
muxerFFmpeg::muxerFFmpeg()
{
    fmt=NULL;
    oc=NULL;
    for(int i=0;i<ADM_MAX_AUDIO_STREAM;i++)
        audio_st[i]=NULL;
    video_st=NULL;
    audioDelay=0;
    initialized=false;
}
/**
    \fn closeMuxer
*/
bool muxerFFmpeg::closeMuxer()
{
    if(oc)
    {
        if(initialized==true)
        {
            av_write_trailer(oc);
            url_fclose((oc->pb));
        }
    }
    for(int i=0;i<ADM_MAX_AUDIO_STREAM;i++)
    {
        if(audio_st[i])
            av_free(audio_st[i]);
        audio_st[i]=NULL;
    }
    if(video_st)
    {
         av_free(video_st);
    }
    video_st=NULL;
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
    fmt=av_guess_format(format, NULL, NULL);
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
#warning use AV METADATA
#if 0
    strcpy(oc->title,ADM_GetFileName(foo));
    strcpy(oc->author,"Avidemux");
#endif
    printf("[FF] Muxer opened\n");
    return true;
}
/**
    \fn initVideo
    \brief setup video part of muxer
*/
bool muxerFFmpeg::initVideo(ADM_videoStream *stream)
{
    audioDelay=stream->getVideoDelay();
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
        c->codec_type = AVMEDIA_TYPE_VIDEO;
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
                    ADM_warning("Incoming stream does not provide PTS \n");
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
                                                uint32_t id=stream->getFCC();

                                                CodecID cid=ADM_codecIdFindByFourcc(fourCC::tostring(id));
                                                if(cid==CODEC_ID_NONE)
                                                {
                                                    printf("[FF] Unknown video codec\n");
                                                    return false;
                                                }
                                                c->codec_id=cid;
                                            }
                                        }
                        }
                }
        }
        if(useGlobalHeader()==true)
        {
            if(videoExtraDataSize)
            {
                ADM_info("Video has extradata and muxer requires globalHeader, assuming it is done so.\n");
                c->flags|=CODEC_FLAG_GLOBAL_HEADER;
            }else
            {
                ADM_warning("Video has no extradata but muxer requires globalHeader.\n");
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
    
    for(int i=0;i<nbAudioTrack;i++)
    {
          uint32_t audioextraSize;
          uint8_t  *audioextraData;

          audio[i]->getExtraData(&audioextraSize,&audioextraData);

          audio_st[i] = av_new_stream(oc, 1+i);
          if (!audio_st[i])
          {
                  printf("[FF]: new stream failed (audio)\n");
                  return false;
          }
          WAVHeader *audioheader=audio[i]->getInfo();;
          AVCodecContext *c;
          c = audio_st[i]->codec;
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
                  case WAV_DTS: c->codec_id = CODEC_ID_DTS;c->frame_size=1024;break;
                  case WAV_EAC3: c->codec_id = CODEC_ID_EAC3;c->frame_size=6*256;break;
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
          c->codec_type = AVMEDIA_TYPE_AUDIO;
          c->bit_rate = audioheader->byterate*8;
          c->rc_buffer_size=(c->bit_rate/(2*8)); // 500 ms worth
          c->channels = audioheader->channels;
          if(useGlobalHeader()==true)
          {
            if(audioextraSize)
            {
                ADM_info("Audio has extradata and muxer requires globalHeader, assuming it is done so.\n");
                c->flags|=CODEC_FLAG_GLOBAL_HEADER;
            }else
            {
                ADM_warning("Audio has no extradata but muxer requires globalHeader.\n");
            }
          }
        }
        printf("[FF] Audio initialized\n");
        return true;
}
#define AUDIO_BUFFER_SIZE 48000*6*sizeof(float)
class MuxAudioPacket
{
public:
    MuxAudioPacket() {eof=false;dts=ADM_NO_PTS;present=false;size=0;}
    uint8_t     buffer[AUDIO_BUFFER_SIZE];
    uint32_t    size;
    bool        eof;
    bool        present;
    uint64_t    dts;
    uint32_t    samples;
};

/**
    \fn saveLoop
*/
bool muxerFFmpeg::saveLoop(const char *title)
{


    printf("[FF] Saving\n");
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;
    uint8_t *buffer=new uint8_t[bufSize];
    uint64_t rawDts;
    uint64_t lastVideoDts=0;
    uint64_t videoIncrement;
    int ret;
    int written=0;
    bool result=true;
    int missingPts=0;
    
    float f=(float)vStream->getAvgFps1000();
    f=1000./f;
    f*=1000000;
    videoIncrement=(uint64_t)f;



    ADM_info("avg fps=%u\n",vStream->getAvgFps1000());
    AVRational *scale=&(video_st->codec->time_base);
    uint64_t videoDuration=vStream->getVideoDuration();

    initUI("Saving");
    encoding->setContainer(getContainerName());
    MuxAudioPacket *audioPackets=new MuxAudioPacket[nbAStreams];
    ADMBitstream out(bufSize);
    out.data=buffer;

    while(true==vStream->getPacket(&out))
    {
	AVPacket pkt;

            encoding->refresh();
            if(!encoding->isAlive())
            {
                result=false;
                break;
            }
            int64_t xpts=(int64_t)out.pts;
            int64_t xdts=(int64_t)out.dts;
            if(out.pts==ADM_NO_PTS) xpts=-1;
            if(out.dts==ADM_NO_PTS) xdts=-1;
            aprintf("[FF:V] Pts: %"LLD" DTS:%"LLD" ms\n",xpts/1000,xdts/1000);

            aprintf("[FF:V] LastDts:%08"LLU" Dts:%08"LLU" (%04"LLU") Delta : %"LLU"\n",
                        lastVideoDts,out.dts,out.dts/1000000,out.dts-lastVideoDts);
            rawDts=out.dts;
            if(rawDts==ADM_NO_PTS)
            {
                lastVideoDts+=videoIncrement;
            }else
            {
                lastVideoDts=out.dts;
            }
            if(out.pts==ADM_NO_PTS)
            {
                ADM_warning("No PTS information for frame %"LU"\n",written);
                missingPts++;
                out.pts=lastVideoDts;
            }


            encoding->pushVideoFrame(out.len,out.out_quantizer,lastVideoDts);
            muxerRescaleVideoTimeDts(&(out.dts),lastVideoDts);
            muxerRescaleVideoTime(&(out.pts));
            aprintf("[FF:V] RawDts:%lu Scaled Dts:%lu\n",rawDts,out.dts);
            aprintf("[FF:V] Rescaled: Len : %d flags:%x Pts:%"LLU" Dts:%"LLU"\n",out.len,out.flags,out.pts,out.dts);

            av_init_packet(&pkt);
            pkt.dts=out.dts;
            if(vStream->providePts()==true)
            {
                pkt.pts=out.pts;
            }else
            {
                pkt.pts=pkt.dts;
            }
            pkt.stream_index=0;
            pkt.data= buffer;
            pkt.size= out.len;
            if(out.flags & 0x10) // FIXME AVI_KEY_FRAME
                        pkt.flags |= AV_PKT_FLAG_KEY;
            ret =writePacket( &pkt);
            aprintf("[FF]Frame:%u, DTS=%08lu PTS=%08lu\n",written,out.dts,out.pts);
            if(false==ret)
            {
                printf("[FF]Error writing video packet\n");
                break;
            }
            written++;
            // Now send audio until they all have DTS > lastVideoDts+increment
            for(int audio=0;audio<nbAStreams;audio++)
            {
                MuxAudioPacket *audioTrack=&(audioPackets[audio]);
                ADM_audioStream*a=aStreams[audio];
                uint32_t fq=a->getInfo()->frequency;

                while(1)
                {
                    if(audioTrack->eof==true) break; // no more packet for this track
                    if(audioTrack->present==false)
                    {
                        if(false==a->getPacket(audioTrack->buffer,
                                                &(audioTrack->size),
                                                AUDIO_BUFFER_SIZE,
                                                &(audioTrack->samples),
                                                &(audioTrack->dts)))
                        {
                                audioTrack->eof=true;
                                ADM_info("No more audio packets for audio track %d\n",audio);
                                break;
                        }
                       // printf("Track %d , new audio packet DTS=%"LLD" size=%"LU"\n",audioTrack->dts,audioTrack->size);
                        audioTrack->present=true;
                        // Delay audio by the delay induce by encoder
                        if(audioTrack->dts!=ADM_NO_PTS) audioTrack->dts+=audioDelay;
                    }
                    if(audioTrack->dts!=ADM_NO_PTS)
                    {
                        //printf("Audio PTS:%"LLD", limit=%"LLD"\n",audioTrack->dts,lastVideoDts+videoIncrement);
                        if(audioTrack->dts>lastVideoDts+videoIncrement) break; // This packet is in the future
                    }
                    // Write...
                    AVPacket pkt;
                    uint64_t rescaledDts;
                    rescaledDts=audioTrack->dts;
                    encoding->pushAudioFrame(audioTrack->size);
                    muxerRescaleAudioTime(audio,&rescaledDts,a->getInfo()->frequency);
                   //printf("[FF] A: Video frame  %d, audio Dts :%"LLU" size :%"LU" nbSample : %"LU" rescaled:%"LLU"\n",
                     //               written,audioTrack->dts,audioTrack->size,audioTrack->samples,rescaledDts);
                    av_init_packet(&pkt);

                    pkt.dts=rescaledDts;
                    pkt.pts=rescaledDts;
                    pkt.stream_index=1+audio;
                    pkt.data= audioTrack->buffer;
                    pkt.size= audioTrack->size;
                    pkt.flags |= AV_PKT_FLAG_KEY; // Assume all audio are keyframe, which is slightly wrong
                    ret =writePacket( &pkt);
                    audioTrack->present=false; // consumed
                    if(false==ret)
                    {
                        ADM_warning("[FF]Error writing audio packet\n");
                        break;
                    }
                   // printf("[FF] A:%"LU" ms vs V: %"LU" ms\n",(uint32_t)audioTrack->dts/1000,(uint32_t)(lastVideoDts+videoIncrement)/1000);
                }
                //if(!nb) printf("[FF] A: No audio for video frame %d\n",written);
            }

    }
    delete [] buffer;
    if((videoDuration *4)/5 > lastVideoDts)
    {
        GUI_Error_HIG("Too short","The video has been saved but seems to be incomplete.");
        result=false;
    }
    ADM_info("[FF] Wrote %d frames, nb audio streams %d\n",written,nbAStreams);
    ADM_info("[FF] Found %d missing PTS / %d total frames\n",missingPts,written);
    delete [] audioPackets;
    audioPackets=NULL;
    return result;
}
// EOF
