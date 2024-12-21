/***************************************************************************
    \file       ADM_nativeAvsAudio.cpp
    \author (C) 2018 by mean  fixounet@free.fr

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
#include "fourcc.h"
#include "DIA_coreToolkit.h"

#include "fourcc.h"
#include "ADM_nativeAvs.h"
/**
    \fn ADM_avsAccess
*/
nativeAvsAudio::nativeAvsAudio(nativeAvsHeader *avsh, WAVHeader *wav,int sampleT,uint64_t duration):
            clock(wav->frequency)
{
    this->avs=avsh;
    this->wavHeader=wav;
    this->duration=duration;
    nextSample=0;
    sampleType = sampleT;    
}
/**
    \fn ~ADM_avsAccess
*/

nativeAvsAudio::~nativeAvsAudio()
{   
	avs = NULL;
	wavHeader = NULL;
}
/**
    \fn getDurationInUs
*/
uint64_t  nativeAvsAudio::getDurationInUs(void)
{
    return duration;
}
/**
    \fn goToTime
*/
bool      nativeAvsAudio::goToTime(uint64_t timeUs)
{
    // convert us to sample
    float f=timeUs;
    f*=wavHeader->frequency;    
    f/=1000000.;
    nextSample=(uint32_t )f;
    clock.setTimeUs(timeUs);
    return true;
}
/**
    \fn getPacket
*/
bool      nativeAvsAudio::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
    uint32_t sizeInSample;
    int multiply;
    switch (sampleType)
    {
        case SAMPLE_INT16:
                        multiply = (2 * wavHeader->channels);                        
                        break;
        case SAMPLE_FLOAT:
                        multiply = (4 * wavHeader->channels);                        
                        break;
        default:
            return false;
    }
    sizeInSample = maxSize / multiply;
    
    if (!avs->getAudioPacket(nextSample, buffer, sizeInSample))
    {
        ADM_warning("Error getPacket\n");
        return false;
    }
    *dts = clock.getTimeUs();
    clock.advanceBySample(sizeInSample );
    
    *size = sizeInSample*multiply;
    nextSample += sizeInSample;
    
    if (!*size)
        return false;
    return true;
};
//EOF

