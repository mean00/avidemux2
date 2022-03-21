/***************************************************************************
       \file audiofilter_channels.cpp
       \brief Channel manipulations
           (c) 2022 szlldm
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

#include "ADM_audioFilter.h"
#include "audiofilter_channels.h"
#include <math.h>

void AUDMAudioFilterChannels::resetConf(CHANSparam * cfg)
{
    for (int i=0; i<ADM_CH_LAST; i++)
        cfg->chGainDB[i] = 0;
}

AUDMAudioFilterChannels::AUDMAudioFilterChannels(AUDMAudioFilter *instream, CHANSparam * cfg):AUDMAudioFilter (instream)
{
    for (int i=0; i<ADM_CH_LAST; i++)
        chGain[i] = pow(10.0, cfg->chGainDB[i]/20.0);
    channels = _wavHeader.channels;
    memset(channelMapping,0,sizeof(CHANNEL_TYPE)*MAX_CHANNELS);
    CHANNEL_TYPE *map=_previous->getChannelMapping();
    bypass = true;
    if(map && channels)
    {
        memcpy(channelMapping,map,sizeof(CHANNEL_TYPE)*_wavHeader.channels);
        bypass = false;
    }
    
    _previous->rewind();     // rewind
};

AUDMAudioFilterChannels::~AUDMAudioFilterChannels()
{

};


uint32_t AUDMAudioFilterChannels::fill(uint32_t max,float *output,AUD_Status *status)
{
    if(bypass)
    {
        *status=AUD_END_OF_STREAM; // not recoverable for now
        return 0;
    }

    uint32_t rd = 0;
    int nbSampleMax=max/channels;
    if(!nbSampleMax) nbSampleMax=1;

    // Fill incoming buffer
    shrink();
    fillIncomingBuffer(status);
    // Block not filled ?
    if((_tail-_head)<channels)
    {
      if(*status==AUD_END_OF_STREAM && _head)
      {
        memset(_incomingBuffer.at(_head),0,sizeof(float) * channels);
        _tail=_head+channels;
        printf("[Channels] Warning asked %u symbols\n",max);
      }
      else
      {
        return 0;
      }
    }
    // How many ?

    // Let's go
    int available=0;
    if(!nbSampleMax)
    {
      printf("[Channels] Warning max %u, channels %u\n",max,channels);
    }
    available=(_tail-_head)/channels; // nb Sample
    ADM_assert(available);
    if(available > nbSampleMax) available=nbSampleMax;
    
    ADM_assert(available);
    
    // TODO
    float *in = _incomingBuffer.at(_head);
    float * out = output;
    memset(out, 0, sizeof(float) * available * channels);
    for (int i = 0; i < available; i++) {
        for (int c = 0; c < channels; c++) {
            out[c] = chGain[channelMapping[c]] * *in;
            in++;
        }
        out += channels;
    }
    
    //memcpy(output, _incomingBuffer.at(_head), available*channels *sizeof(float));
    rd = available*channels;

    _head+=available*channels;
    return rd;
    
}
