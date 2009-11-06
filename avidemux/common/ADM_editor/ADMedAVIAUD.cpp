/***************************************************************************
   \fn  ADMedAVIAUD.cpp
   \brief Interface to audio track(s) from editor

    Handle switching from pieces of movie
    Also fix the gap/overlap in audio to offer a strictly continuous audio stream
    
    copyright            : (C) 2008/2009 by mean
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


#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"

#include "ADM_debugID.h"
#define MODULE_NAME MODULE_AUDIO_EDITOR
#include "ADM_debug.h"


#define AUDIOSEG 	_segments[_audioseg]._reference
#define SEG 		_segments[seg]._reference

#define ADM_ALLOWED_DRIFT_US 40000 // Allow 4b0 ms jitter on audio

#if 1
#define vprintf(...) {}
#else
#define vprintf printf
#endif

/**
 *      \fn switchToNextAudioSegment
 *
 */
bool ADM_Composer::switchToNextAudioSegment(void)
{
        // Try to switch segment
        if(_audioSeg+1>=_segments.getNbSegments()) return false;

        ADM_warning("Switching to segment %"LU"\n",_audioSeg+1);
        _audioSeg++;
        _SEGMENT *seg=_segments.getSegment(_audioSeg);
        ADM_audioStreamTrack *trk=getTrack(seg->_reference);
        // Go to beginning of the stream
        if(false==trk->stream->goToTime(seg->_refStartTimeUs))
          {
            ADM_warning("Fail to seek audio to %"LLU"ms\n",seg->_refStartTimeUs/1000);
            return false;
          }
        ADM_info("Switched ok to audio segment %"LU"\n",_audioSeg);
        return true;

}
/**
    \fn refillPacketBuffer
    \brief Fetch a new packet
*/
bool ADM_Composer::refillPacketBuffer(void)
{
   packetBufferSize=0; 
   uint64_t dts;
   _SEGMENT *seg=_segments.getSegment(_audioSeg);
   if(!seg) return false;

   if(lastDts>=seg->_startTimeUs+seg->_durationUs)
     {
       ADM_info("Consumed all data from this audio segment\n");
       switchToNextAudioSegment();
       seg=_segments.getSegment(_audioSeg);
     }



   ADM_audioStreamTrack *trk=getTrack(seg->_reference);
    if(!trk) return false;    

    if(!trk->stream->getPacket(packetBuffer,&packetBufferSize,ADM_EDITOR_PACKET_BUFFER_SIZE,
                        &packetBufferSamples,&dts))
    {
              if(true==switchToNextAudioSegment())
                 return refillPacketBuffer();
             ADM_warning("End of audio\n");
             return false;
    }
    //
    // Ok we have a packet, rescale audio
    if(dts==ADM_NO_PTS) packetBufferDts=ADM_NO_PTS;
    else
    {
      if(dts>=seg->_refStartTimeUs) packetBufferDts=dts+seg->_startTimeUs-seg->_refStartTimeUs;
      else
        {
          ADM_warning("Got PTS=%"LLU" which is too early for start=%"LLU"ms\n",dts/1000,seg->_refStartTimeUs);
          return refillPacketBuffer();
        }
    }
    return true;
}

/**
    \fn     getPCMPacket
    \brief  Get audio packet

*/

bool ADM_Composer::getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
uint32_t fillerSample=0;   // FIXME : Store & fix the DTS error correctly!!!!
uint32_t inSize;
bool drop=false;
static bool fail=false;

again:
    *samples=0;
    ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk) return false;
    // Do we already have a packet ?
    if(!packetBufferSize)
    {
        if(!refillPacketBuffer())
        {
            if(fail==false)
              ADM_warning("[Editor] Cannot refill audio\n");
            fail=true;
            return false;
        }
    }
    // We do now
    vprintf("[PCMPacket]  Got %d samples, time code %08lu  lastDts=%08lu delta =%08ld\n",packetBufferSamples,packetBufferDts,lastDts,packetBufferDts-lastDts);
    fail=false;

    // Check if the Dts matches
    if(lastDts!=ADM_AUDIO_NO_DTS &&packetBufferDts!=ADM_AUDIO_NO_DTS)
    {
        if(abs(lastDts-packetBufferDts)>ADM_ALLOWED_DRIFT_US)
        {
            printf("[Composer::getPCMPacket] drift %d, computed :%"LLU" got %"LLU"\n",(int)(lastDts-packetBufferDts),lastDts,packetBufferDts);
            if(packetBufferDts<lastDts)
            {
                printf("[Composer::getPCMPacket] Dropping packet %"LU" last =%"LU"\n",(uint32_t)(lastDts/1000),(uint32_t)(packetBufferDts/1000));
                drop=true;
            }else 
            {
                // There is a "hole" in audio
                // Let's add some filler
                // Compute filler size
                float f=packetBufferDts-lastDts; // in us
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
                *samples=fillerSample;
                vprintf("[Composer::getPCMPacket] Adding %u padding samples, dts is now %lu\n",fillerSample,lastDts);
                return true;
       }
      }
    }
    // If lastDts is not initialized....
    if(lastDts==ADM_AUDIO_NO_DTS) lastDts=packetBufferDts;
    
    //
    //  The packet is ok, decode it...
    //
    uint32_t nbOut=0; // Nb sample as seen by codec
    if(!trk->codec->run(packetBuffer, packetBufferSize, dest, &nbOut))
    {
            packetBufferSize=0; // consume
            adm_printf(ADM_PRINT_ERROR,"[Composer::getPCMPacket] codec failed failed\n");
            return false;
    }
    packetBufferSize=0; // consume

    // Compute how much decoded sample to compare with what demuxer said
    uint32_t decodedSample=nbOut;
    decodedSample/=trk->wavheader.channels;
    if(!decodedSample) goto again;
#define ADM_MAX_JITTER 5000  // in samples, due to clock accuracy, it can be +er, -er, + er, -er etc etc
    if(abs(decodedSample-packetBufferSamples)>ADM_MAX_JITTER)
    {
        printf("[Composer::getPCMPacket] Demuxer was wrong %d vs %d samples!\n",packetBufferSamples,decodedSample);
    }
    
    // This packet has been dropped (too early packt), try the next one
    if(drop==true)
    {
        // TODO Check if the packet somehow overlaps, i.e. starts too early but finish ok
        goto again;
    }
    // Update infos
    *samples=(decodedSample);
    *odts=lastDts;
    advanceDtsBySample(decodedSample);
    vprintf("[Composer::getPCMPacket] Adding %u decodedSample, dts is not %lu\n",fillerSample,lastDts);
    ADM_assert(sizeMax>=(fillerSample+decodedSample)*trk->wavheader.channels);
    return true;
}
/**
        \fn getPacket
        \brief
*/
uint8_t ADM_Composer::getPacket(uint8_t  *dest, uint32_t *len,uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
        
    _SEGMENT *seg=_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrack(seg->_reference);
    if(!trk) return 0;
   
    // Read a packet
zgain:
    bool r=trk->stream->getPacket(dest,len,sizeMax,samples,odts);
    if(r==false) return false;
    //


    // Rescale odts
    if(*odts!=ADM_NO_PTS)
    {
        if(*odts<seg->_refStartTimeUs)
        {
            ADM_warning("Audio packet is too early %"LLU" ms, this segment starts at %"LLU"ms\n",*odts,seg->_refStartTimeUs);
            goto zgain;
        }
#if 0
        ADM_info("Audio DTS:%"LLU" ms, ref StartTime :%"LLU" Delta:%"LLU" duration :%"LLU"\n",
                    *odts/1000,seg->_refStartTimeUs/1000,(*odts-seg->_refStartTimeUs)/1000,seg->_durationUs/1000);
#endif
        *odts-=seg->_refStartTimeUs;
        if(*odts>seg->_durationUs)
        {
            if(switchToNextAudioSegment()==false)
            {
                
                return false;
            }
            seg=_segments.getSegment(_audioSeg);
            goto zgain;
        }
        *odts+=seg->_startTimeUs;
    }else
    {
        *odts=ADM_NO_PTS;
    }
    //advanceDtsBySample(*samples);
    return true;
}
/**
    \fn goToTime
    \brief Audio Seek in ms

*/
bool ADM_Composer::goToTime (uint64_t ustime)
{
  uint32_t seg;
  uint64_t segTime;
    ADM_info(" go to time %02.2f secs\n",((float)ustime)/1000000.);
    if(false==_segments.convertLinearTimeToSeg(ustime,&seg,&segTime))
      {
        ADM_warning("Cannot convert %"LLU" to linear time\n",ustime/1000);
        return false;
      }
    _SEGMENT *s=_segments.getSegment(seg);
    ADM_audioStreamTrack *trk=getTrack(s->_reference);
    if(!trk)
      {
        ADM_warning("No audio for segment %"LU"\n",seg);
        return false;
      }
    uint64_t seekTime;
    seekTime=segTime+s->_refStartTimeUs;
    if(true==trk->stream->goToTime(seekTime))
    {
        _audioSeg=seg;
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

  _SEGMENT *seg=_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrack(seg->_reference);
    if(!trk) return NULL;
    return trk->stream->getInfo();
}
/**
    \fn getChannelMapping
    \brief returns channel mapping
*/
 CHANNEL_TYPE    *ADM_Composer::getChannelMapping(void )
{
  _SEGMENT *seg=_segments.getSegment(_audioSeg);
  ADM_audioStreamTrack *trk=getTrack(seg->_reference);
    if(!trk) return NULL;
    return trk->codec->channelMapping;
}
/**
    \fn getExtraData
*/
bool            ADM_Composer::getExtraData(uint32_t *l, uint8_t **d)
{
  _SEGMENT *seg=_segments.getSegment(_audioSeg);
  ADM_audioStreamTrack *trk=getTrack(seg->_reference);

    *l=0;
    *d=NULL;
    if(!trk) return false;
    return trk->stream->getExtraData(l,d); 

}
/**
    \fn getTrack
    \brief Returns Track for ref video given as parameter
    @param : Reference video
*/
ADM_audioStreamTrack *ADM_Composer::getTrack(uint32_t refVideo)
{
    _VIDEOS *v=_segments.getRefVideo(refVideo);
    if(!v->audioTracks) return NULL;

    return v->audioTracks[v->currentAudioStream];
}

//EOF

