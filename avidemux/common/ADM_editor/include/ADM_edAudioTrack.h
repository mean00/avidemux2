
/***************************************************************************
    \file  ADM_edAudioTrack
    \brief Manage audio track(s)

    (c) 2012 Mean, fixounet@free.Fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_EDAUDIOTRACK
#define ADM_EDAUDIOTRACK

#include "ADM_default.h"
#include "ADM_audioStream.h"

#define ADM_EDITOR_PACKET_BUFFER_SIZE (60*1024)
#define ADM_ALLOWED_DRIFT_US 40000 // Allow 4b0 ms jitter on audio
#define ADM_MAXIMUM_AMOUT_AUDIO_STREAMS 4 // Allow up to 4 audio streams
typedef enum
{
    ADM_EDAUDIO_FROM_VIDEO,
    ADM_EDAUDIO_EXTERNAL,
    ADM_EDAUDIO_LAST
}ADM_EDAUDIO_TRACK_TYPE;
class ADM_Composer;
class ADM_edAudioTrackFromVideo;
class ADM_edAudioTrackExternal;
/**
    \class ADM_edAudioTrack
*/
class ADM_edAudioTrack : public ADM_audioStream
{
protected:
                    uint8_t  packetBuffer[ADM_EDITOR_PACKET_BUFFER_SIZE];
                    uint32_t packetBufferSize;
                    uint64_t packetBufferDts;
                    uint32_t packetBufferSamples;

                    int64_t   _audioSample;     // current sample number

                    
                    ADM_EDAUDIO_TRACK_TYPE  trackType;


public:
                    ADM_edAudioTrack(ADM_EDAUDIO_TRACK_TYPE type, WAVHeader *hdr)
                            : ADM_audioStream(hdr,NULL)
                    {
                            trackType=type;
                            _audioSample=0; 
                            packetBufferSize=0;
                            packetBufferDts=ADM_NO_PTS;
                    }
            ADM_EDAUDIO_TRACK_TYPE getTrackType() {return trackType;}
            virtual ~ADM_edAudioTrack()
                    {

                    }
            virtual bool            destroyable()=0;
            virtual uint32_t      getOutputFrequency(void)=0; // sbr
            virtual CHANNEL_TYPE * getChannelMapping(void )=0;
            virtual bool            getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts)=0;
            virtual ADM_edAudioTrackFromVideo *castToTrackFromVideo(void) 
                            {
                                    return NULL;
                            }
            virtual ADM_edAudioTrackExternal *castToExternal(void)
                            {
                                    return NULL;
                            }


};
#endif
//EOF
