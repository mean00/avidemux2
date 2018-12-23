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
nativeAvsAudio::nativeAvsAudio(nativeAvsHeader *avsh, WAVHeader *wav,int sampleT,uint64_t duration)
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
    return true;
}
/**
    \fn sampleToTime
*/
uint64_t nativeAvsAudio::sampleToTime(uint64_t sample)
{
    float f=sample;
    f/=wavHeader->frequency;
    f*=1000000;
    f /= wavHeader->channels; 
    return (uint64_t)f;
}
/**
    \fn increment
*/
void nativeAvsAudio::increment(uint64_t sample)
{
    nextSample+=sample;
}
/**
    \fn getPacket
*/
bool      nativeAvsAudio::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
    uint32_t sizeInSample;

    switch (sampleType)
    {
        case SAMPLE_INT16:
                        sizeInSample = maxSize / 2;
                        break;
        case SAMPLE_FLOAT:
                        sizeInSample = maxSize / 4;
                        break;
        default:
            return false;
    }

    
    if (!avs->getAudioPacket(nextSample, buffer, sizeInSample))
    {
        ADM_warning("Error getPacket\n");
        return false;
    }
    switch (sampleType)
    {
        case SAMPLE_FLOAT: // FIXME !
            {

                float *p = (float *)buffer;
                int16_t *n = (int16_t *)buffer;
                for (int i = 0; i < sizeInSample; i++)
                {
                    float v = p[i];
                    v *= 32000.; // FIXME ALSO !
                    n[i] = (int16_t)v;
                }
            }

            *size = sizeInSample*2;
            break;
        default:
            *size = sizeInSample*2;
            break;
    }

    *dts = sampleToTime(nextSample);    
    increment(sizeInSample);
    return true;
};
//EOF

