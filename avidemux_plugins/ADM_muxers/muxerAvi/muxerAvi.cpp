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


#define ADM_NO_PTS 0xFFFFFFFFFFFFFFFFLL // FIXME
#define AUDIO_BUFFER_SIZE 48000*6*sizeof(float)

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

avi_muxer muxerConfig=
{
    HIDDEN
};


/**
    \fn     muxerAVI
    \brief  Constructor
*/
muxerAvi::muxerAvi()
{
    audioBuffer=NULL;
    videoBuffer=NULL;
    clocks=NULL;
};
/**
    \fn     muxerAVI
    \brief  Destructor
*/

muxerAvi::~muxerAvi()
{
    printf("[AviMuxer] Destructing\n");
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
        audioDelay=s->getVideoDelay();
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
                    if(audioDts!=ADM_NO_DTS) audioDts+=audioDelay;
                    aprintf("[Audio] Packet size %"LU" sample:%"LU" dts:%"LLU" target :%"LLU"\n",audioSize,nbSample,audioDts,targetDts);
                    if(audioDts!=ADM_NO_PTS)
                        if( abs(audioDts-clk->getTimeUs())>32000)
                        {
                            ADM_warning("[AviMuxer] Audio skew!\n");
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
                if(!nb) aprintf("[AviMuxer] No audio for video frame %lu\n",targetDts);
            }
            return true;
}
/**
    \fn save
*/
bool muxerAvi::save(void)
{
    printf("[AviMuxer] Saving\n");
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;
    bool result=true;
    uint32_t len,flags;
    uint64_t pts,dts,rawDts;
    uint64_t lastVideoDts=0;
    int ret;
    int written=0;
   

    audioBuffer=new uint8_t[AUDIO_BUFFER_SIZE];
    videoBuffer=new uint8_t[bufSize];

    ADM_info("[AviMuxer]avg fps=%u\n",vStream->getAvgFps1000());

    uint64_t aviTime=0;
    if(false==vStream->getPacket(&len, videoBuffer, bufSize,&pts,&dts,&flags)) goto abt;
    if(dts==ADM_NO_PTS) dts=0;
    lastVideoDts=dts;

    initUI("Saving Avi");
    

    while(1)
    {
            if(dts>aviTime+videoIncrement)
            {
                writter.saveVideoFrame( 0, 0,videoBuffer); // Insert dummy video frame
            }else
            {
                if(!writter.saveVideoFrame( len, flags,videoBuffer))  // Put our real video
                {
                        ADM_warning("[AviMuxer] Error writting video frame\n");
                        result=false;
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

            if(updateUI(aviTime)==false)
            {  
                result=false;
                goto abt;
            }
           
            written++;
            aviTime+=videoIncrement;
    }
abt:
    closeUI();
    writter.setEnd();
    delete [] videoBuffer;
    videoBuffer=NULL;
    delete [] audioBuffer;
    audioBuffer=NULL;
    ADM_info("[AviMuxer] Wrote %d frames, nb audio streams %d\n",written,nbAStreams);
    return result;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerAvi::close(void)
{

    ADM_info("[AviMuxer] Closing\n");
    return true;
}
//EOF



