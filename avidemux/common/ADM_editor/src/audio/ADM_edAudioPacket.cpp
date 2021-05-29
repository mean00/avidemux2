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
#include "ADM_edit.hxx"
#include "ADM_edAudioTrackFromVideo.h"


#include "ADM_vidMisc.h"

//#define AUDIOSEG 	_segments[_audioseg]._reference
//#define SEG 		_segments[seg]._reference

static bool warned = false;

#if 1
#define vprintf(...) {}
#else
#define vprintf printf
#endif

/**
 *      \fn switchToNextAudioSegment
 *
 */
bool ADM_edAudioTrackFromVideo::switchToNextAudioSegment(void)
{
    // Try to switch segment
    if(_audioSeg+1>=parent->_segments.getNbSegments()) return false;

    ADM_warning("Switching to segment %" PRIu32"\n",_audioSeg+1);
    _audioSeg++;
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(seg->_reference);
    //
    if(!trk)
    {
        ADM_warning("No next segment audio\n");
        return false;
    }
    // Go to beginning of the stream
    if(false==trk->stream->goToTime(seg->_refStartTimeUs))
    {
        ADM_warning("Cannot seek in audio to %s\n",ADM_us2plain(seg->_refStartTimeUs));
        return false;
    }
    ADM_Audiocodec *codec=NULL;
    if(trk)
        if(trk->codec)
            codec=trk->codec;
    if(codec)
    {
#if 1
        /* If the codec requires extradata and codec parameters have changed,
        we would need to respawn the decoder. However, doing so while unable to
        handle on-the-fly changes means we cannot resume normal playback after
        switch to the original channel layout present at the start of playback
        prior to segment switch. FIXME */
        codec->resetAfterSeek();
#else
        /* If the codec requires extradata, we need to respawn the decoder
        to deal with a chance that codec parameters have changed. */
        uint32_t eLen=0;
        uint8_t *eData=NULL;
        trk->stream->getExtraData(&eLen,&eData);
        if(eLen && (trk->extraCopyLen!=eLen || (trk->extraCopy && memcmp(trk->extraCopy,eData,eLen))))
        {
            ADM_warning("Extradata does not match cached copy, respawning the decoder.\n");
            delete codec;
            delete [] trk->extraCopy;
            trk->extraCopy=new uint8_t[eLen];
            memcpy(trk->extraCopy,eData,eLen);
            trk->extraCopyLen=eLen;
            WAVHeader *hdr=&(trk->wavheader);
            trk->codec=getAudioCodec(hdr->encoding,hdr,eLen,eData);
        }
        if(trk->codec)
            trk->codec->resetAfterSeek();
#endif
    }
    ADM_info("Switched ok to audio segment %" PRIu32", with a ref time=%s\n",
            _audioSeg,ADM_us2plain(seg->_refStartTimeUs));
    return true;

}
/**
    \fn refillPacketBuffer
    \brief Fetch a new packet
*/
bool ADM_edAudioTrackFromVideo::refillPacketBuffer(void)
{
    if(_audioSeg >= parent->_segments.getNbSegments())
        return false;

    packetBufferSize=0;
    uint64_t dts;
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    static bool endOfAudio=false;
    if(!seg) return false;

    if(lastDts>=seg->_startTimeUs+seg->_durationUs)
    {
        if(!warned)
        {
            ADM_info("Consumed all data from this audio segment\n");
            warned = true;
        }
        if(switchToNextAudioSegment())
            warned = false;
        seg=parent->_segments.getSegment(_audioSeg);
        if(!seg) return false;
    }



    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(seg->_reference);
    if(!trk) return false;

    if(!getPacket(packetBuffer,&packetBufferSize,ADM_EDITOR_PACKET_BUFFER_SIZE,
                        &packetBufferSamples,&dts))
    {
        if(endOfAudio==false)
            ADM_warning("End of audio\n");
        endOfAudio=true;
        return false;
    }
    // fixup dts
    packetBufferDts=dts;
    endOfAudio=false;
    warned = false;
    return true;
}

/**
        \fn getPacket
        \brief
*/
uint8_t ADM_edAudioTrackFromVideo::getPacket(uint8_t  *dest, uint32_t *len,uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
    if(_audioSeg >= parent->_segments.getNbSegments())
        return 0;
zgain:
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(seg->_reference);
    if(!trk) return 0;

    WAVHeader *hdr = &trk->wavheader;
    if((hdr->encoding == WAV_PCM || hdr->encoding == WAV_LPCM) && hdr->blockalign > 1)
        sizeMax -= sizeMax % hdr->blockalign;

    // Read a packet

    bool r=trk->stream->getPacket(dest,len,sizeMax,samples,odts);
    if(r==false) 
    {
        if(msgRatelimit->done())
        {
            if(msgSuppressed)
            {
                ADM_warning("Audio getPacket failed, audioSegment=%d (message repeated %d times)\n",(int)_audioSeg,msgSuppressed+1);
                msgSuppressed = 0;
            }else
            {
                ADM_warning("Audio getPacket failed, audioSegment=%d\n",(int)_audioSeg);
            }
        }else
        {
            msgSuppressed++;
        }
        // if it fails, we have to switch segment
        if(false==switchToNextAudioSegment())
        {
            if(msgRatelimit->done())
            {
                ADM_warning("..and this is the last segment\n");
                msgRatelimit->reset();
            }
            return false;
        }
        goto zgain;
    }else
    {
        msgSuppressed = 0;
    }

    if((hdr->encoding == WAV_PCM || hdr->encoding == WAV_LPCM) && hdr->blockalign > 1 && *len % hdr->blockalign)
    {
        ADM_warning("Audio packet not aligned, truncating.\n");
        *len -= *len % hdr->blockalign;
    }

    // Rescale odts
    if(*odts!=ADM_NO_PTS)
    {
        if(*odts<seg->_refStartTimeUs)
        {
            ADM_warning("Audio packet is too early %" PRIu64" ms, this segment starts at %" PRIu64"ms\n",*odts,seg->_refStartTimeUs);
            goto zgain;
        }
#if 0
        ADM_info("Audio DTS:%"PRIu64" ms, ref StartTime :%"PRIu64" Delta:%"PRIu64" duration :%"PRIu64"\n",
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
    msgSuppressed = 0;
    return true;
}
/**
    \fn goToTime
    \brief Audio Seek in ms

*/
bool ADM_edAudioTrackFromVideo::goToTime (uint64_t ustime)
{
    uint32_t seg;
    uint64_t segTime;
    ADM_info("[edAudioTrackFromVideo] Seeking to %s\n",ADM_us2plain(ustime));
    if(false==parent->_segments.convertLinearTimeToSeg(ustime,&seg,&segTime))
    {
        ADM_warning("Cannot convert %" PRIu64" to linear time\n",ustime);
        return false;
    }
    ADM_info("[edAudioTrackFromVideo] => seg %d, rel time %s\n",(int)seg,ADM_us2plain(segTime));
    _SEGMENT *s=parent->_segments.getSegment(seg);
    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(s->_reference);
    if(!trk)
      {
        ADM_warning("No audio for segment %" PRIu32"\n",seg);
        return false;
      }
    uint64_t seekTime;
    seekTime=segTime+s->_refStartTimeUs;
    if(trk->stream->goToTime(seekTime))
    {
        _audioSeg=seg;
        setDts(ustime);
        packetBufferSize=0; // Flush PCM decoder
        /* If the codec requires extradata, we need to respawn the decoder
        if codec parameters have changed. */
        uint32_t eLen=0;
        uint8_t *eData=NULL;
        trk->stream->getExtraData(&eLen,&eData);
        if(eLen && (trk->extraCopyLen!=eLen || (trk->extraCopy && memcmp(trk->extraCopy,eData,eLen))))
        {
            ADM_warning("Extradata does not match cached copy, respawning the decoder.\n");
            delete trk->codec;
            delete [] trk->extraCopy;
            trk->extraCopy=new uint8_t[eLen];
            memcpy(trk->extraCopy,eData,eLen);
            trk->extraCopyLen=eLen;
            WAVHeader *hdr=&(trk->wavheader);
            trk->codec=getAudioCodec(hdr->encoding,hdr,eLen,eData);
        }
        return true;
    }
    ADM_warning("Go to time failed\n");
    return false;
}

/**
    \fn getAudioStream

*/
uint8_t ADM_edAudioTrackFromVideo::getAudioStream (ADM_audioStream ** audio)
{
   ADM_audioStreamTrack *trk=getTrackAtVideoNumber(0);
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
WAVHeader       *ADM_edAudioTrackFromVideo::getInfo(void)
{
    if(_audioSeg >= parent->_segments.getNbSegments())
    {
        ADM_error("Audio segment %u out of range!\n",_audioSeg);
        return NULL;
    }
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(seg->_reference);
    if(!trk) // this segment has no audio, try to get info from another one
    {
        trk=getTrackAtVideoNumber(0); // might be wrong, does not matter
        if(!trk)
            return NULL;
    }
    return &(trk->wavheader);
}
/**
    \fn getChannelMapping
    \brief returns channel mapping
*/
 CHANNEL_TYPE    *ADM_edAudioTrackFromVideo::getChannelMapping(void )
{
    if(_audioSeg >= parent->_segments.getNbSegments())
    {
        ADM_error("Audio segment %u out of range!\n",_audioSeg);
        return NULL;
    }
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(seg->_reference);
    if(!trk) return NULL;
    return trk->codec->channelMapping;
}
/**
    \fn getExtraData
*/
bool            ADM_edAudioTrackFromVideo::getExtraData(uint32_t *l, uint8_t **d)
{
    if(_audioSeg >= parent->_segments.getNbSegments())
    {
        ADM_error("Audio segment %u out of range!\n",_audioSeg);
        return false;
    }
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(seg->_reference);

    *l=0;
    *d=NULL;
    if(!trk) return false;
    return trk->stream->getExtraData(l,d); 

}
/**
    \fn getTrackAtVideoNumber
    \brief Returns Track for ref video given as parameter
    @param : Reference video
*/
ADM_audioStreamTrack *ADM_edAudioTrackFromVideo::getTrackAtVideoNumber(uint32_t refVideo)
{
    _VIDEOS *v=parent->_segments.getRefVideo(refVideo);
    if(!v->audioTracks.size()) return NULL;

    return v->audioTracks[myTrackNumber];
}
/**
    \fn getCurrentTrack
    \brief Returns Track for ref video given as parameter
    @param : Reference video
*/
ADM_audioStreamTrack *ADM_edAudioTrackFromVideo::getCurrentTrack()
{
    if(_audioSeg >= parent->_segments.getNbSegments())
    {
        ADM_error("Audio segment %u out of range!\n",_audioSeg);
        return NULL;
    }
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    if(!seg) return NULL;
    _VIDEOS *v=parent->_segments.getRefVideo(seg->_reference);
    if(!v->audioTracks.size()) return NULL;

    return v->audioTracks[myTrackNumber];
}
/**
    \fn updateHeader
    \brief Update header and tell decoder that we can handle a change in # of channels.
           We have consumed one packet, the caller should repeat seek.
*/
bool ADM_edAudioTrackFromVideo::updateHeader(void)
{
    if(_audioSeg >= parent->_segments.getNbSegments())
    {
        ADM_error("Audio segment %u out of range!\n",_audioSeg);
        return false;
    }
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(seg->_reference);
    if(!trk) return false;
    if(!trk->codec) return false;
    bool r=parent->checkSamplingFrequency(trk);
    trk->codec->resetAfterSeek();
    trk->codec->reconfigureCompleted();
    return r;
}
//EOF

