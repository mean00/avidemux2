/***************************************************************************
                          audiogen.cpp  -  description

	Handle generic audio stream, whatever the source is.

                             -------------------
    begin                : Wed Dec 19 2001
    copyright            : (C) 2001 by mean
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
//#include <stream.h>
#include "ADM_assert.h"
#include <math.h>

#include "config.h"
#include "avifmt.h"
#include "avifmt2.h"
#include "fourcc.h"
#include "aviaudio.hxx"
//#include "ADM_outputs/oplug_avi/avilist.h"
#include "ADM_a52info.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_AUDIO
#include "ADM_osSupport/ADM_debug.h"

extern uint16_t MP1L2Bitrate[2][16];
extern uint16_t MP2L1Bitrate[2][16];
#if 0
AVDMGenericAudioStream::AVDMGenericAudioStream(void)
{

        _codec=NULL;
        _current=0;
        _audioMap=NULL;
        _wavheader=NULL;
        _file=NULL;
        _LAll=NULL;
        _mpegSync[0]=_mpegSync[1]=_mpegSync[2]=0;
        packetHead=packetTail=0;
        
}
AVDMGenericAudioStream::~AVDMGenericAudioStream()
{
	if(_codec)
	{
	delete _codec;
	_codec=NULL;
	}

	if (_audioMap)
	{
		delete[] _audioMap;
		_audioMap = NULL;
	}

}
uint8_t AVDMGenericAudioStream::isPaketizable( void)
{

	// packetizable stuff
	switch(_wavheader->encoding)
	{
	
		case WAV_MP3:		// Yes
		case WAV_MP2:
		case WAV_WMA:
		case WAV_PCM:

		case WAV_LPCM:
		case WAV_AC3:
		case WAV_8BITS:
		case WAV_8BITS_UNSIGNED:
		case WAV_ULAW:
                
			return 1;
			break;
		case WAV_MSADPCM:
                case WAV_IMAADPCM:
		case WAV_MP4:		// No
		case WAV_AAC:
		case WAV_OGG:
			return 0;
			break;
		default:		// in doubt : no
			return 0;
			break;
	}	

}
//________________________________________________________________
//      Go to a beginning of an audio frame after the offset given
//  Offset can be not frame bounded (MP3), correct that.
//_________________________________________________________________
uint8_t AVDMGenericAudioStream::goToSync(uint32_t offset)
{
    static uint8_t a,b, c;
    #define SYNCAC3 8192
    uint8_t	syncbuff[SYNCAC3];
    uint32_t suboffset = 0,byterate;
    uint32_t tryz;
    
    ADM_assert(_wavheader);

    // can't deal with something not mp3...
  //  printf("syncing asked : %lu",offset);
    switch(  _wavheader->encoding )
    {

    		case WAV_AC3:
					{
					  uint32_t fq,br,chan;
					  if(!goTo(offset)) return 0;
					  if(SYNCAC3!=read(SYNCAC3,syncbuff))
					  {
					  	printf("Cannot read enough bytes!\n");
						return 0;					  
					  }
					
					  if(!ADM_AC3GetInfo(syncbuff, SYNCAC3,&fq, &br, &chan,&suboffset))
					  {
					  	return 0;
					  }
					  if(!goTo(offset+suboffset)) return 0;
					  printf("A52 sync found at %lu + %lu\n",offset,suboffset);		
					  }
					  return 1;
					  break;
		case WAV_WMA:
					uint32_t wmaoffset;
					if(_wavheader->blockalign)
						wmaoffset=offset - (offset%_wavheader->blockalign);
					else 
						wmaoffset=offset;
		      			printf("... wma offset : %lu",wmaoffset);
				    	if(!goTo(wmaoffset)) return 0;
					return 1;
		case WAV_MP4:
		#warning FIXME NEED DOC
				suboffset=0;
				tryz=0;
				if(!goTo(offset)) return 0;
				while(tryz<1200)
				{
					if (!read(1, &a))
					{
						printf("MP4sync :Read failed\n");
	      					return 0;
					}
					if(a==0x21)
						{
							if (!read(1, &a))
	      							return 0;
							if(a==0x0a)
							{
								if(!goTo(offset+suboffset))
								{
									printf("MP4sync :Seek failed\n");
									return 0;
								}
								printf("MP4sync : sync %ld\n",suboffset);
								return 1;
							}
							suboffset++;
						}
					suboffset++;
					tryz++;
				}
				printf("MP4sync : cound not find sync\n");
				return 0;
						
      		case WAV_PCM:
      			uint32_t mask;

         		mask=0xffffffff;
           	switch( _wavheader->channels)
            	{
               		case 1:
                			mask=0xfffffffe; // just even
                   		break;
                  	case 2:
                  		mask=0xfffffffc;   // 1100
                    		break;
                   	default:
                   		mask=0xffffffff;
                     		printf("\n more than 2 channels on PCM ????\n");
                }
				return goTo(mask & offset);
    			break;
       case WAV_MP3:
       case WAV_MP2:
    //
    // in case of MP3, we do know how to search sync frame...
    if(!goTo(offset)) return 0;
    // search sync
    do
      {
_doitagain:      
	  if (!read(1, &a))
	      return 0;		//EOF
	  suboffset++;
	  if (a == 0xff)
	    {
rebranch2:
		if (!read(1, &c))
		    return 0;
		suboffset++;

		switch (c)
		  {
#define FALSE_ALARM
                    
#ifndef FALSE_ALARM
		  case 0xff:
		      goto rebranch2;
#endif
		  case 0xfa:
		  case 0xfb:
         	  case 0xfc:
         	  case 0xfd:
   		  case 0xf3:
                  case 0xf4:
	     	  case 0xf5:
#ifdef FALSE_ALARM
                    case 0xff:
#endif
		  	// For mpeg1/2 layer 1/2 (svcd/VCD/DVD)
			// We make some extra check to avoid cutting
			// on a falsely detected mpeg frame that would
			// make the muxer barfs
		  	if(_wavheader->encoding==WAV_MP2 && 1 && _mpegSync[0])
			{
				if (!read(1, &a))
			    		return 0;
				suboffset++;
				if(a==0xff) goto rebranch2;
			
    				 if (!read(1, &b))
			    		return 0;
				suboffset++;
				if(b==0xff) goto rebranch2;
                                a&=0xfd; // ignore padding bit
				if(c==_mpegSync[0]&& a==_mpegSync[1] && ((b&0xc0)==(_mpegSync[2]&0xC0)))
				{
					suboffset-=2;
					goto contact2;
				}
				// Failed
				printf("False header, continuing..(%x %x %x expected %x %x %x\n",c,a,b,
                                       _mpegSync[0],_mpegSync[1],_mpegSync[2]);
				continue;
         		}
		      goto contact2;
		  }
	    }
      }
    while (1);
  contact2:
    aprintf("\n audio suboffset :%lu", suboffset - 2);
     aprintf("\n total offset :%lu", offset +suboffset - 2);
    return goTo(offset + suboffset - 2);
    break;

    default:
    		return goTo(offset);
      		break;
      }
      ADM_assert(0);
      return 0;
}

//_____________________
//
// Clear decompressor
//______________________
uint8_t AVDMGenericAudioStream::endDecompress(void)
{

	ADM_assert(_codec);
 	return _codec->endDecompress();
}

//________________________________________________________________
//   Read but decompress first !
//   Return at least what's being asked
//   Call in case of sequential read only !
//________________________________________________________________
uint8_t AVDMGenericAudioStream::beginDecompress(void)
{
	ADM_assert(_codec);
 	return _codec->beginDecompress();
}

//
//
//__________________________________________
uint32_t AVDMGenericAudioStream::readDecompress(uint32_t size,
						uint8_t * ptr)//deprecate
{
    uint32_t rd = 0, d = 0, in = 0,samples=0;
    uint8_t r=0;
    // Paranoia check
    ADM_assert(_wavheader);
    ADM_assert(_codec);
    ADM_assert(isDecompressable());

  
        while(rd<size)
        {
           	// Read from stream
		r=getPacket(internalBuffer, &in, &samples);           		
		if(!r)
          	{
drop:
			printf(" read failed, end of stream ? \n");
			return rd;
               	}
//		if(!_codec->run(internalBuffer,in,ptr+rd,&d))
		{
			printf("\n Codec error !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			goto drop;
		}
		if(d)
			rd+=d;
      	}
	return rd;


}

uint32_t AVDMGenericAudioStream::readDecompress(uint32_t size, float *ptr)
{
	uint32_t rd = 0, d = 0, in = 0,samples=0;
	uint8_t r = 0;
	// Paranoia check
	ADM_assert(_wavheader);
	ADM_assert(_codec);
	ADM_assert(isDecompressable());

	while(rd<size)
	{
		// Read from stream
		r=getPacket(internalBuffer, &in, &samples);
		if (!r) {
drop:
			printf(" read failed, end of stream ? \n");
			return rd;
		}
		if (!_codec->run(internalBuffer,in,ptr+rd,&d))
		{
			printf("\n Codec error !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			goto drop;
		}
		if (d)
			rd+=d;
	}

	return rd;
}

//________________________________________________________________
//   Returns 1 if audio is compressed else 0
//________________________________________________________________
uint8_t AVDMGenericAudioStream::isCompressed(void)
{
    if (_wavheader)
      {
			ADM_assert(_codec);
 			return _codec->isCompressed();
      }
    return 1;
}

//________________________________________________________________
//   Instead of going to a byte offset, go to a time offset           Ooops
//________________________________________________________________
uint8_t AVDMGenericAudioStream::isDecompressable(void)
{
    if (_wavheader)
      {
			ADM_assert(_codec);
 			return !_codec->isDummy();
      }
    return 0;
}


//________________________________________________________________
//   Instead of going to a byte offset, go to a time offset
// Time offset is in ms
//________________________________________________________________
uint8_t AVDMGenericAudioStream::goToTime(uint32_t timeoffset)
{

    uint32_t offset;
    packetHead=packetHead=0;
    offset = convTime2Offset(timeoffset);
      aprintf("\n Time Offset : %lu", timeoffset);

   if( _audioMap)
   	{
      		// dichotomic search
        	uint32_t min,max,pivot;
           double doff;

         	doff=2*_wavheader->channels*_wavheader->frequency;
         	doff*=timeoffset;
          doff/=1000;
          offset=(uint32_t )floor(doff);
		    aprintf("\n Linear Offset : %lu", offset);

         	min=0;
          max=_nbMap-1;

          do
          {
            	pivot=(min+max)>>1;
             	if( _audioMap[pivot].woffset<offset) //min    offset pivot max
              		{
                         min=pivot;
                  	}
			else              	if( _audioMap[pivot].woffset>offset) //min    pivot offsetmax
                    {

                       		max=pivot;

                      }
                      else // found !
                      { min=pivot;max=min+1;}


            }while( (min+1)!=max);
            	if(_audioMap[max].woffset==offset) min=max;
//              goToSync( _audioMap[min].foffset); normally already synchronized
				  if(min) min--;
                goTo( _audioMap[min].foffset);
                _current=_audioMap[min].foffset;
              aprintf("\n asked : %lu , got : %lu, next :%lu,file:%lu\n",offset,
              				_audioMap[min].woffset,
              				_audioMap[min+1].woffset,
                  			_audioMap[min].foffset);
              return 1;

      }
    aprintf("\n Time-> linear Offset : %lu", offset);

    return goToSync(offset);
}

//_________________________________________________________
//      Convert a time value into offset value
//_________________________________________________________
uint32_t AVDMGenericAudioStream::convTime2Offset(uint32_t time)
{
    double one_frame_double;
    uint32_t offset;
    // convert time information into
    // byte offset
    ADM_assert(_wavheader);    
    one_frame_double = time;
    one_frame_double *= _wavheader->byterate;
    one_frame_double /= 1000.;

    offset = (uint32_t) floor(one_frame_double);
    return offset;

}

//_________________________________________________________
//      Write header if needed
//_________________________________________________________
uint8_t AVDMGenericAudioStream::writeHeader(FILE * out)
{
    WAVHeader wh, *last_wave;
    
    
    
    last_wave = getInfo();
    if (last_wave->encoding != WAV_PCM)    
                return 1;
                
        _file=new ADMFile();                        
        if(!_file->open(out)) return 0;
        _LAll = new AviList("RIFF", _file);
        _LAll->Begin("WAVE");
        memcpy(&wh, last_wave, sizeof(WAVHeader));
    // update Header
   
    wh.encoding = WAV_PCM;

    wh.blockalign = 2 * wh.channels;
    wh.bitspersample = 16;
    wh.byterate = last_wave->frequency * last_wave->channels * 2;
    // Do it for little endian
     Endian_WavHeader(&wh);
    //
    _LAll->WriteChunk((uint8_t *) "fmt ", sizeof(WAVHeader),
		      (uint8_t *) & wh);
    _LAll->Write32("data");
    // size to be completed later
    _dlen = _LAll->Tell();
    _LAll->Write32((uint32_t) 0L);
    return 1;
}

uint8_t AVDMGenericAudioStream::endWrite(FILE * out, uint32_t len)
{
    if (getInfo()->encoding != WAV_PCM)
	return 1;
    _LAll->End();
    fseek(out, _dlen, SEEK_SET);
    fwrite(&len, 4, 1, out);
    delete _LAll;
    _LAll = NULL;
    delete _file;
    _file=NULL;
    return 1;

}
#endif
/**
 * 	\fn ADM_audioCompareChannelMapping
 *  \brief return true if the two channel mapping are identical, false else.
 */
bool ADM_audioCompareChannelMapping(WAVHeader *wh1, WAVHeader *wh2,CHANNEL_TYPE *map1,CHANNEL_TYPE *map2)
{
	if(wh1->channels != wh2->channels) return false; // cannot be identical..
		
			for (int j = 0; j < wh1->channels; j++)
			{
				if (map1[j] != map2[j]) 
				{
					return false;
					
				}
			}
	return true;
}

//
