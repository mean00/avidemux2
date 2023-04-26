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
#include "ADM_eac3info.h"
#include "ADM_mp3info.h"
#include "ADM_dcainfo.h"
#include "ADM_aacadts.h"
#include "ADM_aacLatm.h"
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

/**
    \fn addAudioTrack
    \brief gather information about audio & add audio track to the list

*/
bool tsGetAudioInfo(tsPacketLinear *p,tsAudioTrackInfo *trackInfo)
{
#define PROBE_ANALYZE_SIZE 8000 // Should be enough in all cases (need ~ 2 blocks)
uint8_t audioBuffer[PROBE_ANALYZE_SIZE];
        
        trackInfo->extraDataLen=0;
        // dont even try if it is not audio
        switch(trackInfo->trackType)
        {
            case ADM_TS_MPEG_AUDIO:
            case ADM_TS_AC3:
            case ADM_TS_DTS:
            case ADM_TS_EAC3:
            case ADM_TS_AAC_ADTS:
            case ADM_TS_AAC_LATM:
            case ADM_TS_LPCM:
                        break;
            default:
                ADM_warning("Unsupported audio track (%d)\n",trackInfo->trackType);
                return false;
        }
        // Go back where we were
        p->changePid(trackInfo->esId); 
        //Realign
        p->seek(0,0);
        // This is a special case...
        // For ADTS, we read one packet and ask ffmpeg to extract
        // Extra data, frequency etc...
        ADM_TS_TRACK_TYPE trackType=trackInfo->trackType;

        //--------------------- AAC -------------------------
        if(trackType==ADM_TS_AAC_ADTS || trackType==ADM_TS_AAC_LATM)
        {
            // Step 1, try to get a packet...
            TS_PESpacket pes(trackInfo->esId);
            int retries=5;
            uint32_t i,eLen=0;
            uint8_t *eData=NULL;

            trackInfo->wav.encoding=WAV_AAC;  

            if(false==p->getNextPES(&pes))
                {
                    ADM_warning("Cannot get pes packet for AAC track\n");
                    return false;
                }
            retries--;    
            
            // try to decode header..
            uint8_t *ptr=pes.payload+pes.offset;
            int size=pes.payloadSize-pes.offset;
            switch(trackType)
            {
                default:
                case ADM_TS_AAC_LATM:
                {
                    ADM_latm2aac latm;
                    ADM_info("Looking up LATM info\n");
                    retries=20;
                    while(retries)
                    {
                        ptr=pes.payload+pes.offset;
                        size=pes.payloadSize-pes.offset;
                        if(false==latm.pushData(size,ptr))
                            latm.flush();
                        int errors=8; // arbitrary
                        while(true)
                        {
                            ADM_latm2aac::LATM_STATE outcome=latm.convert(0);
                            if(outcome==ADM_latm2aac::LATM_ERROR && errors-- > 0)
                                continue;
                            if(outcome==ADM_latm2aac::LATM_MORE_DATA_NEEDED)
                                break;
                            if(latm.getFrequency())
                            {
                                ADM_assert(latm.getExtraData(&eLen,&eData));
                                uint32_t fq=latm.getFrequency();
                                if(fq<=24000) fq*=2; // implicit SBR?
                                trackInfo->wav.frequency=fq;
                                trackInfo->wav.channels=latm.getChannels();
                                trackInfo->wav.byterate=128000>>3;
                                trackInfo->extraDataLen=eLen;
                                for(i=0;i<eLen;i++)
                                    trackInfo->extraData[i]=eData[i];
                                trackInfo->mux=ADM_TS_MUX_LATM;
                                ADM_info("AAC extra data (%d): %02x %02x\n",eLen,eData[0],eData[1]);
                                return true;
                            }
                        }
                        // next packet
                        retries--;
                        if(false==p->getNextPES(&pes))
                        {
                            ADM_error("Cannot get next PES packet for LATM extradata\n");
                            return false;
                        }
                    }
                    ADM_error("LATM : Cannot get codec extra data\n");
                    return false;
                    //
                    break;
                }
                case ADM_TS_AAC_ADTS:
                    {
                    ADM_adts2aac  adts;
                    trackInfo->mux=ADM_TS_MUX_ADTS;
                    int dummySize=0;
                    while(ADM_adts2aac::ADTS_OK!=adts.convert2(size,ptr,&dummySize,NULL)) // We dont need the output hence the null
                    {
                        ADM_info("ADTS no sync\n");
                        if(!retries || false==p->getNextPES(&pes))
                        {
                            ADM_warning("Cannot get pes packet for AAC track\n");
                            return false;
                        }
                        ptr=pes.payload+pes.offset;
                        size=pes.payloadSize-pes.offset;
                        retries--;
                    }
                    adts.getExtraData(&eLen,&eData);
                    if( eLen!=2) 
                    {
                        ADM_error("%d bytes of extradata, expecting 2\n");
                        return false;
                    }
                    trackInfo->extraDataLen=eLen;
                    for(i=0;i<eLen;i++)
                        trackInfo->extraData[i]=eData[i];
                    ADM_info("AAC extra data %d: %02x %02x\n",eLen,eData[0],eData[1]);
                    trackInfo->wav.frequency=adts.getFrequency();
                    trackInfo->wav.channels=adts.getChannels();
                    trackInfo->wav.byterate=128000>>3;
                    return true;
                    }
                    break;
            }
            
            return false;
        }
        // -------------- /AAC ------------------
        int rd=PROBE_ANALYZE_SIZE;
        if(!p->read(PROBE_ANALYZE_SIZE,audioBuffer))
        {
            ADM_error("Cannot get info about pid %d 0x%x\n",trackInfo->esId,trackInfo->esId);
            return false;
        }
        uint32_t fq,br,chan,off;
        switch(trackInfo->trackType)
        {
            case ADM_TS_MPEG_AUDIO: // MP2
                            {
                                if(! tsCheckMp2Audio(&(trackInfo->wav),audioBuffer,rd))
                                {
                                    ADM_error("Failed to get info on track : 0x%x (MP2)\n",trackInfo->esId);
                                    goto er;
                                }
                            }
                            break;
            case ADM_TS_LPCM:
                                trackInfo->wav.encoding=WAV_LPCM;
                                trackInfo->wav.frequency=48000;
                                trackInfo->wav.channels=2;
                                trackInfo->wav.byterate=2*2*48000;
                                break;
            case ADM_TS_DTS:
                            {
                                trackInfo->wav.encoding=WAV_DTS;
                                ADM_DCA_INFO dcainfo;
                                if(false==ADM_DCAGetInfo(audioBuffer, rd, &dcainfo, &off))
                                {
                                    ADM_error("Failed to get info on track : 0x%x (DTS)\n",trackInfo->esId);
                                    goto er;
                                }
                                trackInfo->wav.frequency=dcainfo.frequency;
                                trackInfo->wav.channels=dcainfo.channels;
                                trackInfo->wav.byterate=dcainfo.bitrate/8;
                                break;
                            }
            case ADM_TS_AC3:
            case ADM_TS_EAC3:
                            {
                                bool ac3;
                                ADM_EAC3_INFO info;
                                if(!ADM_EAC3GetInfo(audioBuffer, rd, &off, &info, &ac3))
                                {
                                    ADM_error("Failed to get info on track : 0x%x (%s)\n", trackInfo->esId, (trackInfo->trackType == ADM_TS_AC3)? "AC3" : "EAC3");
                                    goto er;
                                }
                                trackInfo->wav.encoding = ac3 ? WAV_AC3 : WAV_EAC3;
                                trackInfo->wav.frequency=info.frequency;
                                trackInfo->wav.channels=info.channels;
                                trackInfo->wav.byterate=info.byterate;
                                break;
                            }
            default:
                        ADM_error("Unsupported audio format pid %d (0x%x)\n",trackInfo->esId,trackInfo->esId);
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
        printf("[psAudioProbe] Mp2 : False MP2 header at %" PRIu32"\n",off);
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
