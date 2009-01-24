/***************************************************************************
                          aviaudio.cpp  -  description
                             -------------------
    begin                :
    copyright            : (C) 2004 by mean
    email                : fixounet@free.fr

Split a stream into packet(s)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"
#include "fourcc.h"
#include "aviaudio.hxx"

#include "ADM_a52info.h"
#include "ADM_mp3info.h"
#include "ADM_aacinfo.h"
#include "ADM_dcainfo.h"

#define MINSTOCK 5000
#define MINUS_ONE 0xffffffff

//
#if 0
uint8_t AVDMGenericAudioStream::flushPacket(void)
{
	packetTail=packetHead=0;
	_eos = 0;
	return 1;
}
uint8_t AVDMGenericAudioStream::getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
	uint32_t instock=0,rd=0;

	if (_eos)
		return 0;

	ADM_assert(_wavheader);

_refill:
	shrink();
	instock=packetTail-packetHead;

	while(instock<MINSTOCK)
	{
		rd=read(MINSTOCK,&packetBuffer[packetTail]);
		if(rd)
		{
			 instock+=rd;
			 packetTail+=rd;
		}
		else
		{
			printf("**PKTZ:READ ERROR\n");
			printf("**END OF AUDIO STREAM\n");
			_eos = 1;
			break;
		}
	}
	if(instock<4)
	{
		printf("Pkt : incoming buffer empty\n");
		return 0;
	}
	switch(_wavheader->encoding)
	{
                case WAV_DTS:
                                return getPacketDTS(dest,len,samples);
                                break;
		case WAV_MP2:
		case WAV_MP3:
                                if(! getPacketMP3(dest,len,samples)) goto _refill;
                                return 1;
				break;
		case WAV_AAC:
				return getPacketAAC(dest,len,samples);
				break;
		case WAV_LPCM:
		case WAV_PCM:
                case WAV_8BITS_UNSIGNED:
		case WAV_ULAW:
                case WAV_MSADPCM:
                case WAV_AMV_ADPCM:
				return getPacketPCM(dest,len,samples);
				break;
		case WAV_AC3:
				return getPacketAC3(dest,len,samples);
				break;

		case WAV_WMA:
				return getPacketWMA(dest,len,samples);
				break;
		default:
				printf("*** PACKETIZER : Unsupported!\n");
				return 0;
	}
	return 0;

}
uint8_t AVDMGenericAudioStream::shrink( void )
{
	ADM_assert(packetTail>=packetHead);
	if(packetTail>SIZE_INTERNAL)
	{
		// Wrap
		memmove(packetBuffer,&packetBuffer[packetHead],packetTail-packetHead);
		packetTail-=packetHead;
		packetHead=0;
	}

	return 1;
}
//___________________________
uint8_t		AVDMGenericAudioStream::getPacketWMA(uint8_t *dest, uint32_t *len,
								uint32_t *samples)
{
	uint32_t align=_wavheader->blockalign;
	uint32_t avail;


	avail=packetTail-packetHead;

	if(avail>=align)
	{
		//printf("WMA: %lu\n",align);
		memcpy(dest,&packetBuffer[packetHead],align);
		packetHead+=align;
		*samples=1024;
		#warning FIXME
		*len=align;
		return 1;
	}
	*len=0;
	*samples=0;
	printf("Packetizer wma: no more data\n");
	return 0;

}
/**
        \fn getPacketPCM
        \brief
*/
uint8_t		AVDMGenericAudioStream::getPacketPCM(uint8_t *dest, uint32_t *len,
								uint32_t *samples)
{
// Take ~ 32 samples packet
//
	uint32_t count,sample;
    uint32_t sampleSizeInBytes=1;
            if(_wavheader->encoding!=WAV_ULAW && _wavheader->encoding!=WAV_8BITS_UNSIGNED)
			{
				sampleSizeInBytes=2;
			}
			count=32*_wavheader->channels;
            count*=sampleSizeInBytes;

			if(packetTail-packetHead<count)
			{
				count=packetTail-packetHead;
				count&=~(sampleSizeInBytes*_wavheader->channels);
			}
			memcpy(dest,&packetBuffer[packetHead],count);
			packetHead+=count;

			// now revert to sample
			sample=count/_wavheader->channels;
			if(!sample)
			{
				packetHead=packetTail=0;
				printf("Wav Packetizer: running empty, last packet sent\n");
				return 0;
			}
			*samples=sample/sampleSizeInBytes;
			*len=count;
			return 1;
}

uint8_t		AVDMGenericAudioStream::getPacketAC3(uint8_t *dest, uint32_t *len,
								uint32_t *samples)
{
	uint32_t instock,rd;
	uint32_t startOffset,endOffset;
	uint8_t  lock=0;
	uint8_t  headerfound=0;

			ADM_assert(_wavheader->encoding==WAV_AC3);
			ADM_assert(packetTail>=packetHead);
			if(packetTail<packetHead+6)
			{
				printf("PKTZ:AC3 Buffer empty\n");
				return 0;
			}
			// It is MP3, read packet
			// synchro start
			int flags,sample_rate,bit_rate;
			int found=0;
			uint32_t start,size=0;

			start=packetHead;

			while(start+6<packetTail)
			{
				if(packetBuffer[start]!=0x0b || packetBuffer[start+1]!=0x77)
				{
					if(!lock)
					{
						printf("AC3: searching sync ");
						lock=1;
					}
					else
					{
						printf(".");
					}
					start++;
					continue;
				}


				size= ADM_a52_syncinfo (&packetBuffer[start], &flags,
						&sample_rate, &bit_rate);
				if(!size)
				{
					printf("AC3: Cannot sync\n");
					start++;
					continue;
				}
				if(size+packetHead<=packetTail)
				{
					// Found
					memcpy(dest,&packetBuffer[start],size);
					packetHead=start+size;
					found=1;
					break;
				}
				else
				{	// not enought data left
					headerfound=1;
					printf("AC3Pkt: Need %lu have:%lu\n",size,packetTail-packetHead);
					break;
				}

			}
			if(!found)
			{
				printf("AC3pak: Cannot find packet (%lu)\n",packetTail-packetHead);

				if(!headerfound)
				{

					// no header found, we can skip up to the 6 last bytes
					uint32_t left;
					left=packetTail-packetHead;
					if(left>6) left=6;
					packetHead=packetTail-left;

					printf("AC3Pak: No ac3 header found, purging buffer (%lu - %lu)\n",packetHead,packetTail);
				}
				return 0;
			}
			*len=size;
			*samples=1536;


			return 1;
}
uint8_t		AVDMGenericAudioStream::getPacketDTS(uint8_t *dest, uint32_t *len,uint32_t *samples)
{
uint32_t instock,rd;
uint32_t startOffset,endOffset;
uint8_t  lock=0;
uint8_t  headerfound=0;

                ADM_assert(_wavheader->encoding==WAV_DTS);
                ADM_assert(packetTail>=packetHead);
                if(packetTail<packetHead+DTS_HEADER_SIZE)
                {
                        printf("PKTZ:DTS Buffer empty\n");
                        return 0;
                }


                uint32_t flags,sample_rate,bit_rate,syncoff,chan,nbs;
                int found=0;
                uint32_t start,size=0;

                start=packetHead;

                while(start+DTS_HEADER_SIZE<packetTail)
                {
                        if(packetBuffer[start]!=0x07f || packetBuffer[start+1]!=0xfe||
                            packetBuffer[start+2]!=0x80|| packetBuffer[start+3]!=0x01)
                        {
                            if(!lock)
                            {
                                    printf("DTS: searching sync ");
                                    lock=1;
                            }
                            else
                            {
                                    printf(".");
                            }
                        start++;
                        continue;
                        }
//int ADM_DCAGetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *br, uint32_t *chan,uint32_t *syncoff,uint32_t *flags);
                    //    size= ADM_DCAGetInfo (&packetBuffer[start],packetTail-start,  &sample_rate,&bit_rate, &chan,&syncoff,&flags,&nbs);
                        if(!size)
                        {
                                printf("DTS: Cannot sync\n");
                                start++;
                                continue;
                        }
                        if(size+packetHead<=packetTail)
                        {
                                // Found
                                memcpy(dest,&packetBuffer[start],size);
                                packetHead=start+size;
                                found=1;
                                break;
                        }
                        else
                        {	// not enought data left
                                headerfound=1;
                                printf("DtsPkt: Need %lu have:%lu\n",size,packetTail-packetHead);
                                break;
                        }
                }
                if(!found)
                {
                        printf("DTSpak: Cannot find packet (%lu)\n",packetTail-packetHead);
                        if(!headerfound)
                        {
                            // no header found, we can skip up to the 6 last bytes
                            uint32_t left;
                                left=packetTail-packetHead;
                                if(left>DTS_HEADER_SIZE-1) left=DTS_HEADER_SIZE-1;
                                packetHead=packetTail-left;
                                printf("DtsPak: No dts header found, purging buffer (%lu - %lu)\n",packetHead,packetTail);
                        }
                        return 0;
                }
                *len=size;
                *samples=nbs;
                return 1;
}
/*

	Extract a packet from MP3 stream
	We only sync with mpeg audio startcode and frequency ATM
	We should/will compute the min packet size also
		to avoid as much as possible false detection

*/
uint8_t		AVDMGenericAudioStream::getPacketMP3(uint8_t *dest, uint32_t *len,
								uint32_t *samples)
{
	uint32_t instock,rd;
	uint32_t startOffset,endOffset;
	uint32_t layer;
	uint32_t size;
	MpegAudioInfo mpegInfo,mpegTemp;

			ADM_assert(_wavheader->encoding==WAV_MP2||_wavheader->encoding==WAV_MP3);
_retry:
			if(packetTail<packetHead+4)
			{
				printf("PKTZ: MP3Buffer empty:%lu / %lu\n",packetHead,packetTail);
				return 0;
			}
			// Build template, only fq ATM
			memset(&mpegTemp,0,sizeof(mpegTemp));
			mpegTemp.samplerate=_wavheader->frequency;

			// It is MP3, read packet
			// synchro start

			if(!getMpegFrameInfo(&(packetBuffer[packetHead]),packetTail-packetHead,
						&mpegInfo,&mpegTemp,&startOffset))
			{
				// Error decoding mpeg
				printf("MPInfo:** CANNOT FIND MPEG START CODE**\n");
                                if(packetTail>packetHead+4) // Flush
                                    packetHead=packetTail-4;
                                else
				    packetHead+=1;
                                return 0;
			}
			if(packetHead+startOffset+mpegInfo.size>packetTail)
			{
				printf("MP3 packetizer: not enough data (start %lu needs %lu, available %lu)\n"
						,startOffset,mpegInfo.size,packetTail-packetHead);
				packetHead+=1;
				return 0;
			}
			memcpy(dest,&packetBuffer[packetHead+startOffset],
					mpegInfo.size);
			*len=mpegInfo.size;
			*samples=mpegInfo.samples;
			//packetHead+=startOffset+mpegInfo.size;
                        // Patch from yhjvgskxwizgr (forum)
                       if(startOffset < mpegInfo.size/2)  // we probably have our correct frame, so set head to the next
                               packetHead+=startOffset+mpegInfo.size;
                       else if(startOffset < mpegInfo.size)  // we probably have the next frame, so duplicate it to keep a/v synced
                               packetHead+=startOffset;
                       else  // we have next or later frame, so next time continue searching one frame later to keep a/v synced
                               packetHead+=mpegInfo.size;
                        // /Patch
			return 1;
}
//---
uint8_t		AVDMGenericAudioStream::getPacketAAC(uint8_t *dest, uint32_t *len,
								uint32_t *samples)
{
	uint32_t instock,rd;
	uint32_t startOffset,endOffset;
	AacAudioInfo mpegInfo,mpegTemp;


			ADM_assert(_wavheader->encoding==WAV_AAC);
			if(packetTail<packetHead+8)
			{
				printf("PKTZ:Buffer empty\n");
				return 0;
			}
			if(!getAACFrameInfo(&(packetBuffer[packetHead]),packetTail-packetHead,
						&mpegInfo,&mpegTemp,&startOffset))
			{
				// Error decoding mpeg
				printf("AACInfo:** CANNOT FIND AAC/ADTS START CODE**\n");
				packetHead+=1;
				return 0;
			}
			if(packetHead+startOffset+mpegInfo.size>packetTail)
			{
				printf("AAC packetizer: not enough data (need %u, got %u)\n",mpegInfo.size,packetTail-packetHead-startOffset);
				return 0;
			}
			memcpy(dest,&packetBuffer[packetHead+startOffset],
					mpegInfo.size);
			*len=mpegInfo.size;
			*samples=mpegInfo.samples;
			packetHead+=startOffset+mpegInfo.size;

			return 1;
}
#endif
//
