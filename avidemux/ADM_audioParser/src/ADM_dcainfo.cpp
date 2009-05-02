/**

        \file ADM_dcainfo
        \brief extract info from DTS/DCA streams
        Author: mean <fixounet@free.fr>, (C) 2004
        Code very derived from libdca

*/
#include "ADM_default.h"
#include "ADM_dcainfo.h"
#define ADM_NO_CONFIG_H
extern "C"
{
#include "ADM_libraries/ADM_ffmpeg/ffmpeg_config/config.h"
#include "ADM_libraries/ADM_ffmpeg/libavutil/internal.h"
#include "ADM_libraries/ADM_ffmpeg/libavutil/common.h"
#include "ADM_libraries/ADM_ffmpeg/libavutil/bswap.h"
#include "ADM_libraries/ADM_ffmpeg/libavcodec/get_bits.h"

}
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

/*
    Return frame size
*/
int  ADM_DCAGetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *br, uint32_t *chan,uint32_t *syncoff,uint32_t *flagso,uint32_t *nbSample)
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
                GetBitContext s;
                init_get_bits( &s,cur, (end-cur)*8);
                skip_bits(&s,32);
                skip_bits(&s,1);
                skip_bits(&s,5);
                skip_bits(&s,1);
                //Nb Samples
                nbBlocks=(get_bits(&s,7)+1);
                // Frame size in bit
                len2=get_bits(&s,14);
                framesize=len2+1;
                //
                //  
                //
                flags=get_bits(&s,6);
                *flagso=flags;
                index=get_bits(&s,4); 
                *fq=dts_sample_rates[index];
                index=get_bits(&s,5); 
                *br=dts_bit_rates[index];
#if 0
                printf("[dts]Flags  :%u\n",flags);
                printf("[dts]Fq  :%u\n",*fq);
                printf("[dts]br  :%u\n",*br);
                printf("[dts]len1  :%u\n",len1);
                printf("[dts]len2  :%u\n",len2);
#endif
                *syncoff=cur-buf;
                if(*syncoff) printf("[dts] Dropped %u bytes\n",*syncoff);
                *chan=dts_channels[flags & 0xf];
//                if(*chan==5 && (flags & 0X80)) *chan++;
                *nbSample=nbBlocks*32;
                return framesize;
                
                
            }
            printf("[DTS] Cannot find sync\n");
	      return 0;
}

