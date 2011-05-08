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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include <math.h>


#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"

#include "ADM_debugID.h"
#define MODULE_NAME MODULE_AUDIO_EDITOR
#include "ADM_debug.h"

#include "ADM_vidMisc.h"

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
        ADM_info("Switched ok to audio segment %"LU", with a ref time=%s\n",
            _audioSeg,ADM_us2plain(seg->_refStartTimeUs));
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
   static bool endOfAudio=false;
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
              {
                 endOfAudio=false;
                 return refillPacketBuffer();
              }
             if(endOfAudio==false)
                ADM_warning("End of audio\n");
             endOfAudio=true;
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
    endOfAudio=false;
    return true;
}

/**
        \fn getPacket
        \brief
*/
uint8_t ADM_Composer::getPacket(uint8_t  *dest, uint32_t *len,uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
zgain:        
    _SEGMENT *seg=_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrack(seg->_reference);
    if(!trk) return 0;
   
    // Read a packet

    bool r=trk->stream->getPacket(dest,len,sizeMax,samples,odts);
    if(r==false) 
    {
            ADM_warning("AudioGetPacket failed, audioSegment=%d\n",(int)_audioSeg);
            // if it fails, we have to switch segment
            if(false==switchToNextAudioSegment())
            {
                ADM_warning("..and this is the last segment\n");
                return false;
            }
            goto zgain;
    }

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
                ADM_warning("Audio:Switching to next segment failed\n");
                return false;
            }
            goto zgain;
        }
        *odts+=seg->_startTimeUs;
    }else
    {
        *odts=ADM_NO_PTS;
    }
    //ADM_info("Time : %s\n",ADM_us2plain(*odts));
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
    ADM_info("=> seg %d, rel time %02.2f secs\n",(int)seg,((float)segTime)/1000000.);
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

