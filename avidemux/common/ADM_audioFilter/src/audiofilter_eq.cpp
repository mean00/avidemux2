/***************************************************************************
       \file audiofilter_eq.cpp
       \brief Equalizer
           (c) 2022 szlldm
       Based on:
            (c) Neil C / Etanza Systems / 2K6
            public domain for all purposes
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
#include "audiofilter_eq.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

EQ3band::EQ3band(float lowFreq, float highFreq, float sampRate, float lowG, float midG, float highG)
{
    vsa = (1.0 / 4294967295.0);   // Very small amount (Denormal Fix)
    reset();
    
    lg = lowG;
    mg = midG;
    hg = highG;

    lf = 2 * sin(M_PI * (lowFreq / sampRate));
    hf = 2 * sin(M_PI * (highFreq / sampRate));
}

EQ3band::~EQ3band()
{
    
}

float EQ3band::update(float samp)
{
    double sample = samp;
    double  l,m,h;      // Low / Mid / High - Sample Values
    
    // Filter #1 (lowpass)
    f1p0  += (lf * (sample   - f1p0)) + vsa;
    f1p1  += (lf * (f1p0 - f1p1));
    f1p2  += (lf * (f1p1 - f1p2));
    f1p3  += (lf * (f1p2 - f1p3));
    l      = f1p3;

    // Filter #2 (highpass)
    f2p0  += (hf * (sample   - f2p0)) + vsa;
    f2p1  += (hf * (f2p0 - f2p1));
    f2p2  += (hf * (f2p1 - f2p2));
    f2p3  += (hf * (f2p2 - f2p3));
    h      = sdm3 - f2p3;

    // Calculate midrange (signal - (low + high))
    m      = sdm3 - (h + l);

    // Scale, Combine and store
    l     *= lg;
    m     *= mg;
    h     *= hg;

    // Shuffle history buffer
    sdm3   = sdm2;
    sdm2   = sdm1;
    sdm1   = sample;

    // Return result
    double res = (l + m + h);

    return ((float)res);    
}

void EQ3band::reset()
{
    f1p0 = f1p1 = f1p2 = f1p3 = 0.0;
    f2p0 = f2p1 = f2p2 = f2p3 = 0.0;
    sdm1 = sdm2 = sdm3 = 0.0;    
}

void AUDMAudioFilterEq::resetConf(EQparam * cfg)
{
    cfg->enable = false;
    cfg->lowDB = 0.0;
    cfg->midDB = 0.0;
    cfg->highDB = 0.0;
    cfg->cutOffLM = 880.0;
    cfg->cutOffMH = 5000.0;
}

uint8_t AUDMAudioFilterEq::rewind(void)
{
    for (int c = 0; c < MAX_CHANNELS; c++)
    {
        eq[c]->reset();
    }
    return AUDMAudioFilter::rewind();
}

AUDMAudioFilterEq::AUDMAudioFilterEq(AUDMAudioFilter *instream, EQparam * cfg):AUDMAudioFilter (instream)
{
    _previous->rewind();     // rewind

    float lowG = pow(10.0, cfg->lowDB/20.0);
    float midG = pow(10.0, cfg->midDB/20.0);
    float highG = pow(10.0, cfg->highDB/20.0);
    float cutOffLM = cfg->cutOffLM;
    float cutOffMH = cfg->cutOffMH;
    
    for (int c = 0; c < MAX_CHANNELS; c++)
    {
        eq[c] = new EQ3band(cutOffLM, cutOffMH, _wavHeader.frequency, lowG, midG, highG);
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
};

AUDMAudioFilterEq::~AUDMAudioFilterEq()
{
    for (int c = 0; c < MAX_CHANNELS; c++)
    {
        delete eq[c];
    }
};


uint32_t AUDMAudioFilterEq::fill(uint32_t max,float *output,AUD_Status *status)
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
        printf("[Eq] Warning asked %u symbols\n",max);
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
      printf("[Eq] Warning max %u, channels %u\n",max,channels);
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
            out[c] = eq[c]->update(*in);
            in++;
        }
        out += channels;
    }
    
    rd = available*channels;

    _head+=available*channels;
    return rd;
    
}
