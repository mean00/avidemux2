/***************************************************************************
            \file audiofilter_access.cpp
            \brief convert audiofilter to audioaccess (used for playback for example)
            (C) Mean 2009 fixounet@free.fr
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
#include <math.h>

#include "ADM_audioFilter.h"
#include "audiofilter_access.h"
#include "audioencoder.h"


/**
    \fn ADMAudioFilter_Access
*/
ADMAudioFilter_Access::ADMAudioFilter_Access(AUDMAudioFilter *incoming,ADM_AudioEncoder *encoder,uint64_t timeUs)
{
    filter=incoming;
    this->encoder=encoder;
    ADM_assert(filter);
    startTimeUs=timeUs;
    samplesSeen=0;
    printf("[FilterAccess] Created, starting at %"LU" ms\n",(uint32_t)(timeUs/1000));
}
/**
    \fn ~ADMAudioFilter_Access
*/
ADMAudioFilter_Access::~ADMAudioFilter_Access()
{
    printf("[FilterAccess] Destroyed\n");
    if(filter)
    {
#warning memleak
        //destroyEncodingFilter();
        filter=NULL;
    }
    if(encoder)
    {
        delete encoder;
        encoder=NULL;
    }
}
/**
    \fn isCBR
*/
bool      ADMAudioFilter_Access::isCBR(void)
{
    if(encoder->isVBR()) return false;
    return true;
}
/**
    \fn setPos
    \brief only goto 0 is allowed
*/
bool      ADMAudioFilter_Access::setPos(uint64_t pos)
{
    samplesSeen=0;
    return filter->rewind();
}
/**
    \fn getPos
*/

uint64_t  ADMAudioFilter_Access::getPos(void)
{
    return 0;
}

/**
    \fn getPacket
*/
bool    ADMAudioFilter_Access::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
static bool endMet=false;
    uint32_t samples;
    bool r=encoder->encode(buffer,size,&samples);
    if(false==r)
    {
        if(endMet==false)
        {
            ADM_warning("[Access] getpacket failed for encoding\n");
            endMet=true;
        }
        return false;
    }

    float d=(float)samplesSeen*1000.*1000.;
    d/=(float)(filter->getInfo()->frequency);
    if(false==encoder->provideAccurateSample())
    {
        if(!samplesSeen) 
            *dts=startTimeUs;
         else 
            *dts=ADM_AUDIO_NO_DTS;  // rely on the parser to get exact DTS
    }else   
    {
        *dts=startTimeUs+(uint64_t)d;
    }
    //printf("EncoderAccess: dts=%"LLD"\n",*dts);
    samplesSeen+=samples;
    endMet=false;
    return true;
}
/**
    \fn getExtraData
    \brief Get extradata from encoder
*/
bool      ADMAudioFilter_Access::getExtraData(uint32_t *l, uint8_t **d)
{
        ADM_assert(encoder);
        return encoder->extraData(l,d);
}

//EOF
