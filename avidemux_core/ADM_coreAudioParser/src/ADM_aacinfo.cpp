//
// C++ Implementation: ADM_aacInfo
//
// Description: 
//		Decode an aac frame an fill the info field
//			The second is a template to check we do not do bogus frame detection
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//	THIS FILE IS OBSOLETE, USE ADTS/AAC INSTEAD...
//
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_default.h"
#include <math.h>


#include "ADM_default.h"
//#include "aviaudio.hxx"
#include "ADM_aacinfo.h"

static const int aacChannels[8]=
{
0, //0: Defined in AOT Specifc Config
1, //1: 1 channel: front-center
2, //2: 2 channels: front-left, front-right
3, //3: 3 channels: front-center, front-left, front-right
4, //4: 4 channels: front-center, front-left, front-right, back-center
5, // 5: 5 channels: front-center, front-left, front-right, back-left, back-right
6, // 6: 6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel
8 // 7: 8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel
};

static 	uint32_t aacBitrate[16]=
{
	96000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025,  8000,
	0,     0,     0,     0 
};
/*
[0]
	12 bits 111111	0xFC Sync tag
	
	1		layer 1 Mpeg2 0 Mpeg4
	2		00
	1		prot : 1 absent 0 present
	
[2]/16 bits	
	2		profile  00 main/01 LC
	4		sampling index
	1		private
	
	1		channel
[3]/24 bits
	2               channel (cont'ed)
	
	1		original
	1		Home
-------------------------------------	
	Sum=28 bits
	
	
	1		copyriht
	1		copyright id 
	2               *** AAC frame length (including headers)
[4]
        8		*** AAC frame length (including headers)
[5]
        3		*** AAC frame length (including headers)
	5		Buffer fullness 0x7FF = vbr
[6]
	6		Buffer fullness 0x7FF = vbr
	2		nb raw frame
	
	28 bits
	
	-- total = 56 bits = 8 bytes
	
	16		crc
	
*/
#define x_log ADM_info
/**
    \fn getAdtsAacInfo
*/

bool getAdtsAacInfo(int size, uint8_t *ptr, AacAudioInfo *info,int *offset,bool createExtraData)
{
    uint8_t *org=ptr;
    uint8_t *limit=ptr+size;
    int objectType;
    if(size<8) return false;
    if(ptr[0]!=0xff || (ptr[1]>>4)!=0xf)
    {
            x_log("Wrong adts header %02x %02x\n",ptr[0],ptr[1]);
            return false;
    }
    // [1] Mpeg2=0,Mpeg4=1
    // [2] Layer =0
    // [1] 1: no CRC, 0:CRC present
    ptr++;
    bool crc=!(ptr[0]&1);
    if(crc)
        x_log("CRC\n");
    else
        x_log("No CRC\n");
    ptr++;
    //----
    // [2] Profile, objectType -1
    // [4] Sample rate
    // [1] 0
    // [1] Channel mapping
    //
    //....
    objectType=1+(ptr[0]>>6);
    int samplerate=(ptr[0]>>2)&0xf;
    int fq=aacBitrate[samplerate];
    ADM_info("Frequency : %d\n",fq);
    if(!fq)
    {
        ADM_warning("Invalid samplerate\n");
        return false;
    }
    info->samplerate=fq;
    int channelMapping=((ptr[0]&1)<<2)+(ptr[1]>>6);
    if(channelMapping>=8)
    {
        ADM_warning("Bad channel\n");
        return false;
    }
    ADM_info("Channels %d\n",aacChannels[channelMapping]);
    info->channels=aacChannels[channelMapping];
    ptr++;
    //-------
    
   
    // [2] Channel mapping continued
    // [1] original =0
    // [1] home =0
    // [1] Copyright=0
    // [1] Copyright start=0
    // [2] Size (including header)
    //
    //....
    
    uint32_t frameSize=(ptr[0]>>6)&1;
    frameSize=(frameSize<<8)+ptr[1];
    frameSize=(frameSize<<3)+(ptr[2]>>5);
    if(frameSize!=size)
    {
        x_log("Frame size mismatch : computed %d, got %d\n",frameSize,size);
        return false;
    }
    ptr++;
    // [8] Size (continued)
    //
    //....
    
    ptr++;
    // [3] Size (continued)
    // [5] ??
    //
    //....

    ptr++;
    // [6] ??
    // [2] Frame count
    //
    //....
    ptr++;
    // Data
    int crcOffset=0;
    if(crc) crcOffset=2;
    *offset=7+crcOffset;
    info->samples=1024;
    info->size=frameSize-7-crcOffset;
    if(true==createExtraData)
    {
        info->extra[0]=(objectType<<3)+(samplerate>>1);
        info->extra[1]=(samplerate<<7)+(channelMapping<<3);
    }
    return true;
}

//____________

