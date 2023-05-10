/***************************************************************************
       \file audiofilter_fade.cpp
       \brief Fade
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
#include "audiofilter_fade.h"
#include <math.h>


void AUDMAudioFilterFade::resetConf(FADEparam * cfg)
{
    cfg->fadeIn = 0.0;
    cfg->fadeOut = 0.0;
    cfg->videoFilterBridge = false;
}

uint8_t AUDMAudioFilterFade::rewind(void)
{
    _currentSampleCount = 0;
    return AUDMAudioFilter::rewind();
}

AUDMAudioFilterFade::AUDMAudioFilterFade(AUDMAudioFilter *instream, FADEparam * cfg):AUDMAudioFilter (instream)
{
    _previous->rewind();     // rewind

    _scanned = 1;
    _totalSampleCount = -1;
    _currentSampleCount = 0;
    
    fadeInSamples = cfg->fadeIn * _wavHeader.frequency;
    fadeOutSamples = cfg->fadeOut * _wavHeader.frequency;
    if (fadeOutSamples > 0)
    {
        _scanned = 0;
    }
    
    channels = _wavHeader.channels;
    bypass = false;
};

AUDMAudioFilterFade::~AUDMAudioFilterFade()
{
    
};

uint8_t AUDMAudioFilterFade::preprocess(void)
{
    _totalSampleCount = 0;
    AUD_Status status;    
    
    _previous->rewind();
    ADM_info("Seeking for sample count, that can take a while\n");
    while (1)
    {
        int ready=_previous->fill(AUD_PROCESS_BUFFER_SIZE>>2,_incomingBuffer.at(0),&status);
        if(!ready)
        {
          if(status==AUD_END_OF_STREAM) 
          {
            break; 
          }
         else 
          {
            ADM_error("Unknown cause : %d\n",status);
            ADM_assert(0); 
          }
        }
        ADM_assert(!(ready %_wavHeader.channels));

        int sample= ready /_wavHeader.channels;
        _totalSampleCount += sample;
    }
    _scanned = 1;
    _previous->rewind();
    return 1;
}

uint32_t AUDMAudioFilterFade::fill(uint32_t max,float *output,AUD_Status *status)
{
    if(bypass)
    {
        *status=AUD_END_OF_STREAM; // not recoverable for now
        return 0;
    }
    
    if ((fadeInSamples <= 0) && (fadeOutSamples <= 0))  // nothing to do
    {
        return _previous->fill(max, output, status);
    }
    
    if(!_scanned) preprocess();

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
        printf("[Fade] Warning asked %u symbols\n",max);
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
      printf("[Fade] Warning max %u, channels %u\n",max,channels);
    }
    available=(_tail-_head)/channels; // nb Sample
    ADM_assert(available);
    if(available > nbSampleMax) available=nbSampleMax;
    
    ADM_assert(available);

  
    float *in = _incomingBuffer.at(_head);
    float * out = output;
    
    if ( (_currentSampleCount >= fadeInSamples) && ((fadeOutSamples <= 0) || (_totalSampleCount <= 0) || ((fadeOutSamples > 0) && (_totalSampleCount > 0) && ((_totalSampleCount - fadeOutSamples) > (_currentSampleCount + available))) )  )
    {
        memcpy(out, in, sizeof(float) * available * channels);
    }
    else
    {
        memset(out, 0, sizeof(float) * available * channels);
        for (int i = 0; i < available; i++) {
            float fader = 1.0;
            if ((fadeInSamples > 0) && ((_currentSampleCount + i) < fadeInSamples))
            {
                float f = (_currentSampleCount + i);
                f /= fadeInSamples;
                fader *= f;
            }
            if ((fadeOutSamples > 0) && (_totalSampleCount > 0))
            {
                if ((_currentSampleCount + i) >= (_totalSampleCount - fadeOutSamples))
                {
                    float f = (_totalSampleCount - (_currentSampleCount + i));
                    f /= fadeOutSamples;
                    fader *= f;
                }
            }
            
            fader *= fader; // better than linear, probably a log-like would be the best
            
            for (int c = 0; c < channels; c++) {
                out[c] = *in * fader;
                in++;
            }
            out += channels;
        }
    }
    
    _currentSampleCount += available;
    rd = available*channels;

    _head+=available*channels;
    return rd;
    
}
