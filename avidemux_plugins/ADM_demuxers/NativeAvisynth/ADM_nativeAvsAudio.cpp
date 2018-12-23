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

    switch (sampleType)
    {
        case SAMPLE_INT16:
                        sizeInSample = maxSize / (2*wavHeader->channels);
                        break;
        case SAMPLE_FLOAT:
                        sizeInSample = maxSize / (4 * wavHeader->channels);
                        break;
        default:
            return false;
    }

    
    if (!avs->getAudioPacket(nextSample, buffer, sizeInSample))
    {
        ADM_warning("Error getPacket\n");
        return false;
    }
    *dts = clock.getTimeUs();
    clock.advanceBySample(sizeInSample );
    

    switch (sampleType)
    {
        case SAMPLE_FLOAT: // FIXME !
            {

                float *p = (float *)buffer;
                int16_t *n = (int16_t *)buffer;
                for (int i = 0; i < sizeInSample*wavHeader->channels; i++)
                {
                    float v = p[i];
                    v *= 32700.; // FIXME ALSO !
                    n[i] = (int16_t)v;
                }
            }

            *size = sizeInSample*wavHeader->channels *2;
            break;
        default:
            *size = sizeInSample*wavHeader->channels*2;
            break;
    }

    nextSample += sizeInSample;
    
    if (!*size)
        return false;
    return true;
};
//EOF

