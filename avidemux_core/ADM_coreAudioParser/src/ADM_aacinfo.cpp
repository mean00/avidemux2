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

static 	uint32_t aacSampleRate[16]=
{
	96000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025,  8000,
	0,     0,     0,     0 
};

#define xdebug(...) {}
/**
    \fn getAdtsAacInfo
*/

bool getAdtsAacInfo(int size, uint8_t *data, AacAudioInfo &info)
{
    if(size<2)
    {
        return false;
    }
    getBits bits(size,data); // get a copy, needed to extract extra data
   
    int audioObjectType=bits.get(5); // warning
    if(audioObjectType==31)
    {
        ADM_error("Unsupported AAC audioObject Type\n");
        return false;
    }
    int samplingFrequencyIndex=bits.get(4);
    int extensionAudioObjectType=0;
    int fq=0;
    if(samplingFrequencyIndex==0xf)
    {
            fq=(bits.get(8)<<16)+bits.get(16);
    }else
    {
        fq=aacSampleRate[samplingFrequencyIndex];
    }
    int channelConfiguration=bits.get(4);
    int channels=aacChannels[channelConfiguration];
    bool sbrPresent=false;
    xdebug("ObjectType=%d\n",audioObjectType);    
  
    switch(audioObjectType)
    {
        case 2: // GASpecificConfig
                {
                bool frameLength=bits.get(1);
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
    int consumed=bits.getConsumedBits();
    if(size*8-consumed>=16) // extensionAudioObjectType
    {
        int syncExtensionType=bits.get(11);
        if(syncExtensionType==0x2b7)
        {
            int extensionAdioObjectType=bits.get(5);
             if(extensionAdioObjectType==31)
                {
                    ADM_error("Unsupported AAC audioObject Type\n");
                    return false;
                }
             if(extensionAdioObjectType==5) 
             {
                    sbrPresent=bits.get(1);
             }
            
        }
    }
    info.frequency=fq;
    info.channels=channels;
    info.sbr=sbrPresent;

    return true;
}

//____________

