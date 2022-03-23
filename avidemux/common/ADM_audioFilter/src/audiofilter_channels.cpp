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
    {
        cfg->chGainDB[i] = 0;
        cfg->chDelayMS[i] = 0;
    }
    cfg->enableRemap = false;
    for (int i=0; i<9; i++)
        cfg->remap[i] = i;
}

CHANNEL_TYPE AUDMAudioFilterChannels::remapToADMChannel(int r)
{
    // fL, fR, fC, sL, sR, rL, rR, rC, LFE
    switch(r)
    {
        case 0:
            return ADM_CH_FRONT_LEFT;
        case 1:
            return ADM_CH_FRONT_RIGHT;
        case 2:
            return ADM_CH_FRONT_CENTER;
        case 3:
            return ADM_CH_SIDE_LEFT;
        case 4:
            return ADM_CH_SIDE_RIGHT;
        case 5:
            return ADM_CH_REAR_LEFT;
        case 6:
            return ADM_CH_REAR_RIGHT;
        case 7:
            return ADM_CH_REAR_CENTER;
        case 8:
            return ADM_CH_LFE;
        default:
            return ADM_CH_INVALID;
    }
}

uint8_t AUDMAudioFilterChannels::rewind(void)
{
    for (int i=0; i<ADM_CH_LAST; i++)
    {
        delayPtr[i] = 0;
        for (int j=0; j<chDelay[i]; j++)
            delayLine[i][j] = 0.0;        
    }
    return AUDMAudioFilter::rewind();
}

AUDMAudioFilterChannels::AUDMAudioFilterChannels(AUDMAudioFilter *instream, CHANSparam * cfg):AUDMAudioFilter (instream)
{
    _previous->rewind();     // rewind
    
    for (int i=0; i<ADM_CH_LAST; i++)
    {
        chGain[i] = pow(10.0, cfg->chGainDB[i]/20.0);
        if (cfg->chDelayMS[i] < 0) cfg->chDelayMS[i] = 0;
        if (cfg->chDelayMS[i] > 10000) cfg->chDelayMS[i] = 10000;
        chDelay[i] = ((double)cfg->chDelayMS[i]/1000.0) * _wavHeader.frequency;
        if (chDelay[i] < 1)
            chDelay[i] = 1;
        delayPtr[i] = 0;
        delayLine[i] = new float [chDelay[i]];
        for (int j=0; j<chDelay[i]; j++)
            delayLine[i][j] = 0.0;
        
    }
    
    channels = _wavHeader.channels;
    memset(channelMapping,0,sizeof(CHANNEL_TYPE)*MAX_CHANNELS);
    CHANNEL_TYPE *map=_previous->getChannelMapping();
    bypass = true;
    if(map && channels)
    {
        memcpy(channelMapping,map,sizeof(CHANNEL_TYPE)*_wavHeader.channels);
        bypass = false;
    }

    
    /*printf("Input channel mapping: ");
    for (int c = 0; c < channels; c++)
        printf("%s ",ADM_printChannel(channelMapping[c]));
    printf("\n");*/
            
    for (int c=0; c<channels; c++)
        channelReMapping[c] = c;
    if (cfg->enableRemap)
    {
        for (int c=0; c<channels; c++)
        {
            channelReMapping[c] = -1;
        }
        for (int i=0; i<9; i++)
        {
            CHANNEL_TYPE from = remapToADMChannel(i);
            CHANNEL_TYPE to = remapToADMChannel(cfg->remap[i]);
            //printf("map %s -> %s\n",ADM_printChannel(from),ADM_printChannel(to));
            for (int c=0; c<channels; c++)
            {
                if (channelMapping[c] == from)
                {
                    for (int d=0; d<channels; d++)
                    {
                        if (channelMapping[d] == to)
                        {
                            channelReMapping[c] = d;
                        }
                    }
                }
            }
        }
    }
    
    
};

AUDMAudioFilterChannels::~AUDMAudioFilterChannels()
{
    for (int i=0; i<ADM_CH_LAST; i++)
    {
        delete [] delayLine[i];
    }
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

  
    float *in = _incomingBuffer.at(_head);
    float * out = output;
    memset(out, 0, sizeof(float) * available * channels);
    for (int i = 0; i < available; i++) {
        for (int c = 0; c < channels; c++) {
            if (channelReMapping[c] >= 0)
            {
                float samp = delayLine[channelMapping[c]][delayPtr[channelMapping[c]]];
                delayLine[channelMapping[c]][delayPtr[channelMapping[c]]] = *in;
                delayPtr[channelMapping[c]]++;
                if (delayPtr[channelMapping[c]] >= chDelay[channelMapping[c]])
                    delayPtr[channelMapping[c]] = 0;
                
                out[channelReMapping[c]] += chGain[channelMapping[c]] * samp;
            }
            in++;
        }
        out += channels;
    }
    
    rd = available*channels;

    _head+=available*channels;
    return rd;
    
}
