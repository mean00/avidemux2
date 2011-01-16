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
#if 0
#define aprintf(...) {}
#define MP4_DEBUG 0
#else
#define aprintf ADM_info
#define MP4_DEBUG MP4_DETAILS_ALL
#endif


/**
    \fn initAudio
*/
bool muxerMp4v2::initAudio(void)
{
    
    for(int i=0;i<nbAStreams;i++)
    {
        WAVHeader *header=aStreams[i]->getInfo();
        ADM_audioStream*a=aStreams[i];
        switch(header->encoding)
        {
            case WAV_MP2:
            case WAV_MP3:
                    #warning fixme : Not always 1152 sample per packet
                    
                    audioTrackIds[i]=MP4AddAudioTrack(handle,
                                                      header->frequency,
                                                      1152,
                                                      MP4_MPEG2_AUDIO_TYPE);
                    if(MP4_INVALID_TRACK_ID==audioTrackIds[i])
                    {
                        ADM_error("Error adding audio track %i of type 0x%x\n",i,header->encoding);
                        return false;
                    }
                    aprintf("Add Track %d fq %d\n",audioTrackIds[i],header->frequency);
                    MP4SetAudioProfileLevel(handle,0x0f);
                    MP4SetTrackIntegerProperty(handle,audioTrackIds[i],"mdia.minf.stbl.stsd.mp4a.channels",
                                header->channels);
                    break;
            default:
                    ADM_error("Cannot create audio track of type 0x%x\n",header->encoding);
                    return false;
        }
        // Preload this track...
        loadAndToggleAudioSlot(i); 
    }
    if(nbAStreams)
         MP4SetTrackIntegerProperty(handle, audioTrackIds[0], "tkhd.flags", 3);
    return true;
}
/**
    \fn loadAndToggleAudioSlot
*/
bool muxerMp4v2::loadAndToggleAudioSlot(int index)
{
        ADM_audioStream                     *a=aStreams[index];
        mp4v2AudioPacket                    *pkt=&(audioPackets[index]);
        mp4v2AudioPacket::mp4v2AudioBlock   *blk=&(pkt->blocks[pkt->nextWrite]);
        if(!a->getPacket(blk->buffer,
                         &(blk->sizeInBytes),
                         AUDIO_BUFFER_SIZE,
                         &(blk->nbSamples),
                         &(blk->dts)))
        {
                ADM_warning("Cannot get audio packet for stream %d\n",index);
                pkt->eos=true;
                return false;
        }
        blk->present=true;
        pkt->nextWrite=!pkt->nextWrite;
        return true;
}
/**
    \fn writeAudioBlock
*/
bool muxerMp4v2::writeAudioBlock(int index,mp4v2AudioPacket::mp4v2AudioBlock *block,uint64_t nbSamples)
{
    ADM_info("Writting audio block : size=%d, samples=%d nbSamples=%d \n",block->sizeInBytes,block->nbSamples,(int)nbSamples);
    bool r=MP4WriteSample(handle,audioTrackIds[index],
                            block->buffer,
                            block->sizeInBytes,
                            nbSamples,
                            0,1);
    if(false==r)
                        {
                            ADM_error("Cannot write audio sample for track %d\n",index);
                            //return false;
                        }
    return true;
}
/**
    \fn fillAudio
    \brief push audio packets until nextDts is reached
*/  
bool muxerMp4v2::fillAudio(uint64_t targetDts)
{
    for(int audioIndex=0;audioIndex<nbAStreams;audioIndex++)
    {
                ADM_audioStream         *a=aStreams[audioIndex];
                uint32_t                fq=a->getInfo()->frequency;
                mp4v2AudioPacket       *pkt=&(audioPackets[audioIndex]);
                if(pkt->eos)            continue;
                
                while(1)
                {
                        int current=!pkt->nextWrite;
                        int other=pkt->nextWrite;
                        mp4v2AudioPacket::mp4v2AudioBlock        *currentBlock=&(pkt->blocks[current]);
                        mp4v2AudioPacket::mp4v2AudioBlock        *otherBlock=&(pkt->blocks[other]);
                        if(currentBlock->dts>targetDts) // In the future
                            break;
                        if(false==writeAudioBlock(audioIndex,currentBlock,currentBlock->nbSamples))
                        {
                            ADM_error("Cannot write audio sample for track %d\n",audioIndex);
                            pkt->eos=true;
                            return false;
                        }
                        // load next
                        if(false==loadAndToggleAudioSlot(audioIndex))
                        {
                            #warning Purge other slot
                            pkt->eos=true;
                        }
                }
    }
    return true;
}
//EOF



