/**

        \file ADM_eac3info
        \brief extract info from EAC3/A52B/DD+ streams
        Author: mean <fixounet@free.fr>, (C) 2009
        Code very derived from ffmpeg (tables etc...)

*/

#include "ADM_default.h"
#include "ADM_eac3info.h"
#include "ADM_getbits.h"
extern "C"
{
#include "libavcodec/avcodec.h"
};

/* The tables below are copied from libavcodec/ac3tab.c */

static const int freqsTable[] = { 48000, 44100, 32000, 0 };
static const int ratesTable[19] = {
     32,  40,  48,  56,  64,  80,  96, 112, 128, 160,
    192, 224, 256, 320, 384, 448, 512, 576, 640
};
static const uint8_t channelTable[8] = { 2, 1, 2, 3, 3, 4, 4, 5 };
static const int frameSizeTable[38][3] = {
    { 64,   69,   96   },
    { 64,   70,   96   },
    { 80,   87,   120  },
    { 80,   88,   120  },
    { 96,   104,  144  },
    { 96,   105,  144  },
    { 112,  121,  168  },
    { 112,  122,  168  },
    { 128,  139,  192  },
    { 128,  140,  192  },
    { 160,  174,  240  },
    { 160,  175,  240  },
    { 192,  208,  288  },
    { 192,  209,  288  },
    { 224,  243,  336  },
    { 224,  244,  336  },
    { 256,  278,  384  },
    { 256,  279,  384  },
    { 320,  348,  480  },
    { 320,  349,  480  },
    { 384,  417,  576  },
    { 384,  418,  576  },
    { 448,  487,  672  },
    { 448,  488,  672  },
    { 512,  557,  768  },
    { 512,  558,  768  },
    { 640,  696,  960  },
    { 640,  697,  960  },
    { 768,  835,  1152 },
    { 768,  836,  1152 },
    { 896,  975,  1344 },
    { 896,  976,  1344 },
    { 1024, 1114, 1536 },
    { 1024, 1115, 1536 },
    { 1152, 1253, 1728 },
    { 1152, 1254, 1728 },
    { 1280, 1393, 1920 },
    { 1280, 1394, 1920 }
};
static const uint8_t eac3NbBlocks[4] = { 1, 2, 3, 6 };
static const uint16_t ac3ChannelLayoutTable[8] = {
    AV_CH_LAYOUT_STEREO,
    AV_CH_LAYOUT_MONO,
    AV_CH_LAYOUT_STEREO,
    AV_CH_LAYOUT_SURROUND,
    AV_CH_LAYOUT_2_1,
    AV_CH_LAYOUT_4POINT0,
    AV_CH_LAYOUT_2_2,
    AV_CH_LAYOUT_5POINT0
};
static const uint64_t eac3CustomChanMapLocations[16][2] = {
    { 1, AV_CH_FRONT_LEFT },
    { 1, AV_CH_FRONT_CENTER },
    { 1, AV_CH_FRONT_RIGHT },
    { 1, AV_CH_SIDE_LEFT },
    { 1, AV_CH_SIDE_RIGHT },
    { 0, AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER },
    { 0, AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT },
    { 0, AV_CH_BACK_CENTER },
    { 0, AV_CH_TOP_CENTER },
    { 0, AV_CH_SURROUND_DIRECT_LEFT | AV_CH_SURROUND_DIRECT_RIGHT },
    { 0, AV_CH_WIDE_LEFT | AV_CH_WIDE_RIGHT },
    { 0, AV_CH_TOP_FRONT_LEFT | AV_CH_TOP_FRONT_RIGHT},
    { 0, AV_CH_TOP_FRONT_CENTER },
    { 0, AV_CH_TOP_BACK_LEFT | AV_CH_TOP_BACK_RIGHT },
    { 0, AV_CH_LOW_FREQUENCY_2 },
    { 1, AV_CH_LOW_FREQUENCY }
};

/**
    \fn ADM_EAC3GetInfo
    \brief Find AC3 syncword and parse the header. If EAC3 is detected, continue
           until the header of the next independent frame or the end of buffer
           is reached, whichever first.
*/
bool ADM_EAC3GetInfo(const uint8_t *data, uint32_t len, uint32_t *syncoff, ADM_EAC3_INFO *info, bool *plainAC3)
{
    if(len < ADM_AC3_HEADER_SIZE)
        return false;

    uint8_t bsid=0;
    uint8_t frameType=3; // reserved
    uint8_t sRateCode=0;
    uint8_t acmod=0;
    uint8_t lfe=0;
    uint8_t shift=0;

    int blk = 6;

    uint32_t of=0;
    uint64_t channelLayout=0;
    bool syncFound = false;
    *syncoff=0;

    uint8_t *buf = new uint8_t[len + AV_INPUT_BUFFER_PADDING_SIZE];
    memcpy(buf,data,len);
    memset(buf+len,0,AV_INPUT_BUFFER_PADDING_SIZE);
    memset(info,0,sizeof(ADM_EAC3_INFO));
    //	printf("\n Syncing on %d \n",len);
    // Search for startcode
    // 0x0b 0x77
    while(1)
    {
        if(len < ADM_AC3_HEADER_SIZE)
        {
            if(len)
                ADM_warning("Not enough data left, stopping search for a52 syncword\n");
            goto _cleanup;
        }
        if(*(buf+of)!=0x0b || *(buf+of+1)!=0x77)
        {
            if(syncFound)
            {
                /* There are two possibilities: either we've got a valid frame followed
                 * by garbage or the frame is damaged and the size is bogus. As we don't
                 * check CRC, we cannot tell one from the other. */
                goto _cleanup;
            }
            len--;
            of++;
            continue;
        }
        // The code below is stolen from libavcodec's ac3_parser.c and eac3dec.c
        getBits bits(len,buf+of);
        bits.skip(16); // syncword
        bsid = bits.show(29) & 0x1f;
        if(bsid > 16)
        {
            ADM_warning("Invalid bitstream id\n");
            len--;
            of++;
            continue;
        }
        if(bsid <= 10) // plain AC-3
        {
            bits.skip(16); // CRC
            sRateCode = bits.get(2);
            if(sRateCode == 3)
            {
                ADM_warning("Invalid sampling rate code\n");
                len--;
                of++;
                continue;
            }
            int frameSizeCode = bits.get(6);
            if(frameSizeCode > 37)
            {
                ADM_warning("Invalid frame size code\n");
                len--;
                of++;
                continue;
            }
            bits.skip(8); // bitstream ID we already have and bitstream mode we don't need
            acmod = bits.get(3);
            if(acmod == 2 /* A52_STEREO */)
            {
                bits.skip(2); // surround mode
            }else
            {
                if((acmod & 1) && acmod != 1 /* A52_MONO */)
                    bits.skip(2); // center mix levels
                if(acmod & 4)
                    bits.skip(2); // surround mix levels
            }
            lfe = bits.get(1);
            shift = (bsid > 8)? bsid - 8 : 0;

            info->frequency = freqsTable[sRateCode] >> shift;
            info->byterate = (ratesTable[frameSizeCode >> 1] * 1000) >> (shift + 3);
            info->channels = channelTable[acmod] + lfe;
            info->frameSizeInBytes = frameSizeTable[frameSizeCode][sRateCode] * 2;
            info->samples = blk * 256;

            *syncoff = of;
            syncFound = true;
            if(plainAC3)
                *plainAC3 = true;
            goto _cleanup;
        }else // E-AC3
        {
            frameType = bits.get(2);
            if(frameType == 3)
            {
                ADM_warning("Invalid EAC3 frame type\n");
                if(syncFound)
                    goto _cleanup;
                len--;
                of++;
                continue;
            }
            if(syncFound && frameType != 1) // the next independent frame
            {
                info->flags |= ADM_EAC3_FLAG_PKT_COMPLETE;
                goto _cleanup;
            }
            if(!syncFound && frameType == 1) // dependent frame without the independent one
            {
                ADM_warning("EAC3 dependent frame found, but independent one missing\n");
                len--;
                of++;
                continue;
            }
            int substreamId = bits.get(3);
            if(substreamId)
            {
                ADM_warning("Non-zero EAC3 substream ID %d\n",substreamId);
                if(syncFound)
                    goto _cleanup;
                len--;
                of++;
                continue;
            }
            int frameSize = (bits.get(11) + 1) * 2; // 16 bits
            if(frameSize < ADM_AC3_HEADER_SIZE)
            {
                ADM_warning("Invalid frame size\n");
                if(syncFound)
                    goto _cleanup;
                len--;
                of++;
                continue;
            }
            sRateCode = bits.get(2);
            if(sRateCode == 3)
            {
                int srcode2 = bits.get(2);
                if(srcode2 == 3)
                {
                    ADM_warning("Invalid sampling rate code\n");
                    if(syncFound)
                        goto _cleanup;
                    len--;
                    of++;
                    continue;
                }
                info->frequency = freqsTable[srcode2];
                shift = 1;
            }else
            {
                int bcode = bits.get(2);
                blk = eac3NbBlocks[bcode];
                info->frequency = freqsTable[sRateCode];
                shift = 0;
            }
            acmod = bits.get(3);
            lfe = bits.get(1);

            bits.skip(5); // skip stuff including bitstream id
            for(int i=0; i < (acmod? 1 : 2); i++)
            {
                bits.skip(5); // dialog normalization
                if(bits.get(1)) // compression
                    bits.skip(8); // heavy dynamic range
            }
            if(frameType == 1 && bits.get(1)) // dependent frame channel map
            {
                int channelMap = bits.get(16);
                for(int i=0; i<16; i++)
                {
                    if(channelMap & (1 << (15 - i)))
                        channelLayout |= eac3CustomChanMapLocations[i][1];
                }
            }else
            {
                channelLayout = ac3ChannelLayoutTable[acmod];
            }

            if(lfe)
                channelLayout |= AV_CH_LOW_FREQUENCY;

            info->channels = av_get_channel_layout_nb_channels(channelLayout);
            info->frequency >>= shift;
            info->samples = blk * 256;
            info->byterate += frameSize * info->frequency / info->samples;

            if(!syncFound && !info->frameSizeInBytes && (frameType == 0 || frameType == 2))
            {
                *syncoff = of;
                syncFound = true;
                if(plainAC3)
                    *plainAC3 = false;
            }
            info->frameSizeInBytes += frameSize;

            if(len < frameSize + ADM_AC3_HEADER_SIZE)
                break;
            len -= frameSize;
            of += frameSize;
        }
    }
_cleanup:
    delete [] buf;
    buf=NULL;
    return syncFound;
}
