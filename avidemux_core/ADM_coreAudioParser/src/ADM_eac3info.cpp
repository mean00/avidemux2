/**

        \file ADM_eac3info
        \brief extract info from EAC3/A52B/DD+ streams
        Author: mean <fixounet@free.fr>, (C) 2009
        Code very derived from ffmpeg (tables etc...)

*/

#include "ADM_default.h"
#include "ADM_eac3info.h"

extern "C"
{
#define sign_extend
#include "libavcodec/internal.h"
#include "libavcodec/ac3_parser.h"

};

/**
    \fn ADM_EAC3GetInfo
*/
bool     ADM_EAC3GetInfo(uint8_t *buf, uint32_t len, uint32_t *syncoff,ADM_EAC3_INFO *info)
{
uint32_t l;
int ibr,ifq,flags;
uint32_t of=0;

	*syncoff=of=0;
 //    	printf("\n Syncing on %d \n",len);
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
            AC3HeaderInfo hdr;
            GetBitContext gb;
            init_get_bits(&gb,buf,len*8);
            if(ff_ac3_parse_header(&gb, &hdr))
            {
                len--;
                buf++;
                of++;
                printf("Sync failed..continuing\n");
                continue;
            }
//            printf("Sync found at offset %"LU"\n",of);
            *syncoff=of;
            info->frequency=(uint32_t)hdr.sample_rate;
            info->byterate=(uint32_t)hdr.bit_rate>>3;
            info->channels=hdr.channels;
            info->frameSizeInBytes=hdr.frame_size;
            info->samples=265*6; // ??
            return 1;
		}
		return 1;
	
}
