/**********************************************************************
            \file            muxerAvi
            \brief           Avi openDML muxer
                             -------------------
    TODO: Fill in drops/holes in audio as for video
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
#include "muxerAvi.h"
#include "DIA_coreToolkit.h"
#include "DIA_encoding.h"
//#include "DIA_encoding.h"

#define ADM_NO_PTS 0xFFFFFFFFFFFFFFFFLL // FIXME
#define AUDIO_BUFFER_SIZE 48000*6*sizeof(float)
// Fwd ref
uint8_t isMpeg4Compatible (uint32_t fourcc);
uint8_t isH264Compatible (uint32_t fourcc);
uint8_t isMSMpeg4Compatible (uint32_t fourcc);
uint8_t isDVCompatible (uint32_t fourcc);

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

AVIMUXERCONFIG muxerConfig=
{
    HIDDEN
};


/**
    \fn     muxerMP4
    \brief  Constructor
*/
muxerAvi::muxerAvi()
{
    audioBuffer=NULL;
    videoBuffer=NULL;
    clocks=NULL;
};
/**
    \fn     muxerMP4
    \brief  Destructor
*/

muxerAvi::~muxerAvi()
{
    printf("[AVI] Destructing\n");
    if(clocks)
    {
        for(int i=0;i<nbAStreams;i++)
            delete clocks[i];
        delete [] clocks;
        clocks=NULL;
    }
}

/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerAvi::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{

        if(!writter.saveBegin (
             file,
		     s,
             nbAudioTrack,
             a))
        {
            GUI_Error_HIG("Error","Cannot create avi file");
            return false;

        }
        vStream=s;
        nbAStreams=nbAudioTrack;
        aStreams=a;
        clocks=new audioClock*[nbAStreams];
        for(int i=0;i<nbAStreams;i++)
            clocks[i]=new audioClock(a[i]->getInfo()->frequency);

        return true;
}
/**
        \fn fillAudio
        \brief Put audio datas until targetDts is reached
*/
bool muxerAvi::fillAudio(uint64_t targetDts)
{
// Now send audio until they all have DTS > lastVideoDts+increment
            for(int audioIndex=0;audioIndex<nbAStreams;audioIndex++)
            {
                uint32_t audioSize,nbSample;
                uint64_t audioDts;
                ADM_audioStream*a=aStreams[audioIndex];
                uint32_t fq=a->getInfo()->frequency;
                int nb=0;
                audioClock *clk=clocks[audioIndex];

                while(a->getPacket(audioBuffer,&audioSize, AUDIO_BUFFER_SIZE,&nbSample,&audioDts))
                {
                    printf("[Audio] Packet size %"LU" sample:%"LU" dts:%"LLU" target :%"LLU"\n",audioSize,nbSample,audioDts,targetDts);
                    if(audioDts!=ADM_NO_PTS)
                        if( abs(audioDts-clk->getTimeUs())>5000)
                        {
                            printf("[Avi] Audio skew!");
                            clk->setTimeUs(audioDts);
                        }
                    nb=writter.saveAudioFrame(audioIndex,audioSize,audioBuffer) ;
                    clk->advanceBySample(nbSample);
                    //printf("%u vs %u\n",audioDts/1000,(lastVideoDts+videoIncrement)/1000);
                    if(audioDts!=ADM_NO_PTS)
                    {
                        if(audioDts>targetDts) break;
                    }
                }
                if(!nb) aprintf("[AVI] No audio for video frame %lu\n",targetDts);
            }
            return true;
}
/**
    \fn save
*/
bool muxerAvi::save(void)
{
    printf("[AVI] Saving\n");
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;

    uint32_t len,flags;
    uint64_t pts,dts,rawDts;
    uint64_t lastVideoDts=0;
    uint64_t videoIncrement;
    uint64_t videoDuration=vStream->getVideoDuration();
    int ret;
    int written=0;
    float f=(float)vStream->getAvgFps1000();
    f=1000./f;
    f*=1000000;
    videoIncrement=(uint64_t)f;  // Video increment in AVI-Tick

    audioBuffer=new uint8_t[AUDIO_BUFFER_SIZE];
    videoBuffer=new uint8_t[bufSize];

    printf("[AVI]avg fps=%u\n",vStream->getAvgFps1000());
    DIA_encodingBase  *progress=createEncoding(vStream->getAvgFps1000());
    progress->setContainer("AVI");

    uint64_t aviTime=0;
    if(false==vStream->getPacket(&len, videoBuffer, bufSize,&pts,&dts,&flags)) goto abt;
    if(dts==ADM_NO_PTS) dts=0;
    lastVideoDts=dts;
    while(1)
    {
            if(dts>aviTime+videoIncrement)
            {
                writter.saveVideoFrame( 0, 0,videoBuffer); // Insert dummy video frame
            }else
            {
                if(!writter.saveVideoFrame( len, flags,videoBuffer))  // Put our real video
                {
                        printf("[AVI] Error writting video frame\n");
                        goto abt;
                }
                if(false==vStream->getPacket(&len, videoBuffer, bufSize,&pts,&dts,&flags)) goto abt;
                if(dts==ADM_NO_PTS)
                {
                    dts=lastVideoDts+videoIncrement;
                }
                lastVideoDts=dts;
            }

            fillAudio(aviTime+videoIncrement);    // and matching audio


            uint32_t  percent=(100*aviTime)/videoDuration;
            if(percent>100) percent=100;
            progress->setPercent(percent);

            written++;
            aviTime+=videoIncrement;
    }
abt:
    writter.setEnd();

    delete [] videoBuffer;
    videoBuffer=NULL;
    delete [] audioBuffer;
    audioBuffer=NULL;
    printf("[AVI] Wrote %d frames, nb audio streams %d\n",written,nbAStreams);
    delete progress;
    return true;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerAvi::close(void)
{

    printf("[AVI] Closing\n");
    return true;
}
//EOF



