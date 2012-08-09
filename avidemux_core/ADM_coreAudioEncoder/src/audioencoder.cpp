/***************************************************************************
    \file audioencoder.cpp

    copyright            : (C) 2002-6 by mean/gruntster/Mihail 
    email                : fixounet@free.fr
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
#include "ADM_coreAudio.h"
#include "ADM_audioFilter.h"
#include "audioencoder.h"
#include "ADM_audioCodecEnum.h"
#include "BVector.h"
#include "audioencoderInternal.h"

BVector <ADM_audioEncoder *> ListOfAudioEncoder;

/**

*/
ADM_AudioEncoder::ADM_AudioEncoder(AUDMAudioFilter *in, CONFcouple *setup)
{
    _extraData=NULL;
    _extraSize=0;
    ADM_assert(in);
    _state=AudioEncoderRunning;
    _incoming=in;
    memset(&wavheader,0,sizeof(wavheader));
    tmphead=tmptail=0;
    WAVHeader  *info=in->getInfo();
    // Copy channels etc.. from incoming
    wavheader.channels=info->channels;
    wavheader.frequency=info->frequency;
}
/**

*/
ADM_AudioEncoder::~ADM_AudioEncoder()
{
    if(_extraData)
    {
        delete [] _extraData;
        _extraData=NULL;
    }
}

/**
    \fn refillBuffer
*/
bool  ADM_AudioEncoder::refillBuffer(int minimum)
{
    uint32_t filler=wavheader.frequency*wavheader.channels;
  uint32_t nb;
  AUD_Status status;
  if(AudioEncoderRunning!=_state) return false;
  while(1)
  {
    ADM_assert(tmptail>=tmphead); 
    if((tmptail-tmphead)>=minimum) return true; // already enough data
  
    if(tmphead && tmptail>filler/2)
    {
      memmove(&tmpbuffer[0],&tmpbuffer[tmphead],(tmptail-tmphead)*sizeof(float)); 
      tmptail-=tmphead;
      tmphead=0;
    }
    ADM_assert(filler>tmptail);
    nb=_incoming->fill( (filler-tmptail)/2,&tmpbuffer[tmptail],&status);
    if(!nb)
    {
      if(status!=AUD_END_OF_STREAM) ADM_assert(0);
      
      if((tmptail-tmphead)<minimum)
      {
        memset(&tmpbuffer[tmptail],0,sizeof(float)*(minimum-(tmptail-tmphead)));
        tmptail=tmphead+minimum;
        _state=AudioEncoderNoInput;  
        return true;
      }
      else continue;
    } else
      tmptail+=nb;
  }
    return true;
}
/**
        \fn reorderChannels
*/
bool   ADM_AudioEncoder::reorderChannels(float *data, uint32_t nb,CHANNEL_TYPE *input,CHANNEL_TYPE *output)
{
int channels=wavheader.channels;
       return ADM_audioReorderChannels(channels,data,nb,input,output);
}

//EOF
