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
#include "ADM_vidMisc.h"
#include "fourcc.h"
#include "ADM_rgb.h"
#include "ADM_coreMuxerFfmpeg.h"
#include "ADM_muxerUtils.h"
#include "ADM_coreCodecMapping.h"
#include "ADM_audioXiphUtils.h"

extern "C" {
#include "libavformat/url.h"
#include "ADM_audioClock.h"
}

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif
/**
    \fn ffmpuxerSetExtradata
    \brief dupe the extradata if needed
*/
bool ffmpuxerSetExtradata(AVCodecParameters *params, int size, const uint8_t *data)
{
    if(!size || !data)
    {
        params->extradata=NULL;
        params->extradata_size=0;
        return true;
    }
    uint8_t *copy=(uint8_t *)av_malloc( (1+(size>>4))<<4);;
    memcpy(copy,data,size);
    params->extradata=copy;
    params->extradata_size=size;
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
    pkt=NULL;
    audioDelay=0;
    initialized=false;
    roundup=0;
    lavfError=0;
}
/**
    \fn closeMuxer
*/
bool muxerFFmpeg::closeMuxer()
{
    int res=0;
    if(oc)
    {
        if(initialized==true)
        {
            res=av_write_trailer(oc);
            if(res<0)
                ADM_warning("Error %d writing trailer.\n",res);
            avio_close(oc->pb);
        }
        avformat_free_context(oc);
        oc=NULL;
    }
    av_packet_free(&pkt);
    for(int i=0;i<ADM_MAX_AUDIO_STREAM;i++)
    {
        audio_st[i]=NULL;
    }
    video_st=NULL;
    return res >= 0;
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
#if defined(__APPLE__)
 #define MAX_LEN 1024
#else
 #define MAX_LEN 4096
#endif
    uint32_t len=strlen(filename);
    if(len>MAX_LEN)
    {
        ADM_error("Filename length %u exceeds limit %u\n",len,MAX_LEN);
        return false;
    }
    char *url=(char *)ADM_alloc(len+8);
    snprintf(url,len+8,"file://%s",filename);
    url[len+7]=0;
    oc->url=url;
//#warning use AV METADATA
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
    printf("[muxerFFmpeg::initVideo] Initial audio delay: %" PRIu64" ms\n",audioDelay/1000);
    video_st = avformat_new_stream(oc, NULL);
    if (!video_st)
    {
        ADM_error("Cannot allocate new stream\n");
        return false;
    }
    if(!pkt)
    {
        pkt = av_packet_alloc();
        if(!pkt)
        {
            ADM_error("Cannot allocate AVPacket\n");
            return false;
        }
    }
    AVCodecParameters *par = video_st->codecpar;
    par->sample_aspect_ratio.num=1;
    par->sample_aspect_ratio.den=1;
    video_st->sample_aspect_ratio=par->sample_aspect_ratio;
    //par->bit_rate=9000*1000;
    par->codec_type = AVMEDIA_TYPE_VIDEO;
    par->width = stream->getWidth();
    par->height =stream->getHeight();

    {
        uint32_t r,p,t,m;
        if(stream->getColorInfo(&r,&p,&t,&m))
        {
            par->color_range = (AVColorRange) validateColorRange(r);
            par->color_primaries = (AVColorPrimaries) validateColorPrimaries(p);
            par->color_trc = (AVColorTransferCharacteristic) validateColorTrC(t);
            par->color_space = (AVColorSpace) validateColorSpace(m);
        }
    }

    uint32_t videoExtraDataSize=0;
    uint8_t *videoExtraData = NULL;
    stream->getExtraData(&videoExtraDataSize,&videoExtraData);
    printf("[FF] Using %d bytes for video extradata\n",(int)videoExtraDataSize);
    ffmpuxerSetExtradata(par,videoExtraDataSize,videoExtraData);

    uint32_t fcc=stream->getFCC();

    if(isMpeg4Compatible(fcc))
        par->codec_id = AV_CODEC_ID_MPEG4;
    else if(isH264Compatible(fcc))
        par->codec_id = AV_CODEC_ID_H264;
    else if(isH265Compatible(fcc))
        par->codec_id = AV_CODEC_ID_HEVC;
    else if(fourCC::check(fcc, (uint8_t *)"MPEG"))
        par->codec_id = AV_CODEC_ID_MPEG2VIDEO;
    else if(fourCC::check(fcc, (uint8_t *)"mp1v"))
        par->codec_id = AV_CODEC_ID_MPEG1VIDEO;
    else if(isDVCompatible(fcc))
        par->codec_id = AV_CODEC_ID_DVVIDEO;
    else if(fourCC::check(fcc, (uint8_t *)"H263"))
        par->codec_id = AV_CODEC_ID_H263;
    else if(isVP6Compatible(fcc))
        par->codec_id = AV_CODEC_ID_VP6F;
    else if(fourCC::check(fcc, (uint8_t *)"FLV1"))
        par->codec_id = AV_CODEC_ID_FLV1;
    else
    {
        AVCodecID cid = ADM_codecIdFindByFourcc(fourCC::tostring(fcc));
        if(cid == AV_CODEC_ID_NONE)
        {
             printf("[muxerFFmpeg::initVideo] Unknown video codec \"%s\"\n", fourCC::tostring(fcc));
             return false;
        }
        par->codec_id = cid;
        par->codec_tag = fcc;
    }
    if(useGlobalHeader())
    {
        if(videoExtraDataSize)
            ADM_info("Video has extradata and muxer requires global header, assuming it is done so.\n");
        else
            ADM_warning("Video has no extradata, but muxer expects global header.\n");
    }

    lavfError = 0;
    printf("[muxerFFmpeg::initVideo] Video initialized\n");

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
        ADM_info("FF: No audio\n");
        return true;
    }
    
    for(int i=0;i<nbAudioTrack;i++)
    {
        uint32_t audioextraSize = 0;
        uint8_t *audioextraData = NULL;

        audio[i]->getExtraData(&audioextraSize,&audioextraData);

        audio_st[i] = avformat_new_stream(oc, NULL);
        if (!audio_st[i])
        {
            ADM_error("Cannot allocate new stream for audio track %d / %u\n", i, nbAudioTrack);
            return false;
        }
        WAVHeader *audioheader = audio[i]->getInfo();
        AVCodecParameters *par = audio_st[i]->codecpar;
        par->frame_size = 1024; //For AAC mainly, sample per frame
        ADM_info("Track %d bitrate: %u kbps\n", i, (audioheader->byterate*8)/1000);
        par->sample_rate = audioheader->frequency;
        par->channel_layout = av_get_default_channel_layout(audioheader->channels);
        ADM_info("Using default channel layout 0x%lx for %u channels\n",par->channel_layout,audioheader->channels);
        switch(audioheader->encoding)
        {
            case WAV_OGG_VORBIS:
                par->codec_id = AV_CODEC_ID_VORBIS;
                par->frame_size = 6*256;
                if(!strcmp(fmt->name,"mp4") || !strcmp(fmt->name,"psp")) // Need to translate from adm to xiph
                {
                    int xiphLen=(int)audioextraSize+(audioextraSize/255)+4+5;
                    uint8_t *xiph=new uint8_t[xiphLen+AV_INPUT_BUFFER_PADDING_SIZE];
                    memset(xiph,0,xiphLen+AV_INPUT_BUFFER_PADDING_SIZE);
                    xiphLen=ADMXiph::admExtraData2xiph(audioextraSize,audioextraData,xiph);
                    ffmpuxerSetExtradata(par,xiphLen,xiph);
                    delete [] xiph;
                    xiph=NULL;
                }else
                {
                    ffmpuxerSetExtradata(par,audioextraSize,audioextraData);
                }
                break;
            case WAV_FLAC:
                par->codec_id = AV_CODEC_ID_FLAC;
                // Do we already have the flac header ? FFmpeg will add it..
                // If we have it, skip it
                if(audioextraSize>=8 && audioextraData[0]==0x66 && audioextraData[1]==0x4c &&audioextraData[2]==0x61 && audioextraData[3]==0x43)
                    ffmpuxerSetExtradata(par,audioextraSize-8,audioextraData+8);
                else
                    ffmpuxerSetExtradata(par,audioextraSize,audioextraData);
                break;
            case WAV_DTS:
                par->codec_id = AV_CODEC_ID_DTS;
                break;
            case WAV_OPUS:
                par->codec_id = AV_CODEC_ID_OPUS;
                ffmpuxerSetExtradata(par,audioextraSize,audioextraData);
                break;
            case WAV_EAC3:
                par->codec_id = AV_CODEC_ID_EAC3;
                par->frame_size = 6*256;
                break;
            case WAV_AC3:
                par->codec_id = AV_CODEC_ID_AC3;
                par->frame_size = 6*256;
                break;
            case WAV_MP2:
                par->codec_id = AV_CODEC_ID_MP2;
                par->frame_size = 1152;
                break;
            case WAV_MP3:
//  #warning FIXME : Probe deeper
                par->codec_id = AV_CODEC_ID_MP3;
                par->frame_size = 1152;
                break;
            case WAV_LPCM:
            case WAV_PCM:
                //par->frame_size=4; // One chunk is 10 ms (1/100 of fq)
                par->frame_size = 0; // so does ffmpeg
                par->block_align = audioheader->blockalign;
                {
                    bool bigEndian = audioheader->encoding == WAV_LPCM;
                    switch(audioheader->bitspersample)
                    {
                        case 16: par->codec_id = bigEndian? AV_CODEC_ID_PCM_S16BE : AV_CODEC_ID_PCM_S16LE; break;
                        case 24: par->codec_id = bigEndian? AV_CODEC_ID_PCM_S24BE : AV_CODEC_ID_PCM_S24LE; break;
                        default: ADM_error("Track %d / %u: unsupported bits per sample value %u for PCM\n", i, nbAudioTrack, audioheader->bitspersample); return false;
                    }
                }
                break;
            case WAV_AAC:
            case WAV_AAC_HE:
                par->codec_id = AV_CODEC_ID_AAC;
                par->frame_size = audio[i]->getSamplesPerPacket();
                ffmpuxerSetExtradata(par,audioextraSize,audioextraData);
                break;
            case WAV_TRUEHD:
                par->codec_id = AV_CODEC_ID_TRUEHD;
                par->frame_size = 40;
                break;
            default:
                ADM_error("Unsupported audio for track %d / %u\n", i, nbAudioTrack);
                return false;
        } // switch(audioheader->encoding)
        par->codec_type = AVMEDIA_TYPE_AUDIO;
        par->bit_rate = audioheader->byterate*8;
        par->channels = audioheader->channels;
        if(useGlobalHeader())
        {
            if(audioextraSize)
                ADM_info("Audio track %d / %u has extradata and muxer requires global header, assuming it is done so.\n", i, nbAudioTrack);
            else
                ADM_warning("Audio track %d / %u has no extradata, but muxer requires global header.\n", i, nbAudioTrack);
        }
        //set language
        const std::string lang = audio[i]->getLanguage();
        if(lang.size())
        {
            AVDictionary *dict = NULL;
            av_dict_set(&dict, "language", lang.c_str(), 0);
            audio_st[i]->metadata = dict;
            ADM_info("Language for track %d is %s\n",i,lang.c_str());
        }
    }
    ADM_info("FF: Audio initialized, %u tracks.\n", nbAudioTrack);
    return true;
}
#define AUDIO_BUFFER_SIZE 48000*6*sizeof(float)
/**
 * \class MuxAudioPacket
 */
class MuxAudioPacket
{
public:
    MuxAudioPacket() {eof=false;dts=ADM_NO_PTS;present=false;size=0;clock=NULL;}
    ~MuxAudioPacket() 
    {
        if(clock) 
        {
            delete clock;
            clock=NULL;
        }
    }
    uint8_t     buffer[AUDIO_BUFFER_SIZE];
    uint32_t    size;
    bool        eof;
    bool        present;
    uint64_t    dts;
    uint32_t    samples;
    audioClock  *clock;
};

/**
    \fn saveLoop
*/
bool muxerFFmpeg::saveLoop(const char *title)
{
    printf("[FF] Saving\n");
    ADM_assert(pkt);
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;
    uint8_t *buffer=new uint8_t[bufSize];
    uint64_t rawDts;
    uint64_t lastVideoDts=0;
    uint64_t lastVideoDtsLav=0;
    uint64_t videoIncrement;
    bool ret=true;
    uint32_t written=0;
    bool result=true;
    bool gotVideoPacket=true;
    int missingPts=0;
    
    float f=(float)vStream->getAvgFps1000();
    f=1000./f;
    f*=1000000;
    videoIncrement=(uint64_t)f;



    ADM_info("avg fps=%u\n",vStream->getAvgFps1000());
    uint64_t videoDuration=vStream->getVideoDuration();

    initUI(QT_TRANSLATE_NOOP("adm","Saving"));
    encoding->setContainer(getContainerName());
    MuxAudioPacket *audioPackets=new MuxAudioPacket[nbAStreams];
    for(int i=0;i<nbAStreams;i++) // ugly...
        audioPackets[i].clock=new audioClock(aStreams[i]->getInfo()->frequency);
    ADMBitstream out(bufSize);
    out.data=buffer;

    while(gotVideoPacket)
    {
        gotVideoPacket=vStream->getPacket(&out);
        uint64_t maxAudioDts;
        if(gotVideoPacket)
        {
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
            aprintf("[FF:V] Pts: %" PRId64" DTS:%" PRId64" ms\n",xpts/1000,xdts/1000);

            aprintf("[FF:V] LastDts:%08" PRIu64" Dts:%08" PRIu64" (%04" PRIu64") Delta : %" PRIu64"\n",
                lastVideoDts,out.dts,out.dts/1000000,out.dts-lastVideoDts);
            rawDts=out.dts;
            if(rawDts==ADM_NO_PTS)
            {
                ADM_warning("No DTS information for frame %" PRIu32"\n",written);
                lastVideoDts+=videoIncrement;
            }else if(lastVideoDts && rawDts<lastVideoDts)
            {
                ADM_warning("Duplicated or going back DTS for frame %" PRIu32"\n",written);
                out.dts=lastVideoDts;
            }else
            {
                lastVideoDts=out.dts;
            }
            maxAudioDts=lastVideoDts+audioDelay;
        }else
        {
            maxAudioDts=videoDuration;
        }

        if(written)
        {
            // Now send audio until they all have DTS > lastVideoDts
            for(int audio=0;audio<nbAStreams;audio++)
            {
                MuxAudioPacket *audioTrack=&(audioPackets[audio]);
                ADM_audioStream*a=aStreams[audio];
                WAVHeader *info=a->getInfo();
                if(!info) // no more track
                    continue;

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
                       // printf("Track %d , new audio packet DTS=%"PRId64" size=%"PRIu32"\n",audioTrack->dts,audioTrack->size);
                        audioTrack->present=true;
                        // Delay audio by the delay induce by encoder
                        if(audioTrack->dts!=ADM_NO_PTS) audioTrack->dts+=audioDelay;
                    }
                    if(audioTrack->dts!=ADM_NO_PTS)
                    {
                        //printf("Audio PTS:%"PRId64", limit=%"PRId64"\n",audioTrack->dts,lastVideoDts+videoIncrement);
                        if(audioTrack->dts>maxAudioDts) break; // This packet is in the future
                    }
                    // Write...
                    uint64_t rescaledDts;
                    rescaledDts=audioTrack->dts;
                    if(rescaledDts==ADM_NO_PTS)
                        rescaledDts=audioTrack->clock->getTimeUs(); // we assume the 1st one has a PTS/DTS..., can we ?
                    else
                        audioTrack->clock->setTimeUs(rescaledDts);
                    audioTrack->clock->advanceBySample(audioTrack->samples);
                    encoding->pushAudioFrame(audioTrack->size);
                    muxerRescaleAudioTime(audio,&rescaledDts,a->getInfo()->frequency);
                   //printf("[FF] A: Video frame  %d, audio Dts :%"PRIu64" size :%"PRIu32" nbSample : %"PRIu32" rescaled:%"PRIu64"\n",
                     //               written,audioTrack->dts,audioTrack->size,audioTrack->samples,rescaledDts);
                    av_packet_unref(pkt);

                    pkt->dts = rescaledDts;
                    pkt->pts = rescaledDts;
                    pkt->stream_index = 1+audio;
                    pkt->data = audioTrack->buffer;
                    pkt->size = audioTrack->size;
                    pkt->flags |= AV_PKT_FLAG_KEY; // Assume all audio are keyframe, which is slightly wrong

                    lavfError = av_write_frame(oc,pkt);

                    audioTrack->present=false; // consumed
                    if(lavfError)
                    {
                        ret = false;
                        if(lavfError < 0)
                        {
                            char er[AV_ERROR_MAX_STRING_SIZE]={0};
                            av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, lavfError);
                            printf("[muxerFFmpeg::saveLoop] Error %d (%s) writing audio packet\n",lavfError,er);
                        }
                        lavfError = 0; // errors from writing audio packets are nonfatal
                        break;
                    }
                   // printf("[FF] A:%"PRIu32" ms vs V: %"PRIu32" ms\n",(uint32_t)audioTrack->dts/1000,(uint32_t)(lastVideoDts+videoIncrement)/1000);
                }
            }
        }

        if(!gotVideoPacket) break;

        ret = true; // ignore possible errors from writing audio packets
        if(out.pts==ADM_NO_PTS)
        {
            ADM_warning("No PTS information for frame %" PRIu32"\n",written);
            missingPts++;
            out.pts=lastVideoDts;
        }
        if(out.pts<lastVideoDts)
        {
            ADM_warning("Bumping PTS to keep PTS >= DTS for frame %" PRIu32"\n",written);
            out.pts=lastVideoDts;
        }

        encoding->pushVideoFrame(out.len,out.out_quantizer,lastVideoDts);
        muxerRescaleVideoTimeDts(&(out.dts),lastVideoDts);
        if(!roundup && lastVideoDtsLav && out.dts==lastVideoDtsLav)
        {
            ADM_warning("Bumping lav DTS to avoid collision for frame %" PRIu32"\n",written);
            out.dts++;
        }
        lastVideoDtsLav=out.dts;
        muxerRescaleVideoTime(&(out.pts));
        if(out.dts>out.pts)
        {
            ADM_warning("Bumping lav PTS to keep PTS >= DTS for frame %" PRIu32"\n",written);
            out.pts=out.dts;
        }
        aprintf("[FF:V] RawDts:%lu Scaled Dts:%lu\n",rawDts,out.dts);
        aprintf("[FF:V] Rescaled: Len : %d flags:%x Pts:%" PRIu64" Dts:%" PRIu64"\n",out.len,out.flags,out.pts,out.dts);

        av_packet_unref(pkt);
        pkt->dts = out.dts;
        if(vStream->providePts()==true)
        {
            pkt->pts = out.pts;
        }else
        {
            pkt->pts = pkt->dts;
        }
        pkt->stream_index = 0;
        pkt->data = buffer;
        pkt->size = out.len;
        if(out.flags & 0x10) // FIXME AVI_KEY_FRAME
            pkt->flags |= AV_PKT_FLAG_KEY;

        lavfError = av_write_frame(oc,pkt);

        aprintf("[FF]Frame:%u, DTS=%08lu PTS=%08lu\n",written,out.dts,out.pts);
        if(lavfError)
        {
            ret = false;
            if(lavfError < 0)
            {
                char er[AV_ERROR_MAX_STRING_SIZE]={0};
                av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, lavfError);
                printf("[muxerFFmpeg::saveLoop] Error %d (%s) writing audio packet\n",lavfError,er);
            }
            break;
        }
        if(!written)
        {
            audioDelay=vStream->getVideoDelay();
            printf("[muxerFFmpeg::saveLoop] Final audio delay: %" PRIu64" ms\n",audioDelay/1000);
        }
        written++;
    }
    delete [] buffer;
    if(false==ret)
    {
        char msg[512 + AV_ERROR_MAX_STRING_SIZE];
        snprintf(msg,512,QT_TRANSLATE_NOOP("adm",
            "The saved video is incomplete. "
            "The error occured at %s (%d\%). "
            "This may happen as result of invalid time stamps in the video."),
            ADM_us2plain(lastVideoDts),
            (int)(lastVideoDts*100/videoDuration));
        if(lavfError < 0)
        { // TODO: Replace hardcoded guess above with a more sensible solution.
            char *s = msg;
            s += strlen(msg);
            int left = 512 - strlen(msg);
            left += AV_ERROR_MAX_STRING_SIZE;
            char er[AV_ERROR_MAX_STRING_SIZE]={0};
            av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, lavfError);
            snprintf(s,left,"\n\nError %d (\"%s\")",lavfError,er);
        }
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Too short"),msg);
        result=false;
    }
    ADM_info("[FF] Wrote %d frames, nb audio streams %d\n",written,nbAStreams);
    ADM_info("[FF] Found %d missing PTS / %d total frames\n",missingPts,written);
    delete [] audioPackets;
    audioPackets=NULL;
    return result;
}
// EOF
