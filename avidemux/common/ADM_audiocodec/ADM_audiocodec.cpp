/***************************************************************************
     \file                     ADM_audiocodec.cpp  
     \brief   Front end for audio decoder
    copyright            : (C) 2002/2009 by mean fixounet@free.fr
 **************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"

#include "fourcc.h"
#include "ADM_coreAudio.h"
#include "ADM_audiocodec/ADM_audiocodec.h"
extern ADM_Audiocodec *ADM_ad_searchCodec(uint32_t fourcc,	WAVHeader *info,uint32_t extraLength,uint8_t *extraData);

ADM_Audiocodec	*getAudioCodec(uint32_t fourcc,WAVHeader *info,uint32_t extra,uint8_t *extraData)
{
ADM_Audiocodec *out = NULL;

		// fake codec for 8 bits
		if(fourcc==WAV_PCM)
			{
				if(info)
					if(info->bitspersample==8)
                        {
                            info->encoding=fourcc=WAV_8BITS_UNSIGNED;
                        }

			}

 		switch(fourcc)
   			{

				case WAV_PCM:
                    printf("[audioCodec] Audio codec:  WAV\n");
#ifdef ADM_BIG_ENDIAN
                    out= (ADM_Audiocodec *)new ADM_AudiocodecWavSwapped(fourcc,*info);
#else
					out= (ADM_Audiocodec *)new ADM_AudiocodecWav(fourcc,*info);
#endif
                  			break;
				case WAV_8BITS:
					printf("[audioCodec] 8 BIts pseudo codec\n");
                    out= (ADM_Audiocodec *)new ADM_Audiocodec8Bits(fourcc,*info);
					break;
				case WAV_8BITS_UNSIGNED:
					printf("[audioCodec] 8 BIts pseudo codec unsigned\n");
    					out= (ADM_Audiocodec *)new ADM_Audiocodec8Bits(fourcc,*info);
					break;
		  		case WAV_LPCM:
					printf("[audioCodec] Audio codec:  LPCM swapped\n");
#ifndef ADM_BIG_ENDIAN
                    out= (ADM_Audiocodec *)new ADM_AudiocodecWavSwapped(fourcc,*info);
#else
					out= (ADM_Audiocodec *)new ADM_AudiocodecWav(fourcc,*info);
#endif
                  		break;
            default:
            	out= ADM_ad_searchCodec(fourcc,info,extra,extraData);
            	break;
        	}

	if (out == NULL)
	{
		printf("[audioCodec] Unknown codec : %"PRIu32"\n",fourcc);
		out = (ADM_Audiocodec *) new ADM_AudiocodecUnknown(fourcc,*info);
	}
	// For channel mapping, simple case we do it here so that the decoder does not have
	// to worry.
	// For more complicated case (channel >2) , it is up to the decoder to do it...
	switch(info->channels)
	{
			case 1: out->channelMapping[0] = ADM_CH_MONO;
					break;
			case 2: out->channelMapping[0] = ADM_CH_FRONT_LEFT;
					out->channelMapping[1] = ADM_CH_FRONT_RIGHT;
					break;
			default:break;


	}
	return out;
}
