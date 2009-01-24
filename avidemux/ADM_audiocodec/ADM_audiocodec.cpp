/***************************************************************************
                          ADM_audiocodec.cpp  -  description
                             -------------------
    begin                : Sat Jun 1 2002
    copyright            : (C) 2002 by mean
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "config.h"
#include "ADM_assert.h"

#include "fourcc.h"
#include "ADM_audio/aviaudio.hxx"
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
                                case WAV_NELLYMOSER:
                                        printf("\n Audio codec:  NELLYMOSER\n");
                                        out= (ADM_Audiocodec *)new ADM_AudiocodecWMA(fourcc,info,extra,extraData);
                                        break;
                                case WAV_IMAADPCM:
                                        printf("\n Audio codec:  IMA MS ADPCM\n");
                                        out= (ADM_Audiocodec *)new ADM_AudiocodecImaAdpcm(fourcc,info);
                                        break;
                                case WAV_MSADPCM:
                                        printf("\n Audio codec:   MS ADPCM\n");
                                        out= (ADM_Audiocodec *)new ADM_AudiocodecMsAdpcm(fourcc,info);
                                        break;
				case WAV_PCM:
    					printf("\n Audio codec:  WAV\n");
#ifdef ADM_BIG_ENDIAN
    					out= (ADM_Audiocodec *)new ADM_AudiocodecWavSwapped(fourcc);
#else
					out= (ADM_Audiocodec *)new ADM_AudiocodecWav(fourcc);
#endif
                  			break;
				case WAV_8BITS:
					printf("\n 8 BIts pseudo codec\n");
    					out= (ADM_Audiocodec *)new ADM_Audiocodec8Bits(fourcc);
					break;
				case WAV_8BITS_UNSIGNED:
					printf("\n 8 BIts pseudo codec unsigned\n");
    					out= (ADM_Audiocodec *)new ADM_Audiocodec8Bits(fourcc);
					break;
		  		case WAV_LPCM:
					printf("\n Audio codec:  LPCM swapped\n");
#ifndef ADM_BIG_ENDIAN
    					out= (ADM_Audiocodec *)new ADM_AudiocodecWavSwapped(fourcc);
#else
					out= (ADM_Audiocodec *)new ADM_AudiocodecWav(fourcc);
#endif
                  		break;
#ifdef USE_LIBDCA
                case WAV_DTS:
					if (dca->isAvailable())
					{
						printf("\n Audio codec:  DTS\n");
						out= (ADM_Audiocodec *) new ADM_AudiocodecDCA(fourcc, info);
					}

					break;
#endif
				case WAV_ULAW:
						printf("\n ULAW codec\n");
						out=(ADM_Audiocodec *) new ADM_AudiocodecUlaw(fourcc,info);
						break;
            case WAV_AMV_ADPCM:
                printf("\n Audio codec:  ffAMV\n");
                out= (ADM_Audiocodec *) new ADM_AudiocodecWMA(fourcc,info,extra,extraData);
                break;

            case WAV_WMA:
                printf("\n Audio codec:  ffWMA\n");
                out= (ADM_Audiocodec *) new ADM_AudiocodecWMA(fourcc,info,extra,extraData);
                break;
            case WAV_QDM2:
                printf("\n Audio codec:  ffQDM2\n");
                out= (ADM_Audiocodec *) new ADM_AudiocodecWMA(fourcc,info,extra,extraData);
                break;
            default:
            	out= ADM_ad_searchCodec(fourcc,info,extra,extraData);
        	}

	if (out == NULL)
	{
		printf("\n Unknown codec : %"LU,fourcc);
		out = (ADM_Audiocodec *) new ADM_AudiocodecUnknown(fourcc);
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
