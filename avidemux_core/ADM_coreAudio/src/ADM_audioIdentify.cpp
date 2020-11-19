/**
    \file  ADM_audioIdentify
    \brief Identify a codec used in a file

    \author copyright            : (C) 2012 by mean
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
#include "ADM_mp3info.h"
#include "ADM_a52info.h"
#include "ADM_eac3info.h"
#include "ADM_audioCodecEnum.h"
#include "ADM_audioIdentify.h"
#include "fourcc.h"
#include "ADM_aacadts.h"
#include "ADM_dcainfo.h"

extern void Endian_WavHeader(WAVHeader *w);
#define INVALID_OFFSET 0XFFFFFFF
/**
    \fn idMP2
    \brief return true if the tracks is mp2
*/
static bool idMP2(int bufferSize,const uint8_t *data,WAVHeader &info,uint32_t &offset)
{
    const uint8_t *mp2Buffer=data;
    const int limit=bufferSize;
    offset=INVALID_OFFSET;
    // Now read buffer until we have 3 correctly decoded packet
    int probeIndex=0;
    int failAttempt=0;
#define MAX_FAIL 20
    while(probeIndex<limit)
    {
        const uint8_t *ptr=mp2Buffer+probeIndex;
        int len=limit-probeIndex;
        if(len<4)
        {
            ADM_info("No more data.\n");
            return false;
        }
        uint32_t syncoff,syncoff2;
        MpegAudioInfo mp2info,confirm;
        if(!getMpegFrameInfo(ptr,len,&mp2info,NULL,&syncoff))
        {
            ADM_info("No sync\n");
            return false;
        }
        probeIndex+=syncoff;
        if(INVALID_OFFSET==offset || probeIndex>offset)
            offset=probeIndex;
        // Skip this packet
        int next=probeIndex+mp2info.size;
        len=limit-next;
        if(len<4)
        {
            ADM_info("Not enough data to confirm detection.\n");
            return false;
        }
        for(int i=0; i < 2; i++)
        {
            if(!getMpegFrameInfo(mp2Buffer+next,len,&confirm,&mp2info,&syncoff2))
            {
                ADM_info("No sync to confirm detection.\n");
                return false;
            }
            if(syncoff2)
            {
                probeIndex=next+syncoff2;
                if(++failAttempt > MAX_FAIL)
                {
                    ADM_warning("Giving up after %d failures to get stable sync.\n",MAX_FAIL);
                    return false;
                }
                ADM_info("Frame size doesn't match, will retry at offset %d\n",probeIndex);
                break;
            }
            if(!i)
            {
                next+=confirm.size;
                len-=confirm.size;
                if(len<4)
                {
                    ADM_info("Not enough data to get 3 matches in a row.\n");
                    return false;
                }
                continue;
            }
            ADM_info("Probably MP2/3 : fq=%u br=%u mode=%u failures=%d\n", mp2info.samplerate, mp2info.bitrate, mp2info.mode, failAttempt);
            // fill in info
            info.frequency=mp2info.samplerate;
            info.byterate=(mp2info.bitrate>>3)*1000;
            if(mp2info.layer==3)
                info.encoding=WAV_MP3;
            else
                info.encoding=WAV_MP2;
            switch(mp2info.mode)
            {
                case 3: // mono
                    info.channels=1;
                    break;
                case 0: // Stereo
                case 1: // Joint stereo
                case 2: // dual channel
                default:
                    info.channels=2;
                    break;
            }
            return true;
        }
    }
    return false;
}
/**
 * \fn idWAV
 * \brief returns true if the file is a riff/wav file
 */
#define wRead32(x) {x=( cur[0]+(cur[1]<<8)+(cur[2]<<16)+(cur[3]<<24));cur+=4;ADM_assert(cur<=tail);}
#define wRead16(x) {x=( cur[0]+(cur[1]<<8));cur+=2;ADM_assert(cur<=tail);}
static bool idWAV(int bufferSize,const uint8_t *data,WAVHeader &info,uint32_t &offset)
{
    const uint8_t *cur=data,*tail=data+bufferSize;
    uint32_t t32;
    uint32_t totalSize;
    offset=0;
    wRead32(t32);
    ADM_info("Checking if it is riff/wav...\n");
    if(!fourCC::check( t32, (uint8_t *) "RIFF"))
    {
          ADM_warning("Not riff.\n");
          fourCC::print(t32);
          goto drop;
    }
    wRead32(totalSize);
    ADM_info("\n %lu bytes total \n", totalSize);

    wRead32(t32);
    if(!fourCC::check( t32, (uint8_t *) "WAVE"))
    {
        ADM_warning("\n no wave chunk..aborting..\n");
        goto drop;
    }
    wRead32(t32);
    if(!fourCC::check( t32, (uint8_t *) "fmt "))
    {
        ADM_warning("\n no fmt chunk..aborting..\n");
        goto drop;
    }
    wRead32(t32);
    if(t32 < sizeof(WAVHeader))
    {
        ADM_warning("\n incorrect fmt chunk..(%ld/%d)\n",t32,sizeof(WAVHeader));
        goto drop;
    }
    // read wavheader
    memcpy(&info,cur,sizeof(WAVHeader));
    cur+=t32;
    if (t32>sizeof(WAVHeader))
        ADM_warning("There are some extradata!\n");
    ADM_assert(cur<tail);

    // For our big endian friends
    Endian_WavHeader(&info);

    wRead32(t32);
    if(!fourCC::check( t32, (uint8_t *) "data"))
    {
        // Maybe other chunk, skip at most one
        wRead32(t32);
        cur+=t32;
        ADM_assert(cur+4<tail);
        wRead32(t32);
        if (!fourCC::check( t32, (uint8_t *) "data"))
        {
            ADM_warning("\n no data chunk..aborting..\n");
            goto drop;
        }
    }

    wRead32(t32);
    ADM_info(" %lu bytes data \n", totalSize);
    info.encoding=WAV_PCM;
    ADM_info("Block alignment: %u\n",info.blockalign);
    offset=(uint32_t)(cur-data);
    ADM_info("yes, it is riff/wav, data starts at %d...\n",(int)offset);
    return true;
drop:
    ADM_info("No, not riff/wav...\n");
    return false;
}
/**
 * \fn detectAC3Internal
 */
static bool detectAC3Internal(int bufferSize, const uint8_t *data, WAVHeader &oinfo, uint32_t &offset, bool enhanced)
{
    uint32_t syncOffset,nextOffset;
    ADM_EAC3_INFO info,info2;

    if( !ADM_EAC3GetInfo(data, bufferSize, &syncOffset, &info, !enhanced))
    {
        if(enhanced)
            ADM_info("Not EAC3\n");
        else
            ADM_info("Not AC3\n");
        return false;
    }
    offset=syncOffset;
    nextOffset=syncOffset+info.frameSizeInBytes;
    ADM_info("First sync at offset %u, freq = %u br = %u chan = %u\n",syncOffset,info.frequency,info.byterate<<3,info.channels);
    // Need 2 more frames to be sure...
    bool confirmed=true;
    uintptr_t tmp=(uintptr_t)data;
    tmp+=nextOffset;
    int remaining=bufferSize-nextOffset;
    const uint32_t max_byterate_deviation=1000;

    for(int i=0;i<2;i++)
    {
        ADM_info("\t pass %d\n",i);
        if(remaining<0)
        {
            if(enhanced)
                ADM_warning("Not enough data to confirm EAC3\n");
            else
                ADM_warning("Not enough data to confirm AC3\n");
            confirmed=false;
            break;
        }

        const uint8_t *data2=(uint8_t *)tmp;

        if( !ADM_EAC3GetInfo(data2, remaining, &syncOffset, &info2, !enhanced))
        {
            ADM_info("Cannot sync (pass %d)\n",i);
            confirmed=false;
            break;
        }
        if(syncOffset)
        {
            ADM_info("Offset between frames present = %" PRIu32" (pass %d)\n",syncOffset,i);
            confirmed=false;
            break;
        }
        ADM_info("Next sync at offset %u, freq = %u bitrate = %u chan = %u\n",nextOffset+syncOffset,info2.frequency,info2.byterate<<3,info2.channels);
        if(info.frequency!=info2.frequency || info.channels!=info2.channels)
        {
            ADM_info("Info doesn't match (pass %d)\n",i);
            confirmed=false;
            break;
        }
        if(info.byterate>info2.byterate+max_byterate_deviation || info.byterate+max_byterate_deviation<info2.byterate)
        {
            ADM_info("Byterate variance too high: %d (limit = %d)\n",
                (info.byterate>info2.byterate ? (int)(info.byterate-info2.byterate) : (int)(info2.byterate-info.byterate)),
                (int)max_byterate_deviation);
            confirmed=false;
            break;
        }
        int sz=syncOffset+info2.frameSizeInBytes;
        nextOffset+=sz;
        tmp+=sz;
        remaining-=sz;
    }
    if(confirmed)
    {
        ADM_info("\tProbably %s: freq=%d byterate=%d chan=%d, offset=%d\n",enhanced? "EAC3" : "AC3",
                 (int)info.frequency,(int)info.byterate,(int)info.channels,(int)offset);
        oinfo.encoding=enhanced? WAV_EAC3 : WAV_AC3;
        oinfo.channels=info.channels;
        oinfo.byterate=info.byterate; // already in bytes/sec
        oinfo.frequency=info.frequency;
        return true;
    }
    if(enhanced)
        ADM_info("Cannot confirm EAC3\n");
    else
        ADM_info("Cannot confirm AC3\n");
    return false;
}

/**
    \fn idEAC3
    \brief return true if the file is EAC3
*/
static bool idEAC3(int bufferSize,const uint8_t *data,WAVHeader &oinfo,uint32_t &offset)
{
    return detectAC3Internal(bufferSize,data,oinfo,offset,true);
}

/**
    \fn idAC3
    \brief return true if the file is AC3
*/
static bool idAC3(int bufferSize,const uint8_t *data,WAVHeader &oinfo,uint32_t &offset)
{
    return detectAC3Internal(bufferSize,data,oinfo,offset,false);
}

/**
 * 
 * @param bufferSize
 * @param data
 * @param info
 * @param offset
 * @return 
 */
static bool idAAACADTS(int bufferSize,const uint8_t *data,WAVHeader &info,uint32_t &offset)
{
    int firstOffset,expected,sync=0;
    ADM_adts2aac aac;
    uint8_t out[ADTS_MAX_AAC_FRAME_SIZE];
    const uint8_t *start=data;
    const uint8_t *end=data+bufferSize;
    offset=firstOffset=expected=0;
    while(start<end)
    {
        int incoming=500;
        int off,outLen=0;
        if(start+500>end) incoming=end-start;
        bool r=false;
        if(incoming>0)
            r=aac.addData(incoming,start);
        ADM_adts2aac::ADTS_STATE state=aac.getAACFrame(&outLen,out,&off);
        start+=incoming;
        switch(state)
        {
            case ADM_adts2aac::ADTS_ERROR:
                    return false;
            case ADM_adts2aac::ADTS_MORE_DATA_NEEDED:
                    if(r) continue;
                    return false;
            case ADM_adts2aac::ADTS_OK:
                    // Got sync
                    if(!sync)
                        firstOffset=off;
                    else if(off>expected)
                    {
                        ADM_warning("Skipped at least %d bytes between frames, assuming a false positive.\n",off-expected);
                        return false;
                    }
                    sync++;
                    expected=off+outLen+9; // ADTS header max size
                    ADM_info("Sync %d at offset %d, frame size %d\n",sync,off,outLen);
                    if(sync<3)
                        continue;
                    info.encoding=WAV_AAC;
                    info.channels=aac.getChannels();
                    info.blockalign=0;
                    info.bitspersample=16;
                    info.byterate=128000>>3;
                    info.frequency=aac.getFrequency();
                    offset=firstOffset;
                    ADM_info("Detected as AAC, fq=%d, channels=%d, offset=%d\n",info.frequency,info.channels,offset);
                    return true;
            default:
                ADM_assert(0);
                break;
        }
    }
    return false;
}

/**
    \fn idDCA
    \brief return true if the track is DTS
*/
static bool idDCA(int bufferSize, const uint8_t *data, WAVHeader &oinfo, uint32_t &offset)
{
    uint32_t syncOffset,packetLen;
    ADM_DCA_INFO info,info2;
    uintptr_t tmp=(uintptr_t)data;
    uint8_t *probe=(uint8_t *)tmp;

    if( !ADM_DCAGetInfo(probe, bufferSize, &info, &syncOffset))
    {
        ADM_info("Not DTS.\n");
        return false;
    }
    packetLen=info.frameSizeInBytes;

    // Need 2 more frames to be sure...
    bool r=true;

    for(int i=0;i<2;i++)
    {
        ADM_info("\t pass %d\n",i);
        bufferSize-=syncOffset+packetLen;
        if(bufferSize<=0)
        {
            ADM_warning("Not enough data to confirm DTS.\n");
            return false;
        }
        probe+=syncOffset+packetLen;

        if( !ADM_DCAGetInfo(probe, bufferSize, &info2, &syncOffset))
        {
            ADM_info("Cannot sync (pass %d)\n",i);
            return false;
        }
        packetLen=info2.frameSizeInBytes;
        if(info.frequency!=info2.frequency || info.channels!=info2.channels || info.bitrate!=info2.bitrate)
        {
            ADM_info("Info doesn't match (pass %d)\n",i);
            r=false;
            break;
        }
        if(syncOffset>2)
        {
            ADM_info("Offset between frames too big = %" PRIu32" (pass %d)\n",syncOffset,i);
            r=false;
            break;
        }
    }
    if(r)
    {
        ADM_info("\tProbably DTS : freq=%d br=%d chan=%d, offset=%d\n",(int)info.frequency,(int)info.bitrate,(int)info.channels,(int)syncOffset);
        oinfo.encoding=WAV_DTS;
        oinfo.channels=info.channels;
        oinfo.byterate=info.bitrate>>3;
        oinfo.frequency=info.frequency;
        return true;
    }else
    {
        ADM_info("Cannot confirm DTS.\n");
    }
    return r;
}

/**
    \fn ADM_identifyAudioStream
    \param bufferSize : nb of bytes available for scanning
    \param buffer     : buffer containing the data to scan
    \param info       : wavheader to fill with detected codec/channels/etcc
    \param offset     : offset in the file where payload starts
*/
bool ADM_identifyAudioStream(int bufferSize,const uint8_t *buffer,WAVHeader &info,uint32_t &offset)
{
    memset(&info,0,sizeof(info));
    if(idWAV(bufferSize,buffer,info,offset)) return true;
    if(idMP2(bufferSize,buffer,info,offset)) return true;
    if(idEAC3(bufferSize,buffer,info,offset)) return true;
    if(idAC3(bufferSize,buffer,info,offset)) return true;
    if(idAAACADTS(bufferSize,buffer,info,offset)) return true;
    if(idDCA(bufferSize,buffer,info,offset)) return true;

    return false;
}
// EOF
