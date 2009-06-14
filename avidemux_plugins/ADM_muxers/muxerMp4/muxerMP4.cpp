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





static    AVOutputFormat *fmt=NULL;
static    AVFormatContext *oc=NULL;
static    AVStream *audio_st;
static    AVStream *video_st;
static    double audio_pts, video_pts;

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

M4MUXERCONFIG muxerConfig=
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
    printf("[MP4] Destructing\n");
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
    {
        for(int i=0;i<nbAudioTrack;i++)
        {
            uint32_t acc=a[i]->getInfo()->encoding;
            if(acc!=WAV_MP2 && acc!=WAV_MP3 && acc!=WAV_AAC)
            {
                GUI_Error_HIG("Unsupported","Only AAC & mpegaudio supported for audio");
                return false;
            }

        }

    }
    /* All seems fine, open stuff */
    const char *f="mp4";
    if(muxerConfig.muxerType==MP4_MUXER_PSP) f="psp";
    fmt=guess_format(f, NULL, NULL);
    if(!fmt)
    {
            printf("[Mp4] guess format failed\n");
            return false;
    }
    oc = av_alloc_format_context();
	if (!oc)
	{
       		printf("[Mp4] alloc format context failed\n");
            return false;
	}
	oc->oformat = fmt;
	snprintf(oc->filename,1000,"file://%s",file);
// *******************************************
// *******************************************
//                  VIDEO
// *******************************************
// *******************************************
    video_st = av_new_stream(oc, 0);
	if (!video_st)
	{
		printf("[Mp4] new stream failed\n");
		return false;
	}
    AVCodecContext *c;
	c = video_st->codec;
    
  // probably a memeleak here
        char *foo=ADM_strdup(file);
        
        strcpy(oc->title,ADM_GetFileName(foo));
        strcpy(oc->author,"Avidemux");
        c->sample_aspect_ratio.num=1;
        c->sample_aspect_ratio.den=1;
        if(isMpeg4Compatible(s->getFCC()))
        {
                c->codec_id = CODEC_ID_MPEG4;
                if(s->providePts()==true)
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
                if(isH264Compatible(s->getFCC()))
                {
                        if(s->providePts()==true)
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
                        if(isDVCompatible(s->getFCC()))
                        {
                          c->codec_id = CODEC_ID_DVVIDEO;
                        }else
                        {
                          if(fourCC::check(s->getFCC(),(uint8_t *)"H263"))
                          {
                                    c->codec_id=CODEC_ID_H263;
                            }else{
                                    return false;
                                }
                        }
                }
        }
        uint32_t videoExtraDataSize=0;
        uint8_t  *videoExtraData;
        s->getExtraData(&videoExtraDataSize,&videoExtraData);
        if(videoExtraDataSize)
        {
                c->extradata=videoExtraData;
                c->extradata_size= videoExtraDataSize;
        }
        
        c->rc_buffer_size=8*1024*224;
        c->rc_max_rate=9500*1000;
        c->rc_min_rate=0;
//        if(!inbitrate)
                c->bit_rate=9000*1000;
        //else
          //      c->bit_rate=inbitrate;
        c->codec_type = CODEC_TYPE_VIDEO;
        c->flags=CODEC_FLAG_QSCALE;
        c->width = s->getWidth();
        c->height =s->getHeight();

        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        c->gop_size=15;
        
        

// *******************************************
// *******************************************
//                  AUDIO
// *******************************************
// *******************************************
        if(nbAudioTrack)
        {
          uint32_t audioextraSize;
          uint8_t  *audioextraData;
          
          a[0]->getExtraData(&audioextraSize,&audioextraData);

          audio_st = av_new_stream(oc, 1);
          if (!audio_st)
          {
                  printf("[MP4]: new stream failed (audio)\n");
                  return false;
          }
          WAVHeader *audioheader=a[0]->getInfo();;

          c = audio_st->codec;
          c->frame_size=1024; //For AAC mainly, sample per frame
          printf("[MP4] Bitrate %u\n",(audioheader->byterate*8)/1000);
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
                                  break;
                  default:
                                 printf("[MP4]: Unsupported audio\n");
                                 return false; 
                          break;
          }
          c->codec_type = CODEC_TYPE_AUDIO;

          c->bit_rate = audioheader->byterate*8;
          c->rc_buffer_size=(c->bit_rate/(2*8)); // 500 ms worth

          c->channels = audioheader->channels;

        }
        // /audio
        oc->mux_rate=10080*1000;
        oc->preload=AV_TIME_BASE/10; // 100 ms preloading
        oc->max_delay=200*1000; // 500 ms
        if (av_set_parameters(oc, NULL) < 0)
        {
            printf("Lav: set param failed \n");
            return false;
        }
        if (url_fopen(&(oc->pb), file, URL_WRONLY) < 0)
        {
            printf("[MP4]: Failed to open file :%s\n",file);
            return false;
        }

        ADM_assert(av_write_header(oc)>=0);
        vStream=s;
        aStreams=a;
        nbAStreams=nbAudioTrack;
        return true;
}

/**
    \fn save
*/
bool muxerMP4::save(void) 
{
    printf("[MP4] Saving\n");
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
#define AUDIO_BUFFER_SIZE 48000*6*sizeof(float)
    uint8_t *audioBuffer=new uint8_t[AUDIO_BUFFER_SIZE];


    printf("[MP4]avg fps=%u\n",vStream->getAvgFps1000());
    AVRational *scale=&(video_st->codec->time_base);
    uint64_t videoDuration=vStream->getVideoDuration();

    char *title=QT_TR_NOOP("Saving mp4");
    if(muxerConfig.muxerType==MP4_MUXER_PSP) title=QT_TR_NOOP("Saving PSP");
    encoding=createWorking(title);

    while(true==vStream->getPacket(&len, buffer, bufSize,&pts,&dts,&flags))
    {
	AVPacket pkt;

            float p=0.5;
            if(videoDuration)
                    p=lastVideoDts/videoDuration;
            p=p*100;
            encoding->update((uint32_t)p);
            if(!encoding->isAlive()) 
            {
                result=false;
                break;
            }


            aprintf("[MP5] LastDts:%08lu Dts:%08lu (%04.4lu) Delta : %u\n",lastVideoDts,dts,dts/1000000,dts-lastVideoDts);
            rawDts=dts;
            if(rawDts==ADM_NO_PTS)
            {
                lastVideoDts+=videoIncrement;
            }else
            {
                lastVideoDts=dts;
            }
            pts=rescaleLavPts(pts,scale);
            dts=rescaleLavPts(dts,scale);
            aprintf("[MP4] RawDts:%lu Scaled Dts:%lu\n",rawDts,dts);
            aprintf("[MP4] Rescaled: Len : %d flags:%x Pts:%llu Dts:%llu\n",len,flags,pts,dts);

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
            ret =av_write_frame(oc, &pkt);
            aprintf("[MP4]Frame:%u, DTS=%08lu PTS=%08lu\n",written,dts,pts);
            if(ret)
            {
                printf("[LavFormat]Error writing video packet\n");
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
                    double f=audioDts;
                    f*=fq; // In samples
                    f/=1000.*1000.; // In sec
                   
                    
                    uint64_t rescaledDts=(uint64_t)(f+0.4);
                    aprintf("[MP4] Video frame  %d, audio Dts :%lu size :%lu nbSample : %lu rescaled:%lu\n",written,audioDts,audioSize,nbSample,rescaledDts);
                    av_init_packet(&pkt);
                    pkt.dts=rescaledDts;
                    pkt.pts=rescaledDts;
                    pkt.stream_index=1+audio;
                    pkt.data= audioBuffer;
                    pkt.size= audioSize;
                    ret =av_write_frame(oc, &pkt);
                    if(ret)
                    {
                        printf("[LavFormat]Error writing audio packet\n");
                        break;
                    }
                    aprintf("%u vs %u\n",audioDts/1000,(lastVideoDts+videoIncrement)/1000);
                    if(audioDts!=ADM_NO_PTS)
                    {
                        if(audioDts>lastVideoDts+videoIncrement) break;
                    }
                }
                if(!nb) printf("[MP4] No audio for video frame %d\n",written);
            }

    }
    delete [] buffer;
    delete [] audioBuffer;
    printf("[MP4] Wrote %d frames, nb audio streams %d\n",written,nbAStreams);
    return result;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerMP4::close(void) 
{
    if(oc)
    {
        av_write_trailer(oc);
        url_fclose((oc->pb));
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
    }
    printf("[MP4] Closing\n");
    return true;
}

//EOF



