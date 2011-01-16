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
#include "fourcc.h"
#include "muxerMp4v2.h"
#include "ADM_codecType.h"
#include "ADM_imageFlags.h"
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif


/**
    \fn initAudio
*/
bool muxerMp4v2::initAudio(void)
{
    for(int i=0;i<nbAStreams;i++)
    {
        WAVHeader *header=aStreams[i]->getInfo();
        switch(header->encoding)
        {
            case WAV_MP2:
            case WAV_MP3:
                    #warning fixme : Not always 1152 sample per packet
                    audioTrackIds[i]=MP4AddAudioTrack(handle,header->frequency,1152,MP4_MPEG2_AUDIO_TYPE);
                    if(MP4_INVALID_TRACK_ID==audioTrackIds[i])
                    {
                        ADM_error("Error adding audio track %i of type 0x%x\n",i,header->encoding);
                        return false;
                    }
                    MP4SetAudioProfileLevel(handle,0x0f);
                    MP4SetTrackIntegerProperty(handle,audioTrackIds[i],"mdia.minf.stbl.stsd.mp4a.channels",
                                header->channels);
                    break;
            default:
                    ADM_error("Cannot create audio track of type 0x%x\n",header->encoding);
                    return false;
        }
    }
    if(nbAStreams)
         MP4SetTrackIntegerProperty(handle, audioTrackIds[0], "tkhd.flags", 3);
    return true;
}
/**
    \fn fillAudio
    \brief push audio packets until nextDts is reached
*/  
bool muxerMp4v2::fillAudio(uint64_t targetDts)
{
    uint8_t buffer[4*1024];
    uint32_t packetLen;
    uint32_t nbSample;
    uint64_t dts;
    for(int audioIndex=0;audioIndex<nbAStreams;audioIndex++)
    {
                ADM_audioStream*a=aStreams[audioIndex];
                uint32_t fq=a->getInfo()->frequency;
                int nb=0;
                while(1)
                {
                        if(!a->getPacket(buffer,
                                         &(packetLen),
                                         4096,
                                         &(nbSample),
                                         &(dts)))
                        {
                                ADM_warning("Cannot get audio packet for stream %d\n",audioIndex);
                                break;
                        }
                        float duration=nbSample;
                        duration/=fq;
                        duration*=90000;
                        if(!MP4WriteSample(handle,audioTrackIds[audioIndex],buffer,packetLen,
                                                            timeScale((uint64_t )duration),
                                                            0,1))
                        {
                            ADM_error("Cannot write audio sample for track %d\n",audioIndex);
                            return false;
                        }
                    // We now have a packet stored
                    if(dts!=ADM_NO_PTS)
                        if(dts>targetDts) break; // this one is in the future
                }
    }
    return true;
}
//EOF



