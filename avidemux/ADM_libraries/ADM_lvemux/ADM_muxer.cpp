//
// C++ Implementation: ADM_muxer
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#include <math.h>
#include "ADM_default.h"
#include "DIA_coreToolkit.h"

#include "ADM_audiofilter/audioprocess.hxx"
#include "ADM_muxer.h"
#include "ADM_a52info.h"

extern "C" {
	#include "mux_out.h"
};

#include "ADM_assert.h"

MpegMuxer::MpegMuxer( void )
{
	packStream=NULL;
	byteHead=byteTail=0;
	keepGoing=1;
}
MpegMuxer::~MpegMuxer(  )
{
	ADM_assert(!packStream);
}
uint8_t MpegMuxer::forceRestamp(void)
{
	ADM_assert(packStream);
	((PackStream *)packStream)->forceRestamp=1;
	return 1;
}
uint8_t MpegMuxer::open(char *filename, uint32_t vbitrate, uint32_t fps1000, WAVHeader *audioheader,float need)
{

double fps;
uint32_t abitrate,frequency;
PackStream *pack=NULL;

int bytes_needed,samples_needed;
float bn,sn;

	fps=fps1000;
	fps/=1000.;
	
	// lookup audio info from wavheader
	abitrate=audioheader->byterate*8;
	
	this->frequency=frequency=audioheader->frequency;
	audioBitrate=(audioheader->byterate*8)/1000;
	switch(audioheader->encoding)
	{
		case WAV_MP2: audioType=AUDIO_ID_MP2;break;
		case WAV_AC3: audioType=AUDIO_ID_AC3;break;
		default:
                  GUI_Error_HIG(QT_TR_NOOP("Incompatible audio"), QT_TR_NOOP("For DVD, audio must be MP2 or AC3."));
			return 0;
	}
	
	pack=mux_open(filename,(int)vbitrate,fps,(int)abitrate,(int)frequency,audioType);
	if(!pack)
	{
          GUI_Error_HIG(QT_TR_NOOP("lvemux init failed"), NULL);
		return 0;
	}
	packStream=(void *)pack;
	printf("Lvemux successfully initialized with :\n fq=%lu audio bitrate"
	" %lu video bitrate %lu framerate :%f id:%lu",frequency,abitrate,vbitrate,fps,audioType);
	
	sn=audioheader->frequency*1000.0;
	sn/=fps1000;
	samples_needed = (int)floor(sn+0.5); // pcm sample per frame
	  
	bn=1000./fps1000;
	bn*=audioheader->byterate;
	
	bytes_needed=(int)floor(bn+0.5);
	
	printf("Sample per frame : %d %f, bytes per frame :%d %f\n",samples_needed,sn,bytes_needed,bn);
	//_packSize=bytes_needed;	  
	_packSize=pack->audio_encoded_fs;
	printf("Pack size:%d\n",_packSize);
	needPerFrame=need;
	printf("Will take %f bytes per frame\n",needPerFrame);
  
	return 1;

}
uint8_t MpegMuxer::writeVideoPacket(uint32_t len, uint8_t *buf)
{
int r;
	ADM_assert(packStream);
	if(!len)
		{
			 keepGoing=0;
			 printf("Stopping packet\n");
		}
	if(keepGoing)
		r=mux_write_packet((PackStream *)packStream, 
                               VIDEO_ID, buf, (int) len); 
	return 1;

}
uint8_t MpegMuxer::close ( void )
{
	ADM_assert(packStream);
	 mux_close((PackStream *)packStream);
	 packStream=NULL;
	 return 1;
	
}

uint8_t MpegMuxer::writeAudioPacket(uint32_t len, uint8_t *buf)
{
int r;
uint32_t t=0;
uint32_t n;
	ADM_assert(packStream);
	if(!len)
	{
		printf("Stopping packet\n"); 
		keepGoing=0;
	}
	if(!keepGoing) return 1;
	memcpy(buffer+byteTail,buf,len);
	byteTail+=len;
	
	
	switch(audioType)
	{
		case AUDIO_ID_AC3: muxAC3();break;
		case AUDIO_ID_MP2: muxMP2();break;
		default:ADM_assert(0);
	}
	//aprintf("This round : %lu\n",t);  
	if(byteTail>=MUX_BUFFER_SIZE)
	{
		memmove(buffer,buffer+byteHead,byteTail-byteHead);
		byteTail-=byteHead;
		byteHead=0;
	
	}
	if(byteTail>MUX_BUFFER_SIZE)
	{
		printf("Tail : %lu\n",byteTail);
		printf("Head : %lu\n",byteHead);
		printf("Delta: %lu\n",byteTail-byteHead);
		printf("Pack : %lu\n",_packSize);
		ADM_assert(0);
	
	}
	return 1;
}
// Give as much complete AC3 frames as possible
//
uint8_t MpegMuxer::muxAC3(void)
{
	uint8_t *end=&buffer[byteTail];	
	uint8_t *ptr=&buffer[byteHead];	
	uint8_t *pos=NULL;
	int len=0;
	
	
	int flags,samprate,bitrate;
	
	
	while(ptr+8<end)
	{
		// look for a new AC3 sync word
		 if( (len=ADM_a52_syncinfo (ptr,&flags, &samprate, &bitrate))>0)
		 {
		 	pos=ptr;
			ptr+=len;
		 }
		 else
		 	ptr++;
	}
	// Write out the result
	if(!pos)
	{
		printf("LVEMux AC3:Could not sync :%lu \n",byteTail-byteHead);
		 return 1;
	}
	
	if(pos==&buffer[byteHead]) 
	{
		// it means we got only one frame
		// write it ? no
		printf("Lvemux : found only one AC3 frame\n");
		return 1;
	
	}
	uint32_t off=(uint32_t)(pos-&buffer[byteHead]);	
	//printf("%lu\n",off);
	mux_write_packet((PackStream *)packStream, 
                              AUDIO_ID_AC3, &buffer[byteHead], off); 
	byteHead+=off;
	return 1;
}
//
//	Borrowed from lvemux
//
uint8_t MpegMuxer::muxMP2(void)
{
 static const int sr[] = {44100, 48000, 32000, 0};
 static const int br[] = {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0 };
  	
	uint8_t *end=&buffer[byteTail];	
	uint8_t *ptr=&buffer[byteHead];	
	uint8_t *pos=NULL;
	
	uint16_t startcode=0,layer;
	static uint16_t longword;
	
	while(ptr+1<end)
	{
		startcode=(startcode<<8)+*ptr;
		if(startcode==0xfff || ((startcode&0xfff0)==0xfff0))
		{
			// grab bitrate & frequency
			longword=(startcode<<8)+*(ptr+1);
			if(startcode==0xfff)  longword<<=4;
			layer=((longword>>8)&0xc);
			
			if(layer==0xc)
				if(sr[((longword & 0xC) >> 2)&3]==frequency)
					if(br[((longword & 0xF0) >> 4)&0xF]==audioBitrate)
					{
						pos=ptr-1;				
						ptr+=3;
					}	
		}
		ptr++;
	}
	// Write out the result
	if(!pos || pos<=&buffer[byteHead])
	{
		 printf("LVEMux MP2: Could not sync :%lu \n",byteTail-byteHead);
		 return 1;
	}
	
	ADM_assert(pos>buffer);
	uint32_t off=(uint32_t)(pos-&buffer[byteHead]);	
	//printf("%lu\n",off);
	mux_write_packet((PackStream *)packStream, 
                              AUDIO_ID_MP2, &buffer[byteHead], off); 
	byteHead+=off;
	return 1;

}
uint8_t MpegMuxer::audioEmpty( void)
{
	if(byteHead==byteTail) return 1;
	return 0;

}
