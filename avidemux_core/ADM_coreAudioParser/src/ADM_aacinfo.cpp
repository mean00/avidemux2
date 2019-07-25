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

extern "C" {
#include "libavcodec/avcodec.h"
}

#include <math.h>
#include "ADM_getbits.h"
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

static int aacSampleRate[16]=
{
    96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025,  8000,
    7350,  0,     0,     0
};

#define xdebug(...) {}

static int getAudioObjectType(getBits *bit)
{
    int type=bit->get(5);
    if(type==31)
        type=32+bit->get(6);
    return type;
}

static int getSamplingFrequency(getBits *bit)
{
    int index=bit->get(4);
    if(index==0xf)
        return bit->get(24);
    return aacSampleRate[index];
}

/**
    \fn ADM_getAacInfoFromConfig
*/

bool ADM_getAacInfoFromConfig(int size, uint8_t *data, AacAudioInfo &info)
{
    if(size<2)
    {
        return false;
    }
    int paddedSize=size+AV_INPUT_BUFFER_PADDING_SIZE;
    uint8_t *padded=new uint8_t[paddedSize];
    memset(padded,0,paddedSize);
    memcpy(padded,data,size);
    getBits bits(size,padded); // get a copy, needed to extract extra data
    int audioObjectType=getAudioObjectType(&bits);
    int fq=getSamplingFrequency(&bits);
    int channelConfiguration=bits.get(4);
    bool sbrPresent=false;
    xdebug("ObjectType=%d\n",audioObjectType);
#if 0
    switch(audioObjectType)
    {
        case 2: // GASpecificConfig
                {
                bits.get(1);	// frameLength
                bool dependsOnCoreCoder=bits.get(1);
                if(dependsOnCoreCoder) bits.skip(14);
                bool extensionFlag=bits.get(1);
                if(!channelConfiguration)
                {
                    ADM_error("No channel configuraiton\n");
                    return false;
                }
                if(extensionFlag)
                {
                    ADM_error("Extension flag\n");
                    return false;
                }
                }
                break;
        default:
                ADM_error("AudoObjecttype =%d not handled\n",audioObjectType);
                return false;
    }
#endif

    if(audioObjectType==5 || (audioObjectType==29 && !(bits.show(3) & 0x03 && !(bits.show(9) & 0x3f)))) // SBR or PS
    {
        sbrPresent=true;
        fq=getSamplingFrequency(&bits);
        audioObjectType=getAudioObjectType(&bits);
        if(audioObjectType==22)
            channelConfiguration=bits.get(4);
    }else
    {
        int max=size*8-16;
        while(bits.getConsumedBits()<max)
        {
            if(bits.show(11)==0x2b7)
            {
                bits.skip(11);
                if(getAudioObjectType(&bits)==5 && bits.get(1)==1)
                {
                    int extFq=getSamplingFrequency(&bits);
                    if(extFq && extFq!=fq)
                    {
                        sbrPresent=true;
                        fq=extFq;
                    }
                }
                break;
            }else
            {
                bits.skip(1);
            }
        }
    }

    delete [] padded;
    padded=NULL;

    if(!channelConfiguration)
    {
        ADM_error("AOT Specific Config not handled!\n");
        return false;
    }
    info.frequency=fq;
    info.channels=aacChannels[channelConfiguration];
    info.sbr=sbrPresent;

    return true;
}

//____________

