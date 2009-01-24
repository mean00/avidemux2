/***************************************************************************
                          aviaudio.hxx  -  description
                             -------------------
    begin                : Thu Nov 22 2001
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
#ifndef __WAV_Audio
#define __WAV_Audio


class AviList;


#include "ADM_coreAudio.h"
typedef struct
{
	uint16_t	encoding;	
	uint16_t	channels;					/* 1 = mono, 2 = stereo */
	uint32_t	frequency;				/* One of 11025, 22050, or 44100 48000 Hz */
	uint32_t	byterate;					/* Average bytes per second */
	uint16_t	blockalign;				/* Bytes per sample block */
	uint16_t	bitspersample;		/* One of 8, 12, 16, or 4 for ADPCM */
 // 16 bytes up to here, 14 left
 // Used for VBR mp3
  uint16_t   		cbsize ;
  uint16_t          wId ;
  uint32_t         	fdwflags ;
  uint16_t          nblocksize ;
  uint16_t          nframesperblock  ;
  uint16_t          ncodecdelay ;
} WAVHeaderVBR;

typedef struct
{
    uint32_t foffset;
    uint32_t woffset;

}ST_point;

#define WAV_MP3 	85
#define WAV_MP2 	80
#define WAV_WMA 	353
#define WAV_PCM 	1
#define WAV_MSADPCM 	2
#define WAV_LPCM 	3
#define WAV_AC3 	0x2000
#define WAV_DTS 	0x2001
#define WAV_OGG 0x676f
#define WAV_8BITS 	53 // dummy id
#define WAV_MP4 	54 // dummy id
#define WAV_AAC 	0xff // dummy id
#define WAV_AAC_HE 	0xfe // dummy id
#define WAV_8BITS_UNSIGNED 	55 // dummy id
#define WAV_AMRNB 	56 // dummy id
#define WAV_ULAW	57 // dummy id
#define WAV_QDM2 	58
#define WAV_IMAADPCM    17
#define WAV_AMV_ADPCM    9900
#define WAV_NELLYMOSER   9901
#define WAV_UNKNOWN     9999
#include "../ADM_audiocodec/ADM_audiocodec.h"
#include "ADM_fileio.h"



class AVDMGenericAudioStream
{
   	protected:
   				
    #define SIZE_INTERNAL 64*1024 
					uint8_t 	internalBuffer[2 * SIZE_INTERNAL];
					float 	internalBuffer_float[SIZE_INTERNAL];
					uint8_t 	packetBuffer[2 * SIZE_INTERNAL];
					uint32_t	packetHead,packetTail;
					uint8_t		isPaketizable( void);
            				WAVHeader	*_wavheader;
                			ADM_Audiocodec 	*_codec;
					uint32_t	_length;
       					uint32_t	_pos;
					uint8_t		_destroyable;
					char   		_name[20];
					uint8_t	_eos;
         					// This is for writing..
			              	uint32_t	_dlen;
					AviList		*_LAll;
                                        ADMFile         *_file;
       					// This is for VBR MP3
       					ST_point	*_audioMap;
            				uint32_t	_nbMap;
                        		uint32_t	_current;
					uint8_t		_mpegSync[3];
					uint8_t 	shrink( void );
					uint8_t		getPacketMP3(uint8_t *dest, uint32_t *len, 
										uint32_t *samples);
					uint8_t		getPacketAC3(uint8_t *dest, uint32_t *len, 
									uint32_t *samples);
					uint8_t		getPacketDTS(uint8_t *dest, uint32_t *len, 
									uint32_t *samples);

					uint8_t		getPacketPCM(uint8_t *dest, uint32_t *len, 
								uint32_t *samples);
					uint8_t		getPacketWMA(uint8_t *dest, uint32_t *len, 
								uint32_t *samples);
					uint8_t		getPacketAAC(uint8_t *dest, uint32_t *len, 
								uint32_t *samples);
                                        uint8_t         getPacketADPCM(uint8_t *dest, uint32_t *len,
                                                                uint32_t *samples);

       protected:
				       	uint8_t  	readc( uint8_t *c);
			      
		public:
					                           AVDMGenericAudioStream( void);
			virtual 				~AVDMGenericAudioStream() ;
          				uint8_t			beginDecompress( void );
        		     		uint32_t		getPos( void ) {return _pos;};
	             		virtual	uint8_t			buildAudioTimeLine( void );
				virtual		uint8_t 	flushPacket(void);
               
               
				  	uint8_t			endDecompress( void );
					uint8_t			isDestroyable( void ) { return _destroyable;}
					uint8_t			isCompressed( void );
					uint8_t			isDecompressable( void );
          			virtual uint32_t		readDecompress( uint32_t size,uint8_t *ptr );//deprecated
          			virtual uint32_t		readDecompress(uint32_t size, float *ptr);
				virtual uint8_t		 	goTo(uint32_t offset)=0;
				virtual uint32_t		read(uint32_t size,uint8_t *ptr)=0;//deprecated
				virtual uint32_t		read(uint32_t size,float *ptr){return 0;}
//-----------------
					WAVHeader 		*getInfo(void) { return _wavheader;}
					
  					uint32_t		getLength(void ){ return _length;};
//----------------
        			uint8_t 			goToSync(uint32_t offset);
			virtual uint8_t				goToTime(uint32_t mstime);
				uint32_t 			convTime2Offset(uint32_t time);
//__________________
				uint8_t				writeHeader(FILE *out);
         			uint8_t				endWrite(FILE *out,uint32_t len);
            	virtual 	uint8_t				isVBR(void ) { if( _audioMap) return 1; else return 0;};
                 		uint32_t			nbFrameSlice( void )
                 						{ if(!_audioMap) {printf("OOOOpppps!");return 1000;}; return  _nbMap;}
//___________________                 
		virtual 	uint8_t				extraData(uint32_t *l,uint8_t **d)
									{
										*l=0;
										*d=NULL;
										return 0;
									}
					// Get Packet
			virtual	uint8_t				getPacket(uint8_t *dest, uint32_t *len, 
										uint32_t *samples);
		// Return 1 if needs special muxing mode (AAC)			
			virtual uint8_t packetPerFrame( void)	{return 0;} 
				// The channel mapping for this stream
				// Only used by decoder...
			virtual CHANNEL_TYPE *getChannelMapping(void) {if(_codec) return _codec->channelMapping; else return NULL;}
}
;
/* Describe the channel mapping */

//
uint8_t mpegAudioIdentify(uint8_t *ptr, uint32_t maxLookUp, WAVHeader *header, uint8_t *tokens=NULL);

#endif
