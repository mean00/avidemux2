/**

        \file ADM_dcainfo
        \brief extract info from DTS/DCA streams
        Author: mean <fixounet@free.fr>, (C) 2004
        Code very derived from libdca

*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_dcainfo.h"
#include "ADM_getbits.h"
#undef printf
/*
        Borrowed from libdca
*/
static const int dts_sample_rates[] =
{
    0, 8000, 16000, 32000, 0, 0, 11025, 22050, 44100, 0, 0,
    12000, 24000, 48000, 96000, 192000
}; // the last two are invalid according to spec

static const int dts_bit_rates[] =
{
    32000, 56000, 64000, 96000, 112000, 128000,
    192000, 224000, 256000, 320000, 384000,
    448000, 512000, 576000, 640000, 768000,
    960000, 1024000, 1152000, 1280000, 1344000,
    1408000, 1411200, 1472000, 1536000, 1920000/* not in spec */,
    2048000/* not in spec */, 3072000/* not in spec */, 3840000/* not in spec */,
    1/*open*/, 2/* variable (not in spec) */, 3/* lossless (not in spec) */
};

static const uint8_t dts_channels[] =
{
    1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 8, 8
};

/**
    \fn getSubstreamSize
    \brief Decode a small chunk of the header to get substream size.
*/
static bool getSubstreamSize(uint8_t *buf, uint32_t len, uint32_t *size)
{
    if(len < DTS_HEADER_SIZE)
        return false;

    if(buf[0]!=0x64 || buf[1]!=0x58 || buf[2]!=0x20 || buf[3]!=0x25)
        return false;

    // getBits buffer must be padded by AV_INPUT_BUFFER_PADDING_SIZE zerobytes
#define PAD_SIZE (DTS_HEADER_SIZE + 64)
    uint8_t hdr[PAD_SIZE]={0};
    uint32_t framesize=0;

    memcpy(hdr,buf,DTS_HEADER_SIZE);
    memset(hdr+DTS_HEADER_SIZE,0,64);
    getBits bits(DTS_HEADER_SIZE,hdr);
    bits.skip(32); // syncword
    bits.skip(8); // user defined bits
    bits.skip(2); // extension substream index
    if(bits.get(1)) // header size flag
    {
        bits.skip(12); // header size
        framesize=bits.get(20);
    }else
    {
        bits.skip(8);
        framesize=bits.get(16);
    }
    framesize++;
    if(framesize < DTS_HEADER_SIZE) // FIXME
        return false;
    *size=framesize;
    return true;
}

/**
    \fn ADM_DCAGetInfo
    @param syncoff: # of dropped bytes from the begining
*/
bool ADM_DCAGetInfo(uint8_t *buf, uint32_t len, ADM_DCA_INFO *info, uint32_t *syncoff, bool substream)
{
    bool sync=false;
    uint8_t *end=buf+len-4-DTS_HEADER_SIZE;
    uint8_t *cur=buf-1;
    uint32_t len1,len2,flags,framesize=0,index,nbBlocks;

    *syncoff=0;

    if(substream) // skip decoding DTS core header, buf points to the substream marker
    {
        uint32_t extra;
        if(getSubstreamSize(buf,len,&extra))
        {
            framesize=info->frameSizeInBytes;
            framesize=(framesize+3)&(~3);
            framesize+=extra;
            info->frameSizeInBytes=framesize;
            return true;
        }
        return false;
    }

    uint8_t pad[PAD_SIZE]={0};

    // Assume 16 bits big endian
    // Search for 7F FE 80 01 as sync start
    while(cur<end)
    {
        cur++;
        if(cur[0]!=0x7F || cur[1]!=0xFE || cur[2]!=0x80 || cur[3]!=0x01)
            continue;
        sync=true;
        *syncoff=cur-buf;
        if(*syncoff) ADM_warning("[dts] Dropped %u bytes\n",*syncoff);
        uint8_t *base=cur;
        // ok we got a starcode
        // State :      32 bits, already got them
        // Frame type   1
        // Sample Deficit 5
        // CRC present  1
        // Frame length  7
        // Frame Size 14
        // **** Inefficient ! ****
        memcpy(pad,cur,DTS_HEADER_SIZE);
        getBits bits(DTS_HEADER_SIZE,cur);
        bits.skip(32);
        int termination=bits.get(1);
        int sampleDeficit=bits.get(5)+1;
#define DTS_PCM_BLOCK_SAMPLES 32
        if(!termination && sampleDeficit != DTS_PCM_BLOCK_SAMPLES) // invalid data, bail out
            return false;
        bits.skip(1);
        nbBlocks=1+bits.get(7); //Nb Samples
        if(nbBlocks & 7)
        {
            ADM_warning("Invalid number of PCM blocks, should be a multiple of 8\n");
            return false;
        }
        len2=bits.get(14);
        framesize=len2+1; // Frame size in bytes including the marker
        if(framesize < 96)
        {
            ADM_warning("Invalid DTS core frame size %u\n",framesize);
            return false;
        }
        info->frameSizeInBytes=framesize;
        framesize=(framesize+3)&(~3);
        flags=bits.get(6);
        if(flags>15)
        {
            ADM_warning("User defined DTS audio modes are not supported (%d)\n",flags);
            return false;
        }
        info->flags=flags;
        index=bits.get(4);
        if(!dts_sample_rates[index])
        {
            ADM_warning("Invalid sample rate index, skipping frame.\n");
            return false;
        }
        info->frequency=dts_sample_rates[index];
        index=bits.get(5);
        info->bitrate=dts_bit_rates[index];
        if(bits.get(1)) // reserved bit
        {
            ADM_warning("Reserved bit set, skipping frame.\n");
            return false;
        }
#if 0
                printf("[dts]Flags  :%u\n",flags);
                printf("[dts]Fq  :%u\n",*fq);
                printf("[dts]br  :%u\n",*br);
                printf("[dts]len1  :%u\n",len1);
                printf("[dts]len2  :%u\n",len2);
#endif
        // Dynamic range flag       1
        // Timestamp present        1
        // Auxiliary data present   1
        // Source is HDCD           1
        // Extension audio desc     3
        // Extended coding present  1
        // Syncword insertion flag  1
        bits.skip(9);
        int lfe=bits.get(2); // ignoring the rest of the core header
        if(lfe==3)
        {
            ADM_warning("Invalid LFE flag.\n");
            return false;
        }
        int c=dts_channels[flags & 0xf];
        if(c==5 && lfe) c++; // LFE
        info->channels=c;
        info->samples=nbBlocks*DTS_PCM_BLOCK_SAMPLES;

        if(*syncoff+framesize+4 >= len) // no extension
            return true;
        break;
    } // while
    if(!sync)
    {
        ADM_warning("No sync, expected: 7F FE 80 01, got: %02x %02x %02x %02x\n",buf[0],buf[1],buf[2],buf[3]);
        return false;
    }

    cur+=framesize;
    // Check for substream marker 64 58 20 25
    if(cur[0]!=0x64 || cur[1]!=0x58 || cur[2]!=0x20 || cur[3]!=0x25)
        return true;

    uint32_t remaining=cur-buf;
    remaining=len-remaining;
    if(remaining < DTS_HEADER_SIZE) // FIXME
    {
        ADM_warning("Substream marker present, but data too short. Truncated frame?\n");
        return true;
    }
    // Get the size of the substream, we don't care about the content
    if(getSubstreamSize(cur,remaining,&framesize))
        info->frameSizeInBytes=((info->frameSizeInBytes+3)&(~3))+framesize;

    return true;
}

