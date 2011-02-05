/**
    \file ADM_psAudioProbe.cpp
    \brief Handle index file reading
    copyright            : (C) 2009 by mean
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
#include "ADM_audiodef.h"
#include "dmxTSPacket.h"
#include "ADM_tsAudioProbe.h"
//
#include "ADM_a52info.h"
#include "ADM_eac3info.h"
#include "ADM_mp3info.h"
#include "ADM_dcainfo.h"
#include "ADM_aacadts.h"
// Number of video packet seen to be enough to sample the audio tracks
#define PROBE_PACKET_VIDEO_COUNT 500
// Max size of a packet. Usually it is a bit more than 2300, so 10000 should be safe
#define PACKET_PROBE_SIZE (100*1024) // Damn tivo...
// Minimum of packet seen to declare it valid
#define PROBE_MIN_PACKET 5
#define PROBE_MIN_SIZE   5000

#define MP2_AUDIO_VALUE 0xC0
#define LPCM_AUDIO_VALUE 0xA0
#define DTS_AC3_AUDIO_VALUE 0x00

bool        
tsGetAudioInfo(tsPacketLinear *p,tsAudioTrackInfo *trackInfo);
static bool tsCheckMp2Audio(WAVHeader *hdr, uint8_t *data, uint32_t dataSize);
void        mixDump(uint8_t *ptr, uint32_t len);


/**
    \fn addAudioTrack
    \brief gather information about audio & add audio track to the list

*/
bool tsGetAudioInfo(tsPacketLinear *p,tsAudioTrackInfo *trackInfo)
{
#define PROBE_ANALYZE_SIZE 6000 // Should be enough in all cases (need ~ 2 blocks)
uint8_t audioBuffer[PROBE_ANALYZE_SIZE];
uint64_t pts,dts,startAt;
        
        
        // dont even try if it is not audio
        switch(trackInfo->trackType)
        {
            case ADM_TS_MPEG_AUDIO:
            case ADM_TS_AC3:
            case ADM_TS_EAC3:
            case ADM_TS_AAC:
                        break;
            default:
                ADM_warning("Unsupported audio track\n");
                return false;
        }
        // Go back where we were
        p->changePid(trackInfo->esId); 
        //Realign
        p->seek(0,0);
        // This is a special case...
        if(trackInfo->trackType==ADM_TS_AAC)
        {
            TS_PESpacket pes(trackInfo->esId);
            int retries=5;
            int offset;
            ADM_adts2aac  adts;
            while(retries)
            {
                if(false==p->getNextPES(&pes))
                {
                    ADM_warning("Cannot get pes packet for AAC track\n");
                    return false;
                }
                uint8_t *ptr=pes.payload+pes.offset;
                int size=pes.payloadSize-pes.offset;
                int dummySize=0;
                if(true==adts.convert(size,ptr,&dummySize,NULL))
                        break;;
                retries--;
            }
            if(!retries)
            {
                ADM_error("Cannot get AAC/ADTS info\n");
                return false;
            }
            trackInfo->wav.encoding=WAV_AAC;    
            trackInfo->wav.frequency=adts.getFrequency();
            trackInfo->wav.channels=adts.getChannels();
            trackInfo->wav.byterate=128000>>3;
            
            uint32_t eLen=0;     
            uint8_t *eData=NULL;
            adts.getExtraData(&eLen,&eData);
            
            if(2!=eLen) 
            {
                ADM_error("%d bytes of extradata, expecting 2");
                return false;
            }
            trackInfo->extraDataLen=2;
            trackInfo->extraData[0]=eData[0];
            trackInfo->extraData[1]=eData[1];
            ADM_info("AAC extra data : %02x %02x\n",eData[0],eData[1]);
            return true;
        }
        int rd=PROBE_ANALYZE_SIZE;
        if(!p->read(PROBE_ANALYZE_SIZE,audioBuffer))
        {
            printf("[tsAudioProbe] Cannot get info about pid %d 0x%x\n",trackInfo->esId,trackInfo->esId);
            return false;
        }
        uint32_t fq,br,chan,off;
        switch(trackInfo->trackType)
        {
            case ADM_TS_MPEG_AUDIO: // MP2
                            {
                                if(! tsCheckMp2Audio(&(trackInfo->wav),audioBuffer,rd))
                                {
                                    printf("[PsProbeAudio] Failed to get info on track :%x (MP2)\n",trackInfo->esId);
                                    goto er;
                                }
                            }
                            break;
            case ADM_TS_AC3: // AC3 or DTS
                            // AC3
                            {
                                trackInfo->wav.encoding=WAV_AC3;
                                if(!ADM_AC3GetInfo(audioBuffer, rd, &fq, &br, &chan,&off))
                                {
                                        printf("[PsProbeAudio] Failed to get info on track :%x\n",trackInfo->esId);
                                        goto er;
                                }
                                trackInfo->wav.frequency=fq;
                                trackInfo->wav.channels=chan;
                                trackInfo->wav.byterate=(br);
                                break;
                            }
            case ADM_TS_EAC3: 
                            {
                                trackInfo->wav.encoding=WAV_EAC3;
                                ADM_EAC3_INFO info;
                                if(!ADM_EAC3GetInfo(audioBuffer, rd, &off,&info))
                                {
                                        printf("[PsProbeAudio] Failed to get info on track :%x\n",trackInfo->esId);
                                        goto er;
                                }
                                trackInfo->wav.frequency=info.frequency;
                                trackInfo->wav.channels=info.channels;
                                trackInfo->wav.byterate=info.byterate;
                                break;
                            }
                            
            default:
                        printf("[tsAudioProbe] Unsupported audio format pid %d 0x%x\n",trackInfo->esId,trackInfo->esId);
                        return false;

        }
        return true;
er:
        return false;

}

/**
    \fn psCheckMp2Audio
    \brief Wait to have 2 audio packets to make sure it is not a false detection (that happens with mp2/mp3 audio)
*/
bool tsCheckMp2Audio(WAVHeader *hdr, uint8_t *data, uint32_t dataSize)
{
    MpegAudioInfo mpeg,first;
    uint32_t off2,off;

    hdr->encoding=WAV_MP2;
again:
    if(!getMpegFrameInfo(data, dataSize, &first,NULL,&off))
    {
            return false;
    }
    if(dataSize<(off+first.size))
    {
        return false;
    }
    if(!getMpegFrameInfo(data+off+first.size, dataSize-off-first.size, &mpeg,NULL,&off2))
    {
            return false;
    }
    if(off2) // false detectio ?
    {
        printf("[psAudioProbe] Mp2 : False MP2 header at %"LU"\n",off);
        if(dataSize<4) return false;
        data+=3;
        dataSize-=3;
        goto again;
    }
    hdr->frequency=mpeg.samplerate;
    if(mpeg.mode==3)
        hdr->channels=1;
    else
        hdr->channels=2;
    hdr->byterate=(mpeg.bitrate*1000)>>3;
    return true;
}
//EOF
