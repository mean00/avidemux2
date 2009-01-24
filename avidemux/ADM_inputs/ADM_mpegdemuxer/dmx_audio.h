/***************************************************************************
                          dmx_audio.h  
                        Demuxer for audio stream

    copyright            : (C) 2005 by mean
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
 #ifndef MPX_AUDIO
 #define MPX_AUDIO
#include "dmx_demuxer.h"
#include "dmx_demuxerEs.h"
#include "dmx_demuxerPS.h"
#include "dmx_demuxerTS.h"
#include "dmx_demuxerMSDVR.h"
#include "ADM_audio/aviaudio.hxx"

#define DMX_MAX_TRACK 16

 typedef struct 
 {
                uint32_t img;                      // Corresponding image
                uint64_t start;                    // Start of packet
                uint64_t count[DMX_MAX_TRACK];     // Size of audio seen
}dmxAudioIndex;

class dmxAudioTrack
{
public:
                      dmxAudioTrack(void) {};
                      ~dmxAudioTrack() {};
      uint32_t        myPes,myPid;
      WAVHeader       wavHeader;
      int32_t         avSync;
};
/**
    \class ADM_audioAccessMpeg
    \brief Access layer for mpeg audio (TS/PS/...)

*/
class ADM_audioAccessMpeg
{
protected:
                uint8_t                 probeAudio (void);
                dmx_demuxer            *demuxer;
                dmxAudioIndex           *_index;
                uint32_t                nbIndex;
                uint32_t                nbTrack;
                dmxAudioTrack           *_tracks;
                uint32_t                currentTrack;
                uint8_t                 getAudioStreamsInfo(uint32_t *nbStreams, audioInfo **infos);
                uint8_t                 changeAudioTrack(uint32_t newtrack);

public:
                                  ADM_audioAccessMpeg(const char *name);
                virtual           ~ADM_audioAccessMpeg();
                                    /// Hint, the stream is pure CBR (AC3,MP2,MP3)
                virtual bool      isCBR(void) { return true;}
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return true;};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return false;};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return true;};
                                    /// Returns audio duration in us
                virtual uint64_t  getDurationInUs(void);
                                    /// Go to a given time
                virtual bool      goToTime(uint64_t timeUs);
                
                virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);
};


#endif
