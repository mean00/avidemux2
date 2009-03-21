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

extern bool            destroyEncodingFilter(void);

/**
    \fn ADMAudioFilter_Access
*/
ADMAudioFilter_Access::ADMAudioFilter_Access(AUDMAudioFilter *incoming,ADM_AudioEncoder *encoder,uint64_t timeUs)
{
    filter=incoming;
    this->encoder=encoder;
    ADM_assert(filter);
    startTimeUs=timeUs;
    memcpy(&header,incoming->getInfo(),sizeof(header));
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
        destroyEncodingFilter();
        filter=NULL;
    }
    if(encoder)
    {
        delete encoder;
        encoder=NULL;
    }
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
    uint32_t samples;
    bool r=encoder->encode(buffer,size,&samples);
    if(false==r)
    {
        printf("[Access] getpacket failed for encoding\n");
        return false;
    }

    float d=(float)samplesSeen/(float)(header.frequency);
    d*=1000*1000;
    *dts=startTimeUs+(uint64_t)d;
    samplesSeen+=samples;
    return true;
}


//EOF
