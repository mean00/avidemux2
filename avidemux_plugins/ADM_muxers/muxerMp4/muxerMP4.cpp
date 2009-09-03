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

#if 0
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
        for(int i=0;i<nbAudioTrack;i++)
        {
            uint32_t acc=a[i]->getInfo()->encoding;
            if(acc!=WAV_MP2 && acc!=WAV_MP3 && acc!=WAV_AAC)
            {
                GUI_Error_HIG("Unsupported","Only AAC & mpegaudio supported for audio");
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
        c = video_st->codec;
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
    

        rescaleFps(s->getAvgFps1000(),&(c->time_base));
        c->gop_size=15;
        
        if(initAudio(nbAudioTrack,a)==false)
        {
            printf("[MP4] Failed to init audio\n");
            return false;
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

    const char *title=QT_TR_NOOP("Saving mp4");
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
            int64_t xpts=(int64_t)pts;
            int64_t xdts=(int64_t)dts;
            if(pts==ADM_NO_PTS) xpts=-1;
            if(dts==ADM_NO_PTS) xdts=-1;
            aprintf("[MP4:V] Pts: %"LLD" DTS:%"LLD" ms\n",xpts/1000,xdts/1000);

            aprintf("[MP4:V] LastDts:%08"LLU" Dts:%08"LLU" (%04"LLU") Delta : %"LLU"\n",
                        lastVideoDts,dts,dts/1000000,dts-lastVideoDts);
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
            aprintf("[MP4:V] RawDts:%lu Scaled Dts:%lu\n",rawDts,dts);
            aprintf("[MP4:V] Rescaled: Len : %d flags:%x Pts:%"LLU" Dts:%"LLU"\n",len,flags,pts,dts);

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
                    aprintf("[MP4] A: Video frame  %d, audio Dts :%"LLU" size :%"LU" nbSample : %"LU" rescaled:%"LLU"\n",
                                    written,audioDts,audioSize,nbSample,rescaledDts);
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
                    aprintf("[MP4] A:%"LU" ms vs V: %"LU" ms\n",(uint32_t)audioDts/1000,(uint32_t)(lastVideoDts+videoIncrement)/1000);
                    if(audioDts!=ADM_NO_PTS)
                    {
                        if(audioDts>lastVideoDts+videoIncrement) break;
                    }
                }
                if(!nb) printf("[MP4] A: No audio for video frame %d\n",written);
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
   
    printf("[MP4] Closing\n");
    return closeMuxer();
}

//EOF



