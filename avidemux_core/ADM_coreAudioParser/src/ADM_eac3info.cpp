/**

        \file ADM_eac3info
        \brief extract info from EAC3/A52B/DD+ streams
        Author: mean <fixounet@free.fr>, (C) 2009
        Code very derived from ffmpeg (tables etc...)

*/

#include "ADM_default.h"

#include "ADM_eac3info.h"
/**
 * Possible frame sizes.
 * from ATSC A/52 Table 5.18 Frame Size Code Table.
 * Borrowed from ffmpeg
 */
const uint16_t ff_ac3_frame_size_tab[38][3] = {
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
    { 1280, 1394, 1920 },
};
// Borrowed from ffmpeg
const uint16_t ff_ac3_bitrate_tab[19] = {
    32, 40, 48, 56, 64, 80, 96, 112, 128,
    160, 192, 224, 256, 320, 384, 448, 512, 576, 640
};

/**
    \fn ADM_a52b_syncinfo
*/
bool ADM_a52b_syncinfo (uint8_t * buf, int * flags, int * sample_rate, int * bit_rate)
{
    int frmsizecod;
    int bitrate;
    int half;
    int acmod;

    if ((buf[0] != 0x0b) || (buf[1] != 0x77))	/* syncword */
        return false;

// 0B77   16 01
// CRC16  16 23
// fscod  2  4
// frmsiz 6  4

    int fscod=buf[0]>>6;
    int frmsize=buf[0]&0x3F;
    if(frmsize>=38) return 0;

    if(fscod==3) return 0;
    switch(fscod)
    {
        case 0: *sample_rate=48000;break;
        case 1: *sample_rate=44100;break;
        case 2: *sample_rate=32000;break;
        default : return 0;
    }
    *bit_rate=ff_ac3_frame_size_tab[frmsize][fscod]*1000;
    return 1;
    
}
/**
    \fn ADM_EAC3GetInfo
*/
bool  ADM_EAC3GetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *byterate, uint32_t *chan,uint32_t *syncoff)
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
	        l=ADM_a52b_syncinfo (buf,&flags, &ifq, &ibr);
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
		*byterate=(uint32_t)ibr>>3;

		return 1;
	}
	return 0;
}
