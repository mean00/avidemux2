/***************************************************************************
            \file            muxerFlv
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
#include "muxerFlv.h"
#include "DIA_coreToolkit.h"
#include "ADM_muxerUtils.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

FLVMUXERCONFIG muxerConfig=
{
    0
};


/**
    \fn     muxerFlv
    \brief  Constructor
*/
muxerFlv::muxerFlv()
{
};
/**
    \fn     muxerFlv
    \brief  Destructor
*/

muxerFlv::~muxerFlv()
{
    printf("[FLV] Destructing\n");
    closeMuxer();

}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerFlv::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
    uint32_t fcc=s->getFCC();
    bool r=true;
    char *fileTitle=NULL;
        
     if(fourCC::check(fcc,(uint8_t *)"FLV1") || isVP6Compatible(fcc))
     {

     }else
        {
            GUI_Error_HIG("Unsupported","Only FLV1 & VP6 supported for video");
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

    if(false==setupMuxer("flv",file))
    {
        printf("[FLV] Failed to open muxer\n");
        return false;
    }
 
   if(initVideo(s)==false) 
    {
        printf("[FLV] Failed to init video\n");
        return false;
    }
  
    AVCodecContext *c;
    c = video_st->codec;   
    rescaleFps(s->getAvgFps1000(),&(c->time_base));
    c->gop_size=15;

    if(initAudio(nbAudioTrack,a)==false)
    {
        printf("[FLV] Failed to init audio\n");
        return false;
    }

        
        oc->mux_rate=10080*1000;
        oc->preload=AV_TIME_BASE/10; // 100 ms preloading
        oc->max_delay=200*1000; // 500 ms
        if (av_set_parameters(oc, NULL) < 0)
        {
            printf("[FLV]: set param failed \n");
            return false;
        }
        if (url_fopen(&(oc->pb), file, URL_WRONLY) < 0)
        {
            printf("[FLV]: Failed to open file :%s\n",file);
            r=false;
            goto finish;
        }

        if(av_write_header(oc)<0)
        {
            printf("[Flv Muxer] Muxer rejected the parameters\n");
            r=false;
            
        }
finish:
        vStream=s;
        aStreams=a;
        nbAStreams=nbAudioTrack;
        return r;
}

/**
    \fn save
*/
bool muxerFlv::save(void)
{
    printf("[FLV] Saving\n");
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;
    uint8_t *buffer=new uint8_t[bufSize];
    uint32_t len,flags;
    uint64_t pts,dts,rawDts;
    uint64_t lastVideoDts=0;
    int ret;
    bool result=true;
    int written=0;
#define AUDIO_BUFFER_SIZE 48000*6*sizeof(float)
    uint8_t *audioBuffer=new uint8_t[AUDIO_BUFFER_SIZE];


    printf("[FLV]avg fps=%u\n",vStream->getAvgFps1000());
    AVRational *scale=&(video_st->codec->time_base);
    initUI("Saving Flv file..");
    while(true==vStream->getPacket(&len, buffer, bufSize,&pts,&dts,&flags))
    {
	AVPacket pkt;
            printf("[MP5] LastDts:%08"LLU" Dts:%08"LLU" (%04"LLU") Delta : %"LLD"\n",lastVideoDts,dts,dts/1000000,dts-lastVideoDts);
            rawDts=dts;
            if(rawDts==ADM_NO_PTS)
            {
                lastVideoDts+=videoIncrement;
            }else
            {
                lastVideoDts=dts;
            }
            if(false==updateUI(lastVideoDts))
            {
                result=false;
                goto abt;
             }
#define RESCALE(x) x=rescaleLavPts(x,scale);

            dts=lastVideoDts/1000;

            aprintf("[FLV] RawDts:%lu Scaled Dts:%lu\n",rawDts,dts);
            aprintf("[FLV] Rescaled: Len : %d flags:%x Pts:%llu Dts:%llu\n",len,flags,pts,dts);
            RESCALE(pts);
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
            aprintf("[FLV]Frame:%u, DTS=%08lu PTS=%08lu\n",written,dts,pts);
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
                uint64_t audioDts,rescaled;
                ADM_audioStream*a=aStreams[audio];
                uint32_t fq=a->getInfo()->frequency;
                int nb=0;
                while(a->getPacket(audioBuffer,&audioSize, AUDIO_BUFFER_SIZE,&nbSample,&audioDts))
                {
                    // Write...
                    nb++;
                    AVPacket pkt;
                    rescaled=audioDts/1000;

                    aprintf("[FLV] Video frame  %d, audio Dts :%lu size :%lu nbSample : %lu rescaled:%lu\n",written,audioDts,audioSize,nbSample,rescaledDts);
                    av_init_packet(&pkt);
                    pkt.dts=rescaled;
                    pkt.pts=rescaled;
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
                if(!nb) printf("[FLV] No audio for video frame %d\n",written);
            }

    }
abt:
    closeUI();
    delete [] buffer;
    delete [] audioBuffer;
    printf("[FLV] Wrote %d frames, nb audio streams %d\n",written,nbAStreams);
    return result;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerFlv::close(void)
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
    printf("[FLV] Closing\n");
    return true;
}

//EOF



