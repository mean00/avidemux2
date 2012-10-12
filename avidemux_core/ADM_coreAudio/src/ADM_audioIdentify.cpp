/**
    \file  ADM_audioIdentify
    \brief Identify a codec used in a file

    \author copyright            : (C) 2012 by mean
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_audiodef.h"
#include "ADM_mp3info.h"
#include "ADM_a52info.h"
#include "ADM_audioCodecEnum.h"
#include "ADM_audioIdentify.h"
#include "fourcc.h"

extern void Endian_WavHeader(WAVHeader *w);
#define INVALID_OFFSET 0XFFFFFFF
/**
    \fn idMP2
    \brief return true if the tracks is mp2
*/
static bool idMP2(int bufferSize,const uint8_t *data,WAVHeader &info,uint32_t &offset)
{
        const uint8_t *mp2Buffer=data;
        const int limit=bufferSize;
        offset=INVALID_OFFSET;
        // Now read buffer until we have 3 correctly decoded packet
        int probeIndex=0;
        int failAttempt=0;
        while(probeIndex<limit)
        {

            const uint8_t *ptr=mp2Buffer+probeIndex;
            int     len=limit-probeIndex;
            if(len<4)
            {
                    ADM_info("\t no sync(3)\n");
                    return false;
            }
            uint32_t syncoff,syncoff2;
            MpegAudioInfo mp2info,confirm;
            if( !getMpegFrameInfo(ptr,len,&mp2info,NULL,&syncoff))
            {
                    ADM_info("\t no sync\n");
                    return false;
            }
            if(INVALID_OFFSET==offset) offset=syncoff;
          // Skip this packet
            int next=probeIndex+syncoff+mp2info.size;
            len=limit-next;
            if(len<4)
            {
                    ADM_info("\t no sync(2)\n");
                    return false;
            }
            if(getMpegFrameInfo(mp2Buffer+next,len,&confirm,&mp2info,&syncoff2))
            {
                    if(!syncoff2)
                    {
                            ADM_warning("\tProbably MP2/3 : Fq=%d br=%d chan=%d\n", (int)mp2info.samplerate,
                                                                (int)mp2info.bitrate,
                                                                (int)mp2info.mode);
                            // fill in info
                            info.frequency=mp2info.samplerate;
                            info.byterate=(mp2info.bitrate>>3)*1000;
                            if(mp2info.layer==1) info.encoding=WAV_MP2;
                                else             info.encoding=WAV_MP3;
                            switch(mp2info.mode)
                            {
                                 case 1: // Joint stereo
                                 case 0: // Stereo
                                 case 2: // dual channel
                                 default:
                                            info.channels=2;
                                            break;
                                 case 3: // mono
                                            info.channels=1;
                                            break;
                            }
                            return true;
                    }
                    failAttempt++;
                    if(failAttempt>10) return false;
            }
            probeIndex+=syncoff+1;
        }
        return false;
}
/**
 * \fn idWAV
 * \brief returns true if the file is a riff/wav file
 */
#define wRead32(x) {x=( cur[0]+(cur[1]<<8)+(cur[2]<<16)+(cur[3]<<24));cur+=4;ADM_assert(cur<=tail);}
#define wRead16(x) {x=( cur[0]+(cur[1]<<8));cur+=2;ADM_assert(cur<=tail);}
static bool idWAV(int bufferSize,const uint8_t *data,WAVHeader &info,uint32_t &offset)
{
		const uint8_t *cur=data,*tail=data+bufferSize;
	 	uint32_t t32;
	 	uint32_t totalSize;
        offset=0;
	 	wRead32(t32);
        ADM_info("Checking if it is riff/wav...\n");
	    if (!fourCC::check( t32, (uint8_t *) "RIFF"))
	      {
		  ADM_warning("Not riff.\n");
		  fourCC::print(t32);
		  goto drop;
	      }
	    wRead32(totalSize);
	    ADM_info("\n %lu bytes total \n", totalSize);

	    wRead32(t32);
	    if (!fourCC::check( t32, (uint8_t *) "WAVE"))
	      {
		  ADM_warning("\n no wave chunk..aborting..\n");
		  goto drop;
	      }
	    wRead32(t32);
	    if (!fourCC::check( t32, (uint8_t *) "fmt "))
	      {
	    	ADM_warning("\n no fmt chunk..aborting..\n");
		  goto drop;
	      }
	    wRead32(t32);
	    if (t32 < sizeof(WAVHeader))
	      {
	    	ADM_warning("\n incorrect fmt chunk..(%ld/%d)\n",t32,sizeof(WAVHeader));
		  goto drop;
	      }
	    // read wavheader
	    memcpy(&info,cur,sizeof(WAVHeader));
	    cur+=t32;
        if(t32>sizeof(WAVHeader))
        {
            ADM_warning("There are some extradata!\n");
        }
	    ADM_assert(cur<tail);

	    // For our big endian friends
	    Endian_WavHeader(&info);
	    //
	    wRead32(t32);
	    if (!fourCC::check( t32, (uint8_t *) "data"))
	      {
	          // Maybe other chunk, skip at most one
	    	  wRead32(t32);
	    	  cur+=t32;
	    	  ADM_assert(cur+4<tail);
	    	  wRead32(t32);
	          if (!fourCC::check( t32, (uint8_t *) "data"))
	          {
	        	  ADM_warning("\n no data chunk..aborting..\n");
		       goto drop;
	          }
	       }

	    uint32_t length;
	    wRead32(t32);
	    ADM_info(" %lu bytes data \n", totalSize);
	    info.blockalign=1;
        offset=(uint32_t)(cur-data);
        ADM_info("yes, it is riff/wav, data starts at %d...\n",(int)offset);
	    return true;
	  drop:
        ADM_info("No, not riff/wav...\n");
	    return false;
}
/**
    \fn idAC3
    \brief return true if the tracks is mp2
*/
static bool idAC3(int bufferSize,const uint8_t *data,WAVHeader &info,uint32_t &offset)
{
    uint32_t fq,br,chan,syncoff;
    uint32_t fq2,br2,chan2,syncoff2;

   if( !ADM_AC3GetInfo(data,bufferSize, &fq, &br, &chan,&syncoff))
    {
            ADM_info("Not ac3\n");
            return false;
    }
    offset=syncoff;
    // Need a 2nd packet...
    const uint8_t *second=data+syncoff;
    int size2=bufferSize-syncoff;
    ADM_assert(size2>0);
    ADM_info("Maybe AC3... \n");
    // Try on 2nd packet...
    if( ADM_AC3GetInfo(second,size2, &fq2, &br2, &chan2,&syncoff2))
    {
        if((fq==fq2) && (br2==br) && (chan==chan2))
        {
            ADM_warning("\tProbably AC3 : Fq=%d br=%d chan=%d\n",(int)fq,(int)br,(int)chan);
            info.encoding=WAV_AC3;
            info.channels=chan;
            info.byterate=(br>>3)*1000;
            info.frequency=fq;
            return true;
        }
    }
    ADM_info("Cannot confirm ac3\n");
    return false;
}
/**
    \fn ADM_identifyAudioStream
    \param bufferSize : nb of bytes available for scanning
    \param buffer     : buffer containing the data to scan
    \param info       : wavheader to fill with detected codec/channels/etcc
    \param offset     : offset in the file where payload starts
*/
bool ADM_identifyAudioStream(int bufferSize,const uint8_t *buffer,WAVHeader &info,uint32_t &offset)
{
    memset(&info,0,sizeof(info));
    if(idWAV(bufferSize,buffer,info,offset)) return true;
    if(idMP2(bufferSize,buffer,info,offset)) return true;
    if(idAC3(bufferSize,buffer,info,offset)) return true;

    return false;
}
// EOF
