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
};

static const int dts_bit_rates[] =
{
    32000, 56000, 64000, 96000, 112000, 128000,
    192000, 224000, 256000, 320000, 384000,
    448000, 512000, 576000, 640000, 768000,
    896000, 1024000, 1152000, 1280000, 1344000,
    1408000, 1411200, 1472000, 1536000, 1920000,
    2048000, 3072000, 3840000, 1/*open*/, 2/*variable*/, 3/*lossless*/
};

static const uint8_t dts_channels[] =
{
    1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 8, 8
};

/**
    \fn ADM_DCAGetInfo
    @param syncoff: # of dropped bytes from the begining
*/
bool ADM_DCAGetInfo(uint8_t *buf, uint32_t len,ADM_DCA_INFO *info,uint32_t *syncoff)
//int  ADM_DCAGetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *br, uint32_t *chan,uint32_t *syncoff,uint32_t *flagso,uint32_t *nbSample)
{
uint8_t *end=buf+len-4-DTS_HEADER_SIZE;
uint8_t *cur=buf-1;
uint32_t size,len1,len2,flags,sr,framesize=0,index,nbBlocks;
             // Assume 16 bits big endian
            // Search for 7F FE 80 01 as sync start
            *syncoff=0;
            while(cur<end)
            {
                cur++;
                if(*cur!=0x7F) continue;
                if(cur[1]!=0xfe) continue;
                if(cur[2]!=0x80) continue;
                if(cur[3]!=0x01) continue;
                // ok we got a starcode
                // State :      32 bits, already got them
                // Frame type   1
                // Sample Deficit 5
                // CRC present  1
                // Frame length  7
                // Frame Size 14
                // **** Inefficient ! ****
                getBits bits((int)(end-cur),cur);
                bits.skip(32);
                bits.skip(1);
                bits.skip(5);
                bits.skip(1);
                //Nb Samples
                nbBlocks=1+bits.get(7);
                // Frame size in bit
                len2=bits.get(14);
                framesize=len2+1;
                //
                //
                //
                flags=bits.get(6);
                info->flags=flags;
                index=bits.get(4);
                info->frequency=dts_sample_rates[index];
                index=bits.get(5);
                info->bitrate=dts_bit_rates[index];
#if 0
                printf("[dts]Flags  :%u\n",flags);
                printf("[dts]Fq  :%u\n",*fq);
                printf("[dts]br  :%u\n",*br);
                printf("[dts]len1  :%u\n",len1);
                printf("[dts]len2  :%u\n",len2);
#endif
                *syncoff=cur-buf;
                if(*syncoff) ADM_warning("[dts] Dropped %u bytes\n",*syncoff);
                bits.get(10);
                int lfe=bits.get(2);
                int c;
                c=dts_channels[flags & 0xf];
                if(c==5 && lfe) c++; // LFE
                info->channels=c;
                info->samples=nbBlocks*32;
                info->frameSizeInBytes=framesize;
                return true;


            }
            ADM_warning("[DTS] Cannot find sync %x %x %x %x\n",buf[0],buf[1],buf[2],buf[3]);
	      return false;
}

