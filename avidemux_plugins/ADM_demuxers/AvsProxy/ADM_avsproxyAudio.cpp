/***************************************************************************
    \file ADM_avsproxyAudio.cpp
    \author (C) 2007-2010 by mean  fixounet@free.fr

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
#include "ADM_avsproxy.h"
#include "ADM_avsproxy_internal.h"
#define AVS_AUDIO_BUFFER_SIZE (48000*6*4)
/**
    \fn ADM_avsAccess
*/
ADM_avsAccess::ADM_avsAccess(avsNet *net, WAVHeader *wav,uint64_t duration)
{
    network=net;
    this->wavHeader=wav;
    this->duration=duration;
    nextSample=0;
    audioBuffer=new uint8_t[AVS_AUDIO_BUFFER_SIZE];
}
/**
    \fn ~ADM_avsAccess
*/

ADM_avsAccess::~ADM_avsAccess()
{
    if(audioBuffer)
        delete [] audioBuffer;
    audioBuffer=NULL;
}
/**
    \fn getDurationInUs
*/
uint64_t  ADM_avsAccess::getDurationInUs(void)
{
    return duration;
}
/**
    \fn goToTime
*/
bool      ADM_avsAccess::goToTime(uint64_t timeUs)
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
uint64_t ADM_avsAccess::sampleToTime(uint64_t sample)
{
    float f=sample;
    f/=wavHeader->frequency;
    f*=1000000;
    return (uint64_t)f;
}
/**
    \fn increment
*/
void ADM_avsAccess::increment(uint64_t sample)
{
    nextSample+=sample;
}
/**
    \fn getPacket
*/
bool      ADM_avsAccess::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
    avsNetPacket out;
    avsNetPacket in;
    avsAudioFrame aFrame;
    aFrame.startSample=nextSample;
    
#warning this is incorrect
    aFrame.sizeInFloatSample=maxSize/(2*wavHeader->channels);
    in.buffer=(uint8_t *)&aFrame;
    in.size=sizeof(aFrame);

    out.buffer=audioBuffer;
    out.sizeMax=maxSize+sizeof(aFrame);
    out.size=0;
    //printf("Asking for frame %d\n",framenum);
    if(!network->command(AvsCmd_GetAudio,0,&in,&out))
    {
        ADM_error("Get audio failed for frame \n");
        return false;   
    }
  //  printf("Out size : %d\n",(int)out.size);
    
    //
    //
    memcpy(&aFrame,audioBuffer,sizeof(aFrame));
   // printf("NbSample : %d\n",(int)aFrame.sizeInFloatSample);
    if(!aFrame.sizeInFloatSample)
    {
        ADM_warning("Error in audio (Zero samples\n");
        return false;
    }
    *dts=sampleToTime(nextSample);
    increment(aFrame.sizeInFloatSample);
    *size=out.size-sizeof(aFrame);
    memcpy(buffer,audioBuffer+sizeof(aFrame),out.size-sizeof(aFrame));
  return true;
};
//EOF

