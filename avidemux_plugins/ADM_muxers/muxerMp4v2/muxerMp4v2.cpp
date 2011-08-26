/**********************************************************************
            \file            muxerMp4v2
            \brief           libmp4v2 muxer
                             -------------------
    
    copyright            : (C) 2011 by mean
    email                : fixounet@free.fr
    Strongly inspired by handbrake code

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
#include "ADM_cpp.h"
#include "fourcc.h"
#include "muxerMp4v2.h"
#include "ADM_codecType.h"
#include "ADM_imageFlags.h"

#if 1
#define aprintf(...) {}
#define cprintf(...) {}
#define MP4_DEBUG 0
#else
#define aprintf printf
#define MP4_DEBUG MP4_DETAILS_ALL

#endif

#define ADM_MP4_OPTIONS_OPEN  (MP4_CREATE_64BIT_DATA)
#define ADM_MP4_OPTIONS_CLOSE (MP4_CLOSE_DO_NOT_COMPUTE_BITRATE )

mp4v2_muxer muxerConfig=
{
   1, // uint32_t optimize;
   0  //uint32_t add_itunes_metadata;
};
/**
    \fn setMaxDurationPerChunk
    \brief Max chunk duration is 1 sec par default; set it to ~ 4 frames
*/
bool muxerMp4v2::setMaxDurationPerChunk(MP4TrackId track, uint32_t samples)
{
    uint32_t trackScale=MP4GetTrackTimeScale(handle,track);
    uint32_t   mx;
    mx=4*samples;
    ADM_info("Setting max chunk duration =%d; scale=%d for track %d\n",(int)mx,(int)trackScale,(int)track);
    if(!MP4SetTrackDurationPerChunk(handle,track,mx))
    {
        ADM_error("Cannot set TrackDurationPerChunk\n");
        return false;
    }
    return true;
}
/**
    \fn timeScale
    \brief convert our unit (us) to mp4v2 unit (90khz tick)
*/
uint64_t muxerMp4v2::timeScale(uint64_t timeUs)
{
    return (uint64_t)((timeUs*90LL)/1000LL);

}
/**
    \fn inverseTimeScale
    \brief convert tick unit (90kHz) to our unit (us)
*/
uint64_t muxerMp4v2::inverseTimeScale(uint64_t timeTick)
{
    return (uint64_t)((timeTick*1000LL)/90LL);
}

/**
    \fn     muxerMp4v2
    \brief  Constructor
*/
muxerMp4v2::muxerMp4v2()
{
        ADM_info("[Mp4v2Muxer] Creating\n");
        handle=NULL;
        audioTrackIds=NULL;
        videoBuffer[0]=NULL;
        videoBuffer[1]=NULL;
        scratchBuffer=NULL;
        nextWrite=0;
        needToConvertFromAnnexB=false;
        lastVideoDts=0;
};
/**
    \fn     muxerMp4v2
    \brief  Destructor
*/

muxerMp4v2::~muxerMp4v2()
{
    ADM_info("[Mp4v2Muxer] Destroying\n");
    close();
    if(handle)
        ADM_error("MP4V2: File still opened\n");
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerMp4v2::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{

        audioDelay=s->getVideoDelay();
        vStream=s;
        nbAStreams=nbAudioTrack;
        aStreams=a;
        videoBufferSize=vStream->getWidth()*vStream->getHeight()*3;
        videoBuffer[0]=new uint8_t[videoBufferSize];
        videoBuffer[1]=new uint8_t[videoBufferSize];
        scratchBuffer=new uint8_t[videoBufferSize];
        in[0].bufferSize=videoBufferSize;
        in[0].data=videoBuffer[0];
        in[1].bufferSize=videoBufferSize;
        in[1].data=videoBuffer[1];
        targetFileName=string(file);
//------Verify everything is ok : Accept Mp4 & H264 for video, AAC for audio ----
        uint32_t fcc=vStream->getFCC();
        if(!isH264Compatible(fcc) && !isMpeg4Compatible(fcc))
        {
            ADM_error("[mp4v2] Only h264 and mp4 video track!\n");
            return false;
        }
        for(int i=0;i<nbAStreams;i++)
        {
            int encoding=aStreams[i]->getInfo()->encoding;
            switch(encoding)
            {
                case WAV_MP2:case WAV_MP3:case WAV_AAC:case WAV_AC3:
                            continue;
                default:
                    GUI_Error_HIG("Audio","Audio format not supported, only AAC/MP3/AC3");
                    return false;
            }            
        }
//------Verify everything is ok : Accept Mp4 & H264 for video, AAC for audio ----
        
        // Create file
        handle=MP4Create( file,  ADM_MP4_OPTIONS_OPEN);
        if(MP4_INVALID_FILE_HANDLE==handle)
        {
            ADM_error("[mp4v2]Cannot create output file %s\n",file);
            return false;
        }
        MP4LogSetLevel(MP4_LOG_INFO);
        
        if (!(MP4SetTimeScale( handle, 90*1000 ))) // 90 kHz tick
        {
            ADM_error("[mp4v2]Cannot set timescale to us\n");
            return false;
        }
        if(false==initAudio())
        {
            ADM_error("Cannot init audio\n");
            return false;
        }
        if(false==initVideo())
        {
            ADM_error("Cannot init video\n");
            return false;
        }

        return true;
er:
        return false;
}
/**
    \fn save
*/
bool muxerMp4v2::save(void)
{
    bool result=true;
    int nbFrame=0;
    printf("[Mp4v2Muxer] Saving\n");
   

    initUI("Saving MP4V2");
    encoding->setContainer("MP4 (libmp4v2)");
    uint64_t lastSentDts=0;
    
    while(loadNextVideoFrame((&(in[nextWrite])))) 
    {
        bool kf=false;
        int other=!nextWrite;
        if(in[other].flags & AVI_KEY_FRAME) kf=true;

        ADM_assert(in[nextWrite].dts!=ADM_NO_PTS)
        ADM_assert(in[nextWrite].dts!=ADM_NO_PTS)
        if(in[other].pts==ADM_NO_PTS || in[other].pts==ADM_NO_PTS)
        {
            GUI_Error_HIG("Video","Video does not have enough timing information. Are you copying from AVI ?");
            goto theEnd;
        }

        

        uint64_t nextDts=in[nextWrite].dts;   // Delta between dts=duration of the frame (sort of)     
        uint64_t myDts=in[other].dts;
        uint64_t myPts=in[other].pts;        
        cprintf(">>next DTS=%"LLU", last DTS=%"LLU"delta=%"LLU"\n",nextDts,lastSentDts,nextDts-lastSentDts);

        encoding->pushVideoFrame(in[other].len,in[other].out_quantizer,myDts);
        uint64_t delta=myPts-lastSentDts; // composition time...
        delta=timeScale(delta);
        uint64_t duration=nextDts-lastSentDts;

        uint64_t scaled_duration=timeScale(duration);
        duration=inverseTimeScale(scaled_duration); // handle rounding error
        nbFrame++;
        cprintf("Sending frame duration=%"LLU", pts/dts=%"LLU"\n",lastSentDts,delta);
        if(false==MP4WriteSample(handle,videoTrackId,in[other].data,in[other].len,
                        scaled_duration, // duration
                        delta, // pts/dts offset
                        kf // Sync Sample
                        ))
        {
            ADM_error("Cannot write video sample\n");
            result=false;
            goto theEnd;
        }
        // update lastSentDts
        lastSentDts+=duration; // beginning of next frame...
        //
        cprintf("lastSendDts=%"LLU", next Dts=%"LLU", skew=%"LLD"\n",lastSentDts,nextDts,
                        (int64_t)nextDts-(int64_t)lastSentDts);
        //
        fillAudio(lastSentDts);
        // toggle
        nextWrite=other;
        if(updateUI()==false)
            {  
                result=false;
                break;
            }
    }
    // Write last frame
    nextWrite=!nextWrite;
    MP4WriteSample(handle,videoTrackId,in[nextWrite].data,in[nextWrite].len,
                        90000/100, // duration=10ms
                        0, // pts/dts offset
                        0 // Sync Sample
                        );
theEnd:
    closeUI();
    close();
    if(muxerConfig.optimize && result==true)
    {
        string tmpTargetFileName=targetFileName+string(".tmp");
        if(!ADM_renameFile(targetFileName.c_str(),tmpTargetFileName.c_str()))
        {
            GUI_Error_HIG("","Cannot rename file (optimize)");
            return false;
        }
        // Optimize
        ADM_info("Optimizing...\n");
        MP4Optimize( tmpTargetFileName.c_str(), targetFileName.c_str() );
        // delete
        unlink(tmpTargetFileName.c_str());
    }
    return result;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerMp4v2::close(void)
{
    if(handle)
    {
            MP4Close(handle,ADM_MP4_OPTIONS_CLOSE);
#warning run MP4Optimize
    }
    handle=NULL;
    if(audioTrackIds) delete [] audioTrackIds;
    audioTrackIds=NULL;
    if(audioPackets) delete [] audioPackets;
    audioPackets=NULL;
    for(int i=0;i<2;i++)
    {
        if(videoBuffer[i]) delete [] videoBuffer[i];
        videoBuffer[i]=NULL;
    }
    if(scratchBuffer)
        {
            delete [] scratchBuffer;
            scratchBuffer=NULL;
        }
    ADM_info("[Mp4v2Muxer] Closing\n");
    return true;
}
//EOF



