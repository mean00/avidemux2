/***************************************************************************
                          ADMedAVIAUD.cpp  -  description
                             -------------------

    Handle switching from pieces of movie
    Also fix the gap/overlap in audio to offer a strictly continuous audio stream
    
    copyright            : (C) 2008 by mean
    email                : fixounet@free.fr

Todo:
-----
        x Fix handling of overlay/gap, it is wrong.


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include "ADM_default.h"
#include <math.h>

#include "config.h"
#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_editor/ADM_edAudio.hxx"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_AUDIO_EDITOR
#include "ADM_osSupport/ADM_debug.h"


#define AUDIOSEG 	_segments[_audioseg]._reference
#define SEG 		_segments[seg]._reference

#define ADM_ALLOWED_DRIFT_US 30000 // Allow 30 ms jitter on audio

#if 1
#define vprintf(...) {}
#else
#define vprintf printf
#endif

// ADM_audioStreamTack *
#define MYAUDIO (_videos[0].audioTracks[_videos[0].currentAudioStream])

/**
    \fn     getPCMPacket
    \brief  Get audio packet

*/

uint8_t ADM_Composer::getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
uint32_t nbSamples,fillerSample=0;   // FIXME : Store & fix the DTS error correctly!!!!
uint64_t dts;
uint32_t inSize,nbOut;
bool drop=false;

    *samples=0;
    ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk) return 0;    

    // Read a packet from stream 0
again:
    drop=false;
    fillerSample=0;
    if(!trk->stream->getPacket(audioBuffer,&inSize,ADM_EDITOR_AUDIO_BUFFER_SIZE,&nbSamples,&dts))
    {
            adm_printf(ADM_PRINT_ERROR,"[Composer::getPCMPacket] Read failed\n");
            return 0;
    }
    vprintf("[PCMPacket]  Got %d samples, time code %08lu  lastDts=%08lu delta =%08ld\n",nbSamples,dts,lastDts,dts-lastDts);
    // Check if the Dts matches
    if(lastDts!=ADM_AUDIO_NO_DTS && dts!=ADM_AUDIO_NO_DTS)
    {
        if(abs(lastDts-dts)>ADM_ALLOWED_DRIFT_US)
        {
        printf("[Composer::getPCMPacket] drift %d, computed :%lu got %lu\n",(int)(lastDts-dts),lastDts,dts);
        if(dts<lastDts)
        {
            printf("[Composer::getPCMPacket] Dropping packet\n");
            drop=true;
        }else 
        {
            // Else add filler 
            // Compute filler size
            float f=dts-lastDts; // in us
            f*=wavHeader.frequency;
            f/=1000000.;
            // in samples!
            uint32_t fillerSample=(uint32_t )(f+0.49);
            uint32_t mx=sizeMax/trk->wavheader.channels;
            
            if(mx<fillerSample) fillerSample=mx;
            // arbitrary cap, max 4kSample in one go
            // about 100 ms
            if(fillerSample>4*1024) 
            {
                fillerSample=4*1024;
            }
            uint32_t start=fillerSample*sizeof(float)*trk->wavheader.channels;
            memset(dest,0,start);

            advanceDtsBySample(fillerSample);
            dest+=fillerSample*trk->wavheader.channels;
            vprintf("[Composer::getPCMPacket] Adding %u padding samples, dts is now %lu\n",fillerSample,lastDts);
       }
      }
    }else
    {
        // If lastDts is not initialized....
        if(lastDts==ADM_AUDIO_NO_DTS) lastDts=dts;
    }
    // Call codec...
    if(!trk->codec->run(audioBuffer, inSize, dest, &nbOut))
    {
            adm_printf(ADM_PRINT_ERROR,"[Composer::getPCMPacket] codec failed failed\n");
            return 0;
    }
    
    //
    //
    uint32_t decodedSample=nbOut;
    decodedSample/=trk->wavheader.channels;
    if(!decodedSample && !fillerSample) goto again;
#define ADM_MAX_JITTER 5000  // in samples
    if(abs(nbSamples-decodedSample)>ADM_MAX_JITTER)
    {
        printf("[Composer::getPCMPacket] Demuxer was wrong %d vs %d samples!\n",nbSamples,decodedSample);
    }
    
    // This packet has been dropped, try the next one
    if(drop==true && !fillerSample) goto again;
    // Update infos
    *samples=(decodedSample+fillerSample);
    *odts=lastDts;
    advanceDtsBySample(decodedSample);
    vprintf("[Composer::getPCMPacket] Adding %u decodedSample, dts is not %lu\n",fillerSample,lastDts);
    ADM_assert(sizeMax>=(fillerSample+decodedSample)*trk->wavheader.channels);
    return 1;
}
/**
        \fn getPacket
        \brief
*/
uint8_t ADM_Composer::getPacket(uint8_t  *dest, uint32_t *len,uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
     ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk) return 0;
    // Read a packet from stream 0
     return trk->stream->getPacket(dest,len,sizeMax,samples,odts);
    
}
/**
    \fn goToTime
    \brief Audio Seek in ms

*/
bool ADM_Composer::goToTime (uint64_t ustime)
{
    printf("[Editor] go to time %02.2f secs\n",((float)ustime)/1000000.);
    ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk) return 0;
    
    
    if(true==trk->stream->goToTime(ustime))
    {
        setDts(ustime);
        return true;
    }
    return false;
}

/**
    \fn getAudioStream

*/
uint8_t ADM_Composer::getAudioStream (ADM_audioStream ** audio)
{
   ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk)
    {
      *audio = NULL;
      return 0;

    }
  *audio = this;
  return 1;
};

/**
    \fn getInfo
    \brief returns synthetic audio info
*/
WAVHeader       *ADM_Composer::getInfo(void)
{
     ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk) return 0;
    return trk->stream->getInfo();
}
/**
    \fn getChannelMapping
    \brief returns channel mapping
*/
 CHANNEL_TYPE    *ADM_Composer::getChannelMapping(void )
{
  ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk) return NULL;
    return trk->codec->channelMapping;

}
/**
    \fn getExtraData
*/
bool            ADM_Composer::getExtraData(uint32_t *l, uint8_t **d)
{
    *l=0;
    *d=NULL;
     ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk) return false;
    return trk->stream->getExtraData(l,d); 

}

//EOF

