//
// C++ Implementation: ADM_a52info
//
// Description:
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "ADM_default.h"

#include "ADM_a52info.h"

#define A52_CHANNEL 0
#define A52_MONO 1
#define A52_STEREO 2
#define A52_3F 3
#define A52_2F1R 4
#define A52_3F1R 5
#define A52_2F2R 6
#define A52_3F2R 7
#define A52_CHANNEL1 8
#define A52_CHANNEL2 9
#define A52_DOLBY 10
#define A52_CHANNEL_MASK 15

#define A52_LFE 16
#define A52_ADJUST_LEVEL 32

static uint8_t halfrate[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3};

static int rate[] = { 32,  40,  48,  56,  64,  80,  96, 112,
			 128, 160, 192, 224, 256, 320, 384, 448,
			 512, 576, 640};
static uint8_t lfeon[8] = {0x10, 0x10, 0x04, 0x04, 0x04, 0x01, 0x04, 0x01};

// Borrowed from a52dec
// Return packed size on success, 0 on failure
// Need at least 6 bytes incoming

int ADM_a52_syncinfo (uint8_t * buf, int * flags, int * sample_rate, int * bit_rate)
{
    int frmsizecod;
    int bitrate;
    int half;
    int acmod;

    if ((buf[0] != 0x0b) || (buf[1] != 0x77))	/* syncword */
	return 0;

    if (buf[5] >= 0x60)		/* bsid >= 12 */
	return 0;
    half = halfrate[buf[5] >> 3];

    /* acmod, dsurmod and lfeon */
    acmod = buf[6] >> 5;
    *flags = ((((buf[6] & 0xf8) == 0x50) ? A52_DOLBY : acmod) |
	      ((buf[6] & lfeon[acmod]) ? A52_LFE : 0));

    frmsizecod = buf[4] & 63;
    if (frmsizecod >= 38)
	return 0;
    bitrate = rate [frmsizecod >> 1];
    *bit_rate = (bitrate * 1000) >> half;

    switch (buf[4] & 0xc0) {
    case 0:
	*sample_rate = 48000 >> half;
	return 4 * bitrate;
    case 0x40:
	*sample_rate = 44100 >> half;
	return 2 * (320 * bitrate / 147 + (frmsizecod & 1));
    case 0x80:
	*sample_rate = 32000 >> half;
	return 6 * bitrate;
    default:
	return 0;
    }
}
//
//	Exctract infos from AC3 stream (used when muxing with external AC3)
//
uint8_t ADM_AC3GetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *br, uint32_t *chan,uint32_t *syncoff)
{
uint32_t l;
int ibr,ifq,flags;
uint32_t of=0;

	*syncoff=of=0;
     	printf("\n Syncing on %d \n",len);
	// Search for startcode
	// 0x0b 0x77
	while(1)
	{
		 if(len<7)
		 {
		 	printf("Not enough info to find a52 syncword\n");
		 	return 0;
		 }
		 if( *buf!=0x0b || *(buf+1)!=0x77)
		 {
		 	len--;
			buf++;
			of++;
			continue;
		 }
		 // Try to get syncinfo
	        l=ADM_a52_syncinfo (buf,&flags, &ifq, &ibr);
		if(!l)
		{
			len--;
			buf++;
			of++;
			printf("Sync failed..continuing\n");
			continue;
		}
		printf("Sync found at offset %"LU"\n",of);
		*syncoff=of;
		*fq=(uint32_t)ifq;
		*br=(uint32_t)ibr>>3;
		switch (flags & A52_CHANNEL_MASK) {
            case A52_CHANNEL:
			case A52_MONO:
				*chan = 1;
			break;
			case A52_STEREO:
			case A52_DOLBY:
				*chan = 2;
			break;
			case A52_3F:
			case A52_2F1R:
				*chan = 3;
			break;
			case A52_3F1R:
			case A52_2F2R:
				*chan = 4;
			break;
			case A52_3F2R:
				*chan = 5;
			break;
			default:
				ADM_assert(0);
		}
		if (flags & A52_LFE)
			(*chan)++;
		return 1;
	}
	return 0;
}
