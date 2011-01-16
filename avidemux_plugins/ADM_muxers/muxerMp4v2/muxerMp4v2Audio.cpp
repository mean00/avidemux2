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
#define MP4_DEBUG 0
#else
#define aprintf ADM_info
#define MP4_DEBUG MP4_DETAILS_ALL
#endif

#warning add audioDelay
#warning fix audio not starting at 0

/**
    \fn initAudio
*/
bool muxerMp4v2::initAudio(void)
{
    audioTrackIds=new MP4TrackId[nbAStreams];
    audioPackets=new mp4v2AudioPacket[nbAStreams];
    
    for(int i=0;i<nbAStreams;i++)
    {
        WAVHeader *header=aStreams[i]->getInfo();
        ADM_audioStream*a=aStreams[i];
        audioPackets[i].clock=new audioClock(header->frequency);
        // Preload this track...
        if(false==loadAndToggleAudioSlot(i))
        {
            audioPackets[i].eos=true;
            continue;
        }
        
        switch(header->encoding)
        {
            case WAV_MP2:
            case WAV_MP3:
                    audioTrackIds[i]=MP4AddAudioTrack(handle,
                                                      header->frequency,
                                                      audioPackets[i].blocks[0].nbSamples,
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
        if(blk->dts!=ADM_NO_PTS)
            blk->dts+=audioDelay;
        blk->present=true;
        pkt->nextWrite=!pkt->nextWrite;
        return true;
}
/**
    \fn writeAudioBlock
*/
bool muxerMp4v2::writeAudioBlock(int index,mp4v2AudioPacket::mp4v2AudioBlock *block,uint64_t nbSamples)
{
    aprintf("Writting audio block : size=%d, samples=%d nbSamples=%d \n",block->sizeInBytes,block->nbSamples,(int)nbSamples);
    bool r=MP4WriteSample(handle,audioTrackIds[index],
                            block->buffer,
                            block->sizeInBytes,
                            nbSamples,
                            0,1);
    if(false==r)
                        {
                            ADM_error("Cannot write audio sample for track %d\n",index);
                            return false;
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
                audioClock             *clock=pkt->clock;
                if(pkt->eos)            continue;
                uint64_t                extraSamples=0;
                while(1)
                {
                        int current=!pkt->nextWrite;
                        int other=pkt->nextWrite;
                        mp4v2AudioPacket::mp4v2AudioBlock        *currentBlock=&(pkt->blocks[current]);
                        mp4v2AudioPacket::mp4v2AudioBlock        *otherBlock=&(pkt->blocks[other]);
                        // Get our currentDts
                        uint64_t currentDts=clock->getTimeUs();                        
                        uint64_t blockDts=currentBlock->dts;
                        extraSamples=0;
                        // Take either block Dts or our own if no DTS is provided
                        if(currentBlock->dts!=ADM_NO_PTS)
                        {
                            if( abs(currentBlock->dts-currentDts)>MP4V2_MAX_JITTER)
                            {
                                if(currentBlock->dts<currentDts)
                                    {
                                            ADM_warning("Audio going back in time audio track %d\n",audioIndex);
                                            ADM_warning("expected %d ms, got %d ms",currentDts/1000,currentBlock->dts/1000);
                                            ADM_warning("Dropping packet\n");
                                            goto nextOne;
                                    }
                                // We have a hole, increase duration of current packet
                                double holeDurationUs=currentBlock->dts-currentDts;
                                ADM_warning("Hole detected in audio of %d ms, track %d\n",(int)(holeDurationUs/1000),audioIndex);
                                holeDurationUs*=fq;
                                holeDurationUs/=1000*1000;
                                ADM_warning("Increasing hole duration by %d samples\n",(int)holeDurationUs);
                                extraSamples=(uint64_t)holeDurationUs;
                            }
                        }else       
                            blockDts=currentDts;
                        if(blockDts>targetDts) // In the future
                            break;
                        if(false==writeAudioBlock(audioIndex,currentBlock,currentBlock->nbSamples+extraSamples))
                        {
                            ADM_error("Cannot write audio sample for track %d\n",audioIndex);
                            pkt->eos=true;
                            return false;
                        }
                        // load next
nextOne:
                        clock->advanceBySample(currentBlock->nbSamples+extraSamples);
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



